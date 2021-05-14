#include "CrModelDecoderGLTF.h"

#include "Core/FileSystem/CrFileSystem.h"
#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/FileSystem/ICrFile.h"
#include "Core/Containers/CrPair.h"
#include "Core/Containers/CrHashMap.h"

#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/CrGPUBuffer.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/CrRenderModel.h"
#include "Rendering/CrMesh.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/ICrTexture.h"
#include "Rendering/CrImage.h"

#include "Image/CrImageDecoderSTB.h"

#include "GeneratedShaders/ShaderMetadata.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include <string.h>

using namespace tinygltf;

class CrMesh;
using CrMeshSharedHandle = CrSharedPtr<CrMesh>;

class CrMaterial;
using CrMaterialSharedHandle = CrSharedPtr<CrMaterial>;

struct SimpleVertex
{
	CrVertexElement<half, cr3d::DataFormat::RGBA16_Float> position;
	CrVertexElement<uint8_t, cr3d::DataFormat::RGBA8_Unorm> normal;
	CrVertexElement<uint8_t, cr3d::DataFormat::RGBA8_Unorm> tangent;
	CrVertexElement<half, cr3d::DataFormat::RG16_Float> uv;

	static CrVertexDescriptor GetVertexDescriptor()
	{
		return { decltype(position)::GetFormat(), decltype(normal)::GetFormat(), decltype(tangent)::GetFormat(), decltype(uv)::GetFormat() };
	}
};

cr3d::DataFormat::T ToDataFormat(int format)
{
	switch (format)
	{
		case TINYGLTF_COMPONENT_TYPE_BYTE:				return cr3d::DataFormat::R8_Sint;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:		return cr3d::DataFormat::R8_Uint;
		case TINYGLTF_COMPONENT_TYPE_SHORT:				return cr3d::DataFormat::R16_Sint;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:	return cr3d::DataFormat::R16_Uint;
		case TINYGLTF_COMPONENT_TYPE_INT:				return cr3d::DataFormat::R32_Sint;
		case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:		return cr3d::DataFormat::R32_Uint;
		case TINYGLTF_COMPONENT_TYPE_FLOAT:				return cr3d::DataFormat::R32_Float;
	}
	CrAssertMsg(false, "Failed to convert data format: %i", format);
	return cr3d::DataFormat::R32_Float;
}

template<typename T>
void LoadAttribute(CrVector<T>& targetBuffer, const unsigned char* sourceBuffer, size_t numComponents, int32_t componentSize, size_t componentStride)
{
	if (componentStride == 0)
	{
		memcpy(targetBuffer.data(), sourceBuffer, componentSize * numComponents);
	}
	else
	{
		for (size_t componentIndex = 0; componentIndex < numComponents; ++componentIndex)
		{
			memcpy(&targetBuffer[componentIndex], sourceBuffer, componentSize);
			sourceBuffer += componentStride;
		}
	}
}

CrMeshSharedHandle LoadMesh(const tinygltf::Model* modelData, const tinygltf::Mesh* meshData, const CrMaterialSharedHandle& material, CrHashMap<int, CrTextureSharedHandle>& textureTable)
{
	CrMeshSharedHandle mesh = CrMakeShared<CrMesh>();

	// TO-DO: primitives refers to sub-meshes (?)
	CrAssertMsg(meshData->primitives.size() == 1, "Not implemented");
	const Primitive& primitive = meshData->primitives[0];
	{
		// Material data
		if (primitive.material != -1)
		{
			const Material& materialInfo = modelData->materials[primitive.material];

			// Diffuse
			if (materialInfo.pbrMetallicRoughness.baseColorTexture.index != -1)
			{
				const Texture& diffuseTexture = modelData->textures[materialInfo.pbrMetallicRoughness.baseColorTexture.index];
				material->AddTexture(textureTable[diffuseTexture.source], Textures::DiffuseTexture0);
			}

			// Normals
			if (materialInfo.normalTexture.index != -1)
			{
				const Texture& normalsTexture = modelData->textures[materialInfo.normalTexture.index];
				material->AddTexture(textureTable[normalsTexture.source], Textures::NormalTexture0);
			}

			// Info
			if (materialInfo.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
			{
				const Texture& metallicRoughTexture = modelData->textures[materialInfo.pbrMetallicRoughness.metallicRoughnessTexture.index];
				material->AddTexture(textureTable[metallicRoughTexture.source], Textures::SpecularTexture0);
			}
		}

		// Index data
		if (primitive.indices != -1)
		{
			const Accessor& indexAccessor = modelData->accessors[primitive.indices];

			// Create the buffer
			cr3d::DataFormat::T format = ToDataFormat(indexAccessor.componentType);
			mesh->m_indexBuffer = ICrRenderSystem::GetRenderDevice()->CreateIndexBuffer(format, (uint32_t)indexAccessor.count);

			// Use the buffer view to copy the data
			const BufferView& bufferView = modelData->bufferViews[indexAccessor.bufferView];
			const unsigned char* data = modelData->buffers[bufferView.buffer].data.data();
			data = data + (indexAccessor.byteOffset + bufferView.byteOffset);

			CrAssertMsg(bufferView.byteStride == 0, "Invalid stride");
			void* indexData = mesh->m_indexBuffer->Lock();
			memcpy(indexData, data, bufferView.byteLength);
			mesh->m_indexBuffer->Unlock();
		}

		// Vertex data
		if (!primitive.attributes.empty())
		{
			struct GLTFFloat3
			{
				float x, y, z;
			};

			struct GLTFFloat2
			{
				float x, y;
			};

			CrVector<GLTFFloat3> positions;
			CrVector<GLTFFloat3> normals;
			CrVector<GLTFFloat2> texCoords;

			// Load each attribute
			for (const auto& attribute : primitive.attributes)
			{
				const std::string& attribName  = attribute.first;
				const Accessor& attribAccessor = modelData->accessors[attribute.second];
				const BufferView& bufferView   = modelData->bufferViews[attribAccessor.bufferView];
				const unsigned char* data      = modelData->buffers[bufferView.buffer].data.data();
				data = data + (attribAccessor.byteOffset + bufferView.byteOffset); // To find where the data starts we need to account for the accessor and view offsets
				int32_t componentSize = tinygltf::GetNumComponentsInType(attribAccessor.type) * tinygltf::GetComponentSizeInBytes(attribAccessor.componentType);
				
				if (attribName == "POSITION")
				{
					CrAssert(attribAccessor.type == TINYGLTF_TYPE_VEC3 && (ToDataFormat(attribAccessor.componentType) == cr3d::DataFormat::R32_Float));
					positions.resize(attribAccessor.count);
					LoadAttribute<GLTFFloat3>(positions, data, attribAccessor.count, componentSize, bufferView.byteStride);
				}
				else if (attribName == "NORMAL")
				{
					CrAssert(attribAccessor.type == TINYGLTF_TYPE_VEC3 && (ToDataFormat(attribAccessor.componentType) == cr3d::DataFormat::R32_Float));
					normals.resize(attribAccessor.count);
					LoadAttribute<GLTFFloat3>(normals, data, attribAccessor.count, componentSize, bufferView.byteStride);
				}
				else if (attribName == "TEXCOORD_0")
				{
					CrAssert(attribAccessor.type == TINYGLTF_TYPE_VEC2 && (ToDataFormat(attribAccessor.componentType) == cr3d::DataFormat::R32_Float));
					texCoords.resize(attribAccessor.count);
					LoadAttribute<GLTFFloat2>(texCoords, data, attribAccessor.count, componentSize, bufferView.byteStride);
				}
			}

			float3 minVertex = float3(FLT_MAX);
			float3 maxVertex = float3(-FLT_MAX);

			// Create the vertex buffer
			mesh->m_vertexBuffer = ICrRenderSystem::GetRenderDevice()->CreateVertexBuffer<SimpleVertex>((uint32_t)positions.size());
			SimpleVertex* vertexBufferData = (SimpleVertex*)mesh->m_vertexBuffer->Lock();
			{
				for (size_t vertexIndex = 0; vertexIndex < positions.size(); ++vertexIndex)
				{
					const GLTFFloat3& position = positions[vertexIndex];
					const GLTFFloat3& normal   = normals[vertexIndex];
					const GLTFFloat2& texCoord = texCoords[vertexIndex];
					SimpleVertex& vertex       = vertexBufferData[vertexIndex];
					vertex.position = { (half)position.x, (half)position.y, (half)position.z };

					minVertex = min(minVertex, float3(position.x, position.y, position.z));
					maxVertex = max(maxVertex, float3(position.x, position.y, position.z));

					vertex.normal =
					{
						(uint8_t)((normal.x * 0.5f + 0.5f) * 255.0f),
						(uint8_t)((normal.y * 0.5f + 0.5f) * 255.0f),
						(uint8_t)((normal.z * 0.5f + 0.5f) * 255.0f),
						0
					};

					vertex.tangent = { 0, 0, 0 , 0 };
					vertex.uv = { (half)texCoord.x, (half)texCoord.y };
				}
			}
			mesh->m_vertexBuffer->Unlock();

			mesh->m_boundingBox.center = (maxVertex + minVertex) * 0.5f;
			mesh->m_boundingBox.extents = (maxVertex - minVertex) * 0.5f;
		}
	}

	return mesh;
}

CrRenderModelSharedHandle CrModelDecoderGLTF::Decode(const CrFileSharedHandle& file)
{
	TinyGLTF loader;
	Model model;

	// Custom image loading implementation, gltf will either load the raw data from the inline buffer
	// or from the specified uri. Textures will be loaded and added to the texture table in preparation
	// to build the material
	CrHashMap<int, CrTextureSharedHandle> textureTable;
	loader.SetImageLoader(
		[]( tinygltf::Image* image, const int imageIndex, std::string* error, std::string* /*warning*/,
			int /*requestedWidth*/, int /*requestedHeight*/, const unsigned char* data, int dataSize, void* userData)
		{
			CrHashMap<int, CrTextureSharedHandle>* textureTable = (CrHashMap<int, CrTextureSharedHandle>*)userData;
			if (!textureTable)
			{
				(*error) += "Invalid textureTable";
				return false;
			}

			// Create a decoder to parse the binary blob
			// TODO We cannot create a decoder here, we need to use the proper image loading API that decides
			// based on extension, etc
			CrSharedPtr<ICrImageDecoder> imageDecoder = CrSharedPtr<ICrImageDecoder>(new CrImageDecoderSTB());
			CrImageHandle loadedImage = imageDecoder->Decode((void*)data, dataSize);
			if (!loadedImage)
			{
				(*error) += "image[" + std::to_string(imageIndex) + "] failed to decode the image";
				return false;
			}

			// Fill in settings for gltf (TODO: do we need to do it?)
			image->width = loadedImage->GetWidth();
			image->height= loadedImage->GetHeight();

			// Create the texture resource
			CrTextureDescriptor textureParams;
			textureParams.width = loadedImage->GetWidth();
			textureParams.height = loadedImage->GetHeight();
			textureParams.format = loadedImage->GetFormat();
			textureParams.initialData = loadedImage->GetData();
			textureParams.initialDataSize = loadedImage->GetDataSize();
			textureParams.mipmapCount = loadedImage->m_mipmapCount;
			textureParams.usage = cr3d::TextureUsage::Default;

			CrTextureSharedHandle texture = ICrRenderSystem::GetRenderDevice()->CreateTexture(textureParams);
			if (!texture)
			{
				(*error) += "image[" + std::to_string(imageIndex) + "] failed to create the texture";
				return false;
			}

			(*textureTable)[imageIndex] = texture;

			return true;
		}
	, (void*)&textureTable);

	bool binaryGLTF = strstr(file->GetFilePath(), ".glb") != nullptr;
	CrPath filePath(file->GetFilePath());
	std::string parentPath = filePath.parent_path().string();
	
	// Load it
	{
		// Read contents of the file
		uint64_t fileSize = file->GetSize();
		void* fileData = malloc(fileSize);
		file->Read(fileData, fileSize);

		std::string error, warning;
		bool success = false;
		if (binaryGLTF)
		{
			success = loader.LoadBinaryFromMemory(&model, &error, &warning, (unsigned char*)fileData, (unsigned int)fileSize, parentPath);
		}
		else
		{
			success = loader.LoadASCIIFromString(&model, &error, &warning, (const char*)fileData, (unsigned int)fileSize, parentPath);
		}
		free(fileData); // Free this now

		// Check for errors
		if (!success)
		{
			CrLog("Failed to load:%s (%s)", file->GetFilePath(), error.c_str());
			return nullptr;
		}

		if (!warning.empty())
		{
			CrLog("Warning when loading:%s (%s)", file->GetFilePath(), warning.c_str());
		}
	}
	
	// Create the render model and setup the material. Note that by now, images have been loaded
	// and textures created within the loader callback
	CrRenderModelSharedHandle renderModel = CrMakeShared<CrRenderModel>();
	renderModel->m_renderMeshes.reserve(model.meshes.size());
	for (const Mesh& mesh : model.meshes)
	{
		CrMaterialSharedHandle material = CrMakeShared <CrMaterial>();
		CrRenderMeshSharedHandle renderMesh = LoadMesh(&model, &mesh, material, textureTable);
		renderModel->m_renderMeshes.push_back(renderMesh);
		renderModel->m_materials.push_back(material);
		renderModel->m_materialMap.insert(CrPair<CrMesh*, uint32_t>(renderMesh.get(), (uint32_t)(renderModel->m_materials.size() - 1)));
	}

	return renderModel;
}