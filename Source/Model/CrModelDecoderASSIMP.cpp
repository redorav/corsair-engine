#include "CrModelDecoderASSIMP.h"

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

#pragma warning(push, 0)
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma warning(pop)

#include <filesystem>

#include "GeneratedShaders/ShaderMetadata.h"

// TODO Explore a better way to do this. Assigning a semantic to an input from Assimp doesn't seem the best solution
static Textures::T GetTextureSemantic(aiTextureType textureType)
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

CrRenderModelSharedHandle CrModelDecoderASSIMP::Decode(const CrFileSharedHandle& file)
{
	// Read the raw data:
	uint64_t fileSize = file->GetSize();
	void* fileRawData = malloc(fileSize);
	if (file->Read(fileRawData, fileSize) != fileSize)
	{
		free(fileRawData);
		return nullptr;
	}

	// Import it:
	Assimp::Importer importer;
	const int importFlags = aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_ConvertToLeftHanded;
	const aiScene* scene = importer.ReadFileFromMemory(fileRawData, fileSize, importFlags);
	free(fileRawData); // We are done with the raw data.
	if (!scene)
	{
		return nullptr;
	}

	CrRenderModelSharedHandle renderModel = CrMakeShared<CrRenderModel>();

	// Load all meshes contained in the mesh
	renderModel->m_renderMeshes.reserve(scene->mNumMeshes);

	for (uint32_t m = 0; m < scene->mNumMeshes; ++m)
	{
		CrMeshSharedHandle renderMesh = LoadMesh(scene->mMeshes[m]);
		renderModel->m_renderMeshes.push_back(renderMesh);
		renderModel->m_materialMap.insert(CrPair<CrMesh*, uint32_t>(renderMesh.get(), scene->mMeshes[m]->mMaterialIndex));
	}

	// Load all materials contained in the mesh. The loading of materials will trigger loading of associated resources too
	const CrPath filePath(file->GetFilePath());
	for (size_t m = 0; m < scene->mNumMaterials; ++m)
	{
		CrMaterialSharedHandle material = LoadMaterial(scene->mMaterials[m], filePath);
		renderModel->m_materials.push_back(material);
	}

	return renderModel;
}

CrMeshSharedHandle CrModelDecoderASSIMP::LoadMesh(const aiMesh* mesh)
{
	CrMeshSharedHandle renderMesh = CrMakeShared<CrMesh>();

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

	renderMesh->m_vertexBuffer = ICrRenderSystem::GetRenderDevice()->CreateVertexBuffer<SimpleVertex>((uint32_t)mesh->mNumVertices);

	float3 minVertex = float3( FLT_MAX);
	float3 maxVertex = float3(-FLT_MAX);

	SimpleVertex* vertexBufferData = (SimpleVertex*)renderMesh->m_vertexBuffer->Lock();
	{
		for (size_t j = 0; j < mesh->mNumVertices; ++j)
		{
			const aiVector3D& vertex = mesh->mVertices[j];
			vertexBufferData[j].position = { (half)vertex.x, (half)vertex.y, (half)vertex.z };

			minVertex = min(minVertex, float3(vertex.x, vertex.y, vertex.z));
			maxVertex = max(maxVertex, float3(vertex.x, vertex.y, vertex.z));

			// Normals
			const aiVector3D& normal = mesh->mNormals[j];
			vertexBufferData[j].normal = { (uint8_t)((normal.x * 0.5f + 0.5f) * 255.0f), (uint8_t)((normal.y * 0.5f + 0.5f) * 255.0f), (uint8_t)((normal.z * 0.5f + 0.5f) * 255.0f), 0 };
			//vertex[j].normal = { n.x, n.y, n.z, 0.0f };

			const aiVector3D& tangent = mesh->mTangents[j];
			vertexBufferData[j].tangent = { (uint8_t)((tangent.x * 0.5f + 0.5f) * 255.0f), (uint8_t)((tangent.y * 0.5f + 0.5f) * 255.0f), (uint8_t)((tangent.z * 0.5f + 0.5f) * 255.0f), 0 };

			// UV coordinates
			const aiVector3D& texCoord = mesh->mTextureCoords[0][j];
			vertexBufferData[j].uv = { (half)texCoord.x, (half)texCoord.y };
		}
	}
	renderMesh->m_vertexBuffer->Unlock();

	CrBoundingBox boundingBox;
	boundingBox.center  = (maxVertex + minVertex) * 0.5f;
	boundingBox.extents = (maxVertex - minVertex) * 0.5f;

	renderMesh->m_boundingBox = boundingBox;

	renderMesh->m_indexBuffer = ICrRenderSystem::GetRenderDevice()->CreateIndexBuffer(cr3d::DataFormat::R16_Uint, (uint32_t)mesh->mNumFaces * 3);

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

	return renderMesh;
}

CrMaterialSharedHandle CrModelDecoderASSIMP::LoadMaterial(const aiMaterial* aiMaterial, const CrPath& relativePath)
{
	CrMaterialSharedHandle material = CrMakeShared<CrMaterial>();

	aiString name;
	aiMaterial->Get(AI_MATKEY_NAME, name);

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
		for (uint32_t t = 0; t < aiMaterial->GetTextureCount(textureType); ++t)
		{
			aiMaterial->GetTexture(textureType, 0, &aiTexturePath);

			CrPath fullPath = relativePath.parent_path() / aiTexturePath.C_Str();
			CrImageHandle image = CrResourceManager::LoadImageFromDisk(fullPath);

			CrTextureDescriptor textureParams;
			textureParams.width = image->GetWidth();
			textureParams.height = image->GetHeight();
			textureParams.format = image->GetFormat();
			textureParams.initialData = image->GetData();
			textureParams.initialDataSize = image->GetDataSize();
			textureParams.mipmapCount = image->m_mipmapCount;
			textureParams.usage = cr3d::TextureUsage::Default;

			CrTextureSharedHandle texture = ICrRenderSystem::GetRenderDevice()->CreateTexture(textureParams);

			if (!texture)
			{
				CrAssertMsg(false, "Texture not loaded");
			}

			material->AddTexture(texture, GetTextureSemantic(textureType));
		}
	}

	return material;
}
