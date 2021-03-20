#include "CrModelDecoderGLTF.h"

#include "Core/FileSystem/CrFileSystem.h"
#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/FileSystem/ICrFile.h"
#include "Core/Containers/CrPair.h"

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
#pragma warning(disable : 4100 4189)

CrRenderModelSharedHandle CrModelDecoderGLTF::Decode(const CrFileSharedHandle& file)
{
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;

	// Custom image loading implementation, gltf will either load the 
	//	raw data from the inline buffer or from the specified uri. Textures will be loaded and added to 
	//  the texture table in preparation to build the material
	std::map<int, CrTextureSharedHandle> textureTable;
	loader.SetImageLoader(
		[]( tinygltf::Image* image, const int imageIndex, std::string* error, std::string* warning,
			int requestedWidth, int requestedHeight, const unsigned char* data, int dataSize, void* userData)
		{
			auto textureTable = (std::map<int, CrTextureSharedHandle>*)userData;
			if (!textureTable)
			{
				(*error) += "Invalid textureTable!";
				return false;
			}

			// Create a decoder to parse the binary blob
			CrSharedPtr<ICrImageDecoder> imageDecoder = CrSharedPtr<ICrImageDecoder>(new CrImageDecoderSTB());
			CrImageHandle loadedImage = imageDecoder->Decode((void*)data, dataSize);
			if (!loadedImage)
			{
				(*error) += "image[" + std::to_string(imageIndex) + "] failed to decode the image.";
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
			textureParams.numMipmaps = loadedImage->m_numMipmaps;
			textureParams.usage = cr3d::TextureUsage::Default;

			CrTextureSharedHandle texture = ICrRenderSystem::GetRenderDevice()->CreateTexture(textureParams);
			if (!texture)
			{
				(*error) += "image[" + std::to_string(imageIndex) + "] failed to create the texture.";
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
	for (const auto mesh : model.meshes)
	{
		CrMaterialSharedHandle material = CrMakeShared <CrMaterial>();
		CrRenderMeshSharedHandle renderMesh = LoadMesh(&model, &mesh, material, textureTable);
		renderModel->m_renderMeshes.push_back(renderMesh);
		renderModel->m_materials.push_back(material);
		renderModel->m_materialMap.insert(CrPair<CrMesh*, uint32_t>(renderMesh.get(), (uint32_t)(renderModel->m_materials.size() - 1)));
	}

	return renderModel;
}

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

cr3d::DataFormat::T ToDataFormat(int format, int type = -1)
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

CrMeshSharedHandle CrModelDecoderGLTF::LoadMesh(const tinygltf::Model* modelData, const tinygltf::Mesh* meshData, CrMaterialSharedHandle material, std::map<int, CrTextureSharedHandle>& textureTable)
{
	CrMeshSharedHandle mesh = CrMakeShared<CrMesh>();

	// TO-DO: primitives refers to sub-meshes (?)
	CrAssertMsg(meshData->primitives.size() == 1, "Not implemented");
	for (const auto primitive : meshData->primitives)
	{		
		// Material data
		if (primitive.material != -1)
		{
			auto materialInfo = modelData->materials[primitive.material];
			
			// Diffuse
			if (materialInfo.pbrMetallicRoughness.baseColorTexture.index != -1)
			{
				auto diffuseTexture = modelData->textures[materialInfo.pbrMetallicRoughness.baseColorTexture.index];
				material->AddTexture(textureTable[diffuseTexture.source], Textures::DiffuseTexture0);
			}

			// Normals
			if (materialInfo.normalTexture.index != -1)
			{
				auto normalsTexture = modelData->textures[materialInfo.normalTexture.index];
				material->AddTexture(textureTable[normalsTexture.source], Textures::NormalTexture0);
			}
		}

		// Index data
		if (primitive.indices != -1) 
		{
			const auto indexAccessor = modelData->accessors[primitive.indices];
			CrAssertMsg(indexAccessor.byteOffset == 0, "Handle this case!");
			
			// Create the buffer
			auto format = ToDataFormat(indexAccessor.componentType);
			mesh->m_indexBuffer = ICrRenderSystem::GetRenderDevice()->CreateIndexBuffer(format, (uint32_t)indexAccessor.count);
		
			// Use the buffer view to copy the data
			const auto bufferView = modelData->bufferViews[indexAccessor.bufferView];
			const unsigned char* data = modelData->buffers[bufferView.buffer].data.data();
			data = data + bufferView.byteOffset;
			if (bufferView.byteStride)
			{
				CrAssertMsg(false, "Byte stride not implemented");
			}
			else
			{
				void* indexData = mesh->m_indexBuffer->Lock();
				memcpy(indexData, data, bufferView.byteLength); // check byteLength
				mesh->m_indexBuffer->Unlock();
			}
		}

		// Vertex data
		if (!primitive.attributes.empty()) 
		{
			std::vector<hlslpp::float3> positions;
			std::vector<hlslpp::float3> normals;
			std::vector<hlslpp::float2> texCoords;

			// TO-DO: If we are using gltf, we should properly leverage it by having it provide 
			//        us with data that we can just copy into our GPU buffers without spending time
			//        doing further processing \O_O/
			// Load each attribute
			for (const auto attribute : primitive.attributes)
			{
				const std::string& attribName = attribute.first;
				const auto attribAccessor = modelData->accessors[attribute.second];
				const auto bufferView = modelData->bufferViews[attribAccessor.bufferView];
				const unsigned char* data = modelData->buffers[bufferView.buffer].data.data(); // .data() contiguous  chunk?
				data = data + bufferView.byteOffset;
				int32_t componentSize = tinygltf::GetNumComponentsInType(attribAccessor.type) * tinygltf::GetComponentSizeInBytes(attribAccessor.componentType);
				if (attribName == "POSITION")
				{
					// Sanity check
					if (attribAccessor.type != TINYGLTF_TYPE_VEC3 || (ToDataFormat(attribAccessor.componentType) != cr3d::DataFormat::R32_Float))
					{
						CrAssert(false);
					}
					positions.resize(attribAccessor.count);
					LoadAttribute<hlslpp::float3>(positions, data, attribAccessor.count, componentSize, bufferView.byteStride);				
				}
				else if (attribName == "NORMAL")
				{
					// Sanity check
					if (attribAccessor.type != TINYGLTF_TYPE_VEC3 || (ToDataFormat(attribAccessor.componentType) != cr3d::DataFormat::R32_Float))
					{
						CrAssert(false);
					}
					normals.resize(attribAccessor.count);
					LoadAttribute<hlslpp::float3>(normals, data, attribAccessor.count, componentSize, bufferView.byteStride);
				}
				else if (attribName == "TEXCOORD_0")
				{
					// Sanity check
					if (attribAccessor.type != TINYGLTF_TYPE_VEC2 || (ToDataFormat(attribAccessor.componentType) != cr3d::DataFormat::R32_Float))
					{
						CrAssert(false);
					}
					texCoords.resize(attribAccessor.count);
					LoadAttribute<hlslpp::float2>(texCoords, data, attribAccessor.count, componentSize, bufferView.byteStride);
				}
			}

			// Create the vertex buffer
			mesh->m_vertexBuffer = ICrRenderSystem::GetRenderDevice()->CreateVertexBuffer<SimpleVertex>((uint32_t)positions.size());
			SimpleVertex* vertexBufferData = (SimpleVertex*)mesh->m_vertexBuffer->Lock();
			{
				for (size_t vtxIndex = 0; vtxIndex < positions.size(); ++vtxIndex)
				{
					const auto curPosition = positions[vtxIndex];
					const auto curNormal = normals[vtxIndex];
					const auto curTexcoord = texCoords[vtxIndex];
					SimpleVertex& curVertex = vertexBufferData[vtxIndex];
					curVertex.position = { (half)curPosition.x, (half)curPosition.y, (half)curPosition.z };
					curVertex.normal = {
						(uint8_t)((curNormal.x * 0.5f + 0.5f) * 255.0f),
						(uint8_t)((curNormal.y * 0.5f + 0.5f) * 255.0f),
						(uint8_t)((curNormal.z * 0.5f + 0.5f) * 255.0f),
					};
					curVertex.tangent = { 0, 0, 0 };
					curVertex.uv = { (half)curTexcoord.x, (half)curTexcoord.y };
				}
			}
			mesh->m_vertexBuffer->Unlock();
		}
	}
	return mesh;
}