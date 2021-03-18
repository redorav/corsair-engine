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
	//  the texture table in preparation to build the material.
	std::vector<CrTextureSharedHandle> textureTable;
	loader.SetImageLoader(
		[]( tinygltf::Image* image, const int imageIndex, std::string* error, std::string* warning,
			int requestedWidth, int requestedHeight, const unsigned char* data, int dataSize, void* userData)
		{
			auto textureTable = (std::vector<CrTextureSharedHandle>*)userData;
			if (!textureTable)
			{
				(*error) += "Invalid textureTable!";
				return false;
			}

			// Create a decoder to parse the binary blob:
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

			// Create the texture resource:
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

			textureTable->push_back(texture);

			return true;
		}
	, (void*)&textureTable);
		

	bool binaryGLTF = strstr(file->GetFilePath(), ".glb") != nullptr;
	CrPath filePath(file->GetFilePath());
	std::string parentPath = filePath.parent_path().string();
	
	// Load it:
	{
		// Read contents of the file:
		uint64_t fileSize = file->GetSize();
		void* fileData = malloc(fileSize);
		file->Read(fileData, fileSize);

		std::string error, warning;
		bool success = false;
		if (binaryGLTF)
		{
			// TO-DO
			//loader.LoadBinaryFromMemory()
		}
		else
		{
			success = loader.LoadASCIIFromString(&model, &error, &warning, (const char*)fileData, (unsigned int)fileSize, parentPath);
		}
		free(fileData); // Free this now.

		// Check for errors:
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
	
	// Create the render mode:
	CrRenderModelSharedHandle renderModel = CrMakeShared<CrRenderModel>();
	renderModel->m_renderMeshes.reserve(model.meshes.size());
	for (const auto mesh : model.meshes)
	{
		renderModel->m_renderMeshes.push_back(LoadMesh(&model, &mesh));
	}


	if (file)
		return nullptr;
	return nullptr;
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

CrMeshSharedHandle CrModelDecoderGLTF::LoadMesh(const tinygltf::Model* modelData, const tinygltf::Mesh* meshData)
{
	CrMeshSharedHandle mesh = CrMakeShared<CrMesh>();

	for (const auto primitive : meshData->primitives)
	{		
		if (primitive.indices != -1) // Index data.
		{
			const auto indexAccessor = modelData->accessors[primitive.indices];
			const auto bufferView = modelData->bufferViews[indexAccessor.bufferView];

			//mesh->m_indexBuffer = ICrRenderSystem::GetRenderDevice()->CreateIndexBuffer(cr3d::DataFormat::R16_Uint, bufferView.);
			//uint16_t* indexData = (uint16_t*)mesh->m_indexBuffer->Lock();
			//mesh->m_indexBuffer->Unlock();

		}
		else if (!primitive.attributes.empty()) // Vertex data.
		{
			for (const auto attribute : primitive.attributes)
			{
				const std::string& attribName = attribute.first;
				const int attribAccessorIndex = attribute.second;

				const auto attribAcessor = modelData->accessors[attribAccessorIndex];

				// puke.
				if (attribute.first == "POSITION")
				{

				}
			}
		}
	}

	return mesh;
}