#include "CrResource_pch.h"

#include "CrModelDecoderTinyGLTF.h"

#include "Core/FileSystem/CrPath.h"
#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/FileSystem/ICrFile.h"
#include "Core/Containers/CrPair.h"
#include "Core/Containers/CrHashMap.h"

#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/CrGPUBuffer.h"
#include "Rendering/CrMaterialCompiler.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/CrRenderModel.h"
#include "Rendering/CrRenderMesh.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/ICrTexture.h"
#include "Rendering/CrImage.h"
#include "Rendering/CrCommonVertexLayouts.h"

#include "Resource/Image/CrImageCodecSTB.h"

#include "GeneratedShaders/ShaderMetadata.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_USE_CPP14
#include "tiny_gltf.h"

#include <string.h>

class CrRenderMesh;
using CrRenderMeshSharedHandle = CrSharedPtr<CrRenderMesh>;

class CrMaterial;
using CrMaterialSharedHandle = CrSharedPtr<CrMaterial>;

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

CrRenderMeshSharedHandle LoadMesh(const tinygltf::Model* modelData, const tinygltf::Mesh* meshData, const CrMaterialSharedHandle& material, CrHashMap<int, CrTextureSharedHandle>& textureTable)
{
	CrRenderMeshSharedHandle mesh = CrMakeShared<CrRenderMesh>();

	// TO-DO: primitives refers to sub-meshes (?)
	CrAssertMsg(meshData->primitives.size() == 1, "Not implemented");
	const tinygltf::Primitive& primitive = meshData->primitives[0];
	{
		// Material data
		if (primitive.material != -1)
		{
			const tinygltf::Material& materialInfo = modelData->materials[primitive.material];

			// Diffuse
			if (materialInfo.pbrMetallicRoughness.baseColorTexture.index != -1)
			{
				const tinygltf::Texture& diffuseTexture = modelData->textures[materialInfo.pbrMetallicRoughness.baseColorTexture.index];
				material->AddTexture(textureTable[diffuseTexture.source], Textures::DiffuseTexture0);
			}

			// Normals
			if (materialInfo.normalTexture.index != -1)
			{
				const tinygltf::Texture& normalsTexture = modelData->textures[materialInfo.normalTexture.index];
				material->AddTexture(textureTable[normalsTexture.source], Textures::NormalTexture0);
			}

			// Info
			if (materialInfo.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
			{
				const tinygltf::Texture& metallicRoughTexture = modelData->textures[materialInfo.pbrMetallicRoughness.metallicRoughnessTexture.index];
				material->AddTexture(textureTable[metallicRoughTexture.source], Textures::SpecularTexture0);
			}
		}

		// Index data
		if (primitive.indices != -1)
		{
			const tinygltf::Accessor& indexAccessor = modelData->accessors[primitive.indices];

			// Create the buffer
			cr3d::DataFormat::T format = ToDataFormat(indexAccessor.componentType);
			CrIndexBufferSharedHandle indexBuffer = ICrRenderSystem::GetRenderDevice()->CreateIndexBuffer(cr3d::MemoryAccess::CPUStreamToGPU, format, (uint32_t)indexAccessor.count);

			// Use the buffer view to copy the data
			const tinygltf::BufferView& bufferView = modelData->bufferViews[indexAccessor.bufferView];
			const unsigned char* data = modelData->buffers[bufferView.buffer].data.data();
			data = data + (indexAccessor.byteOffset + bufferView.byteOffset);

			CrAssertMsg(bufferView.byteStride == 0, "Invalid stride");
			void* indexData = indexBuffer->Lock();
			memcpy(indexData, data, bufferView.byteLength);
			indexBuffer->Unlock();

			mesh->SetIndexBuffer(indexBuffer);
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
				const tinygltf::Accessor& attribAccessor = modelData->accessors[attribute.second];
				const tinygltf::BufferView& bufferView   = modelData->bufferViews[attribAccessor.bufferView];
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

			bool hasPositions = !positions.empty();
			bool hasNormals   = !normals.empty();
			bool hasTextureCoords       = !texCoords.empty();

			float3 minVertex = float3(FLT_MAX);
			float3 maxVertex = float3(-FLT_MAX);

			// Create the vertex buffer
			CrVertexBufferSharedHandle positionBuffer = ICrRenderSystem::GetRenderDevice()->CreateVertexBuffer(cr3d::MemoryAccess::CPUStreamToGPU, PositionVertexDescriptor, (uint32_t)positions.size());
			CrVertexBufferSharedHandle additionalBuffer = ICrRenderSystem::GetRenderDevice()->CreateVertexBuffer(cr3d::MemoryAccess::CPUStreamToGPU, AdditionalVertexDescriptor, (uint32_t)positions.size());
			
			ComplexVertexPosition* positionBufferData = (ComplexVertexPosition*)positionBuffer->Lock();
			ComplexVertexAdditional* additionalBufferData = (ComplexVertexAdditional*)additionalBuffer->Lock();
			{
				for (size_t vertexIndex = 0; vertexIndex < positions.size(); ++vertexIndex)
				{
					if (hasPositions)
					{
						const GLTFFloat3& position = positions[vertexIndex];
						positionBufferData[vertexIndex].position = { (half)position.x, (half)position.y, (half)position.z };

						minVertex = min(minVertex, float3(position.x, position.y, position.z));
						maxVertex = max(maxVertex, float3(position.x, position.y, position.z));
					}

					if (hasNormals)
					{
						const GLTFFloat3& normal = normals[vertexIndex];

						additionalBufferData[vertexIndex].normal =
						{
							(uint8_t)((normal.x * 0.5f + 0.5f) * 255.0f),
							(uint8_t)((normal.y * 0.5f + 0.5f) * 255.0f),
							(uint8_t)((normal.z * 0.5f + 0.5f) * 255.0f),
							0
						};
					}
					else
					{
						additionalBufferData[vertexIndex].normal = { 0, 255, 0, 0 };
					}

					if (hasTextureCoords)
					{
						const GLTFFloat2& texCoord = texCoords[vertexIndex];
						additionalBufferData[vertexIndex].tangent = { 0, 0, 0, 0 };
						additionalBufferData[vertexIndex].uv = { (half)texCoord.x, (half)texCoord.y };
					}
					else
					{
						additionalBufferData[vertexIndex].uv = { 0.0_h, 0.0_h };
					}
				}
			}
			positionBuffer->Unlock();
			additionalBuffer->Unlock();

			mesh->AddVertexBuffer(positionBuffer);
			mesh->AddVertexBuffer(additionalBuffer);

			mesh->SetBoundingBox(CrBoundingBox((maxVertex + minVertex) * 0.5f, (maxVertex - minVertex) * 0.5f));
		}
	}

	return mesh;
}

CrRenderModelSharedHandle CrModelDecoderTinyGLTF::Decode(const CrFileSharedHandle& file)
{
	tinygltf::TinyGLTF gltfLoader;
	tinygltf::Model gltfModel;

	// Custom image loading implementation, gltf will either load the raw data from the inline buffer
	// or from the specified uri. Textures will be loaded and added to the texture table in preparation
	// to build the material
	CrHashMap<int, CrTextureSharedHandle> textureTable;
	gltfLoader.SetImageLoader(
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
	CrPath filePath = file->GetFilePath();
	std::string parentPath = filePath.parent_path().c_str();
	
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
			success = gltfLoader.LoadBinaryFromMemory(&gltfModel, &error, &warning, (unsigned char*)fileData, (unsigned int)fileSize, parentPath);
		}
		else
		{
			success = gltfLoader.LoadASCIIFromString(&gltfModel, &error, &warning, (const char*)fileData, (unsigned int)fileSize, parentPath);
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
	
	CrRenderModelDescriptor modelDescriptor;

	// Create the render model and setup the material. Note that by now, images have been loaded
	// and textures created within the loader callback
	for (uint32_t m = 0; m < gltfModel.meshes.size(); ++m)
	{
		const tinygltf::Mesh& mesh = gltfModel.meshes[m];
		CrMaterialDescriptor materialDescriptor;
		CrMaterialSharedHandle material = CrMaterialCompiler::Get().CompileMaterial(materialDescriptor);
		CrRenderMeshSharedHandle renderMesh = LoadMesh(&gltfModel, &mesh, material, textureTable);

		modelDescriptor.meshes.push_back(renderMesh);
		modelDescriptor.materialIndices.push_back((uint8_t)m);
		modelDescriptor.materials.push_back(material);
	}

	return CrMakeShared<CrRenderModel>(modelDescriptor);
}