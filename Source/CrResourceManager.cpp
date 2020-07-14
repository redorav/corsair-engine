#include "CrResourceManager.h"
#include "Rendering/CrImage.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/CrRenderModel.h"
#include "Rendering/CrMesh.h"
#include "Rendering/ICrTexture.h"
#include "Rendering/CrGPUBuffer.h"
#include "Rendering/ICrRenderDevice.h"
#include "ShaderResources.h" // TODO Hack - delete from the .lua

#include "Core/CrCommandLine.h"
#include "Core/FileSystem/CrFileSystem.h"
#include "Core/Containers/CrPair.h"
#include "Core/Logging/ICrDebug.h"

#include <fstream>
#include <iostream>
#include <cstdint>

using std::ios;

// This define can only be added to a single cpp
#if !defined(STB_IMAGE_IMPLEMENTATION)

#define STB_IMAGE_IMPLEMENTATION
#pragma warning(push, 0)
#include <stb_image.h>
#pragma warning(pop)

#else

#error Only define this in this cpp file!

#endif

// TODO Explore a better way to do this. Assigning a semantic to an input from Assimp doesn't seem the best solution
Textures::T GetTextureSemantic(aiTextureType textureType)
{
	switch (textureType)
	{
		case aiTextureType_DIFFUSE:
			return Textures::DiffuseTexture0;
		case aiTextureType_NORMALS:
			return Textures::NormalTexture0;
		case aiTextureType_SPECULAR:
			return Textures::SpecularTexture0;
		case aiTextureType_EMISSIVE:
			return Textures::EmissiveTexture0;
		case aiTextureType_DISPLACEMENT:
			return Textures::DisplacementTexture0;
		default:
			return Textures::DiffuseTexture0;
	}
}

bool CrResourceManager::LoadMaterial(CrMaterialSharedHandle& material, const aiMaterial* mat, const CrPath& relativePath)
{
	CrMaterial::Create(material);

	aiString name;
	mat->Get(AI_MATKEY_NAME, name);

	aiString baseTexName;
	mat->Get(AI_MATKEY_COLOR_DIFFUSE, baseTexName);

	aiTextureType textureTypes[] =
	{
		aiTextureType_DIFFUSE,
		aiTextureType_NORMALS,
		aiTextureType_SPECULAR,
		aiTextureType_EMISSIVE,
		aiTextureType_DISPLACEMENT,
	};

	aiString aiTexturePath; // Path is relative to location of imported asset

	for (aiTextureType textureType : textureTypes)
	{
		for (uint32_t t = 0; t < mat->GetTextureCount(textureType); ++t)
		{
			mat->GetTexture(textureType, 0, &aiTexturePath);

			CrPath fullPath = relativePath.parent_path() / aiTexturePath.C_Str();
			CrImageHandle image;
			CrResourceManager::LoadImageFromDisk(image, fullPath);

			CrTextureCreateParams textureParams;
			textureParams.width = image->GetWidth();
			textureParams.height = image->GetHeight();
			textureParams.format = image->GetFormat();
			textureParams.initialData = image->GetData();
			textureParams.initialDataSize = image->GetDataSize();
			textureParams.numMipmaps = image->m_numMipmaps;
			textureParams.usage = cr3d::TextureUsage::Default;

			CrTextureSharedHandle texture = ICrRenderDevice::GetRenderDevice()->CreateTexture(textureParams);

			material->AddTexture(texture, GetTextureSemantic(textureType));
		}
	}

	return true;
}

bool CrResourceManager::LoadModel(CrRenderModelSharedHandle& renderModel, const CrPath& relativePath)
{
	Assimp::Importer importer;
	CrPath fullPath = CrResourceManager::GetFullResourcePath(relativePath);

	fullPath = fullPath.lexically_normal();

	std::string stringPath = fullPath.string();

	const aiScene* scene = importer.ReadFile(stringPath.c_str(), aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded); // No additional options

	if (scene == nullptr)
	{
		//CrLogWarning("Mesh " + relativePath + " not found.");
		return false;
	}

	renderModel = CrMakeShared<CrRenderModel>();

	// Load all meshes contained in the mesh
	renderModel->m_renderMeshes.reserve(scene->mNumMeshes);

	for (uint32_t m = 0; m < scene->mNumMeshes; ++m)
	{
		CrMeshSharedHandle renderMesh;
		LoadMesh(renderMesh, scene->mMeshes[m]);
		renderModel->m_renderMeshes.push_back(renderMesh);
		renderModel->m_materialMap.insert(CrPair<CrMesh*, uint32_t>(renderMesh.get(), scene->mMeshes[m]->mMaterialIndex));
	}

	// Load all materials contained in the mesh. The loading of materials will trigger loading of associated resources too
	for (size_t m = 0; m < scene->mNumMaterials; ++m)
	{
		CrMaterialSharedHandle material;
		LoadMaterial(material, scene->mMaterials[m], relativePath);
		renderModel->m_materials.push_back(material);
	}

	return true;
}

void CrResourceManager::LoadMesh(CrMeshSharedHandle& renderMesh, const aiMesh* mesh)
{
	renderMesh = CrMakeShared<CrMesh>();

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

	CrAssertMsg(mesh->HasTextureCoords(0), "Error in mesh: no texture coordinates available.");
	CrAssertMsg(mesh->HasNormals(), "Error in mesh: no normals available.");
	CrAssertMsg(mesh->HasTangentsAndBitangents(), "Error in mesh: no tangents available.");

	renderMesh->m_vertexBuffer = ICrRenderDevice::GetRenderDevice()->CreateVertexBuffer<SimpleVertex>((uint32_t)mesh->mNumVertices);

	SimpleVertex* vertex = (SimpleVertex*)renderMesh->m_vertexBuffer->Lock();
	{
		for (size_t j = 0; j < mesh->mNumVertices; ++j)
		{
			const aiVector3D& v = mesh->mVertices[j];
			vertex[j].position = { (half)v.x, (half)v.y, (half)v.z };

			// Normals
			const aiVector3D& n = mesh->mNormals[j];
			vertex[j].normal = { (uint8_t)((n.x * 0.5f + 0.5f) * 255.0f), (uint8_t)((n.y * 0.5f + 0.5f) * 255.0f), (uint8_t)((n.z * 0.5f + 0.5f) * 255.0f), 0 };
			//vertex[j].normal = { n.x, n.y, n.z, 0.0f };

			const aiVector3D& t = mesh->mTangents[j];
			vertex[j].tangent = { (uint8_t)((t.x * 0.5f + 0.5f) * 255.0f), (uint8_t)((t.y * 0.5f + 0.5f) * 255.0f), (uint8_t)((t.z * 0.5f + 0.5f) * 255.0f), 0 };

			// UV coordinates
			const aiVector3D& uv = mesh->mTextureCoords[0][j];
			vertex[j].uv = { (half)uv.x, (half)uv.y };
		}
	}
	renderMesh->m_vertexBuffer->Unlock();

	renderMesh->m_indexBuffer = ICrRenderDevice::GetRenderDevice()->CreateIndexBuffer(cr3d::DataFormat::R16_Uint, (uint32_t)mesh->mNumFaces * 3);

	size_t index = 0;
	uint16_t* indexData = (uint16_t*)renderMesh->m_indexBuffer->Lock();
	{
		for (size_t j = 0; j < mesh->mNumFaces; ++j)
		{
			const aiFace& triangle = mesh->mFaces[j];

			for (int k = 0; k < 3; ++k)
			{
				indexData[index++] = (uint16_t)(triangle.mIndices[k]);
			}
		}
	}
	renderMesh->m_indexBuffer->Unlock();
}

CrPath CrResourceManager::GetFullResourcePath(const CrPath& relativePath)
{
	CrString dataPath = crcore::CommandLine("-root").str().c_str();
	return CrPath(dataPath.c_str()) / relativePath;
}

void CrResourceManager::ReadTextFile(const CrPath& absolutePath, CrVector<char>& text)
{
	std::ifstream fileStream(absolutePath.string().c_str(), std::ios::in | std::ios::ate | std::ios::binary);
	if (!fileStream.is_open())
	{
		printf("File %s not found\n", absolutePath.string().c_str());
	}
	else
	{
		text.resize(fileStream.tellg());
		fileStream.seekg(0, ios::beg);
		fileStream.read(text.data(), text.size());
		fileStream.close();
		text.push_back(0);
	}
}

void CrResourceManager::ReadTextFile(const CrPath& absolutePath, CrString& text)
{
	std::ifstream fileStream(absolutePath.string().c_str(), std::ios::in | std::ios::ate | std::ios::binary);
	if (!fileStream.is_open())
	{
		printf("File %s not found\n", absolutePath.string().c_str());
	}
	else
	{
		text.resize(fileStream.tellg());
		fileStream.seekg(0, ios::beg);
		fileStream.read(&text[0], text.size());
		fileStream.close();
		text.push_back(0);
	}
}

void CrResourceManager::ReadBinaryFile(const CrPath& absolutePath, CrVector<unsigned char>& bytes)
{
	std::ifstream fileStream(absolutePath.string().c_str(), std::ios::in | std::ios::ate | std::ios::binary);
	if (!fileStream.is_open())
	{
		printf("File %s not found\n", absolutePath.string().c_str());
	}
	else
	{
		bytes.resize(fileStream.tellg());
		fileStream.seekg(0, ios::beg);
		fileStream.read(reinterpret_cast<char*>(&bytes[0]), bytes.size());
		fileStream.close();
	}
}

cr3d::DataFormat::T DXGItoDataFormat(ddspp::DXGIFormat format)
{
	switch (format)
	{
		case ddspp::BC1_UNORM: return cr3d::DataFormat::BC1_RGB_Unorm;
		default: return cr3d::DataFormat::RGBA8_Unorm;
	}
}

void CrResourceManager::LoadImageFromDisk(CrImageHandle& image, const CrPath& relativePath)
{
	image = CrImageHandle(new CrImage);

	CrPath fullPath = CrResourceManager::GetFullResourcePath(relativePath);
	CrPath extension = fullPath.extension();

	ReadBinaryFile(fullPath.string().c_str(), image->m_data);

	if(CrString(".dds").comparei(extension.string().c_str()) == 0)
	{
		ddspp::Descriptor desc;
		image->m_dataPointer = ddspp::decode_header(image->m_data.data(), desc);
		image->m_format = DXGItoDataFormat(desc.format);
		image->m_width = desc.width;
		image->m_height = desc.height;
		image->m_depth = desc.depth;
		image->m_numMipmaps = desc.numMips;
		image->m_dataSize = ICrTexture::GetMipSliceOffset(image->m_format, image->m_width, image->m_height, image->m_numMipmaps, false, image->m_numMipmaps, 0);
		//image->m_type = desc.ty
	}
	else
	{
		int comp, w, h;
		image->m_dataPointer = stbi_load_from_memory(image->m_data.data(), (int) image->m_data.size(), &w, &h, &comp, STBI_default);
		image->m_width = w;
		image->m_height = h;
		image->m_numMipmaps = 1;
		image->m_format = cr3d::DataFormat::RGBA8_Unorm;
		image->m_type = cr3d::TextureType::Tex2D;
		image->m_dataSize = w * h * comp;
	}

	if (!image->m_dataPointer)
	{
		// Error
	}
}
