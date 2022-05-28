#include "CrResource_pch.h"

#include "CrModelDecoderASSIMP.h"

#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/FileSystem/ICrFile.h"
#include "Core/Containers/CrPair.h"
#include "Core/FileSystem/CrPath.h"
#include "Core/CrMacros.h"

#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/ICrTexture.h"
#include "Rendering/CrGPUBuffer.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/CrMaterialCompiler.h"
#include "Rendering/CrRenderModel.h"
#include "Rendering/CrRenderMesh.h"
#include "Rendering/CrImage.h"
#include "Rendering/CrCommonVertexLayouts.h"

warnings_off
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
warnings_on

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

	CrRenderModelDescriptor modelDescriptor;

	for (uint32_t m = 0; m < scene->mNumMeshes; ++m)
	{
		CrRenderMeshSharedHandle renderMesh = LoadMesh(scene->mMeshes[m]);
		modelDescriptor.meshes.push_back(renderMesh);
		modelDescriptor.materialIndices.push_back((uint8_t)scene->mMeshes[m]->mMaterialIndex);
	}

	// Load all materials contained in the mesh. The loading of materials will trigger loading of associated resources too
	const CrPath filePath = file->GetFilePath();
	for (size_t m = 0; m < scene->mNumMaterials; ++m)
	{
		CrMaterialSharedHandle material = LoadMaterial(scene->mMaterials[m], filePath);
		modelDescriptor.materials.push_back(material);
	}

	return CrMakeShared<CrRenderModel>(modelDescriptor);
}

CrRenderMeshSharedHandle CrModelDecoderASSIMP::LoadMesh(const aiMesh* mesh)
{
	CrRenderMeshSharedHandle renderMesh = CrMakeShared<CrRenderMesh>();

	bool hasTextureCoords      = mesh->HasTextureCoords(0);
	bool hasNormals            = mesh->HasNormals();
	bool hasTangentsBitangents = mesh->HasTangentsAndBitangents();

	CrVertexBufferSharedHandle positionBuffer   = ICrRenderSystem::GetRenderDevice()->CreateVertexBuffer(cr3d::MemoryAccess::CPUStreamToGPU, PositionVertexDescriptor, (uint32_t)mesh->mNumVertices);
	CrVertexBufferSharedHandle additionalBuffer = ICrRenderSystem::GetRenderDevice()->CreateVertexBuffer(cr3d::MemoryAccess::CPUStreamToGPU, AdditionalVertexDescriptor, (uint32_t)mesh->mNumVertices);

	float3 minVertex = float3( FLT_MAX);
	float3 maxVertex = float3(-FLT_MAX);

	ComplexVertexPosition* positionBufferData = (ComplexVertexPosition*)positionBuffer->Lock();
	ComplexVertexAdditional* additionalBufferData = (ComplexVertexAdditional*)additionalBuffer->Lock();
	{
		for (size_t j = 0; j < mesh->mNumVertices; ++j)
		{
			const aiVector3D& vertex = mesh->mVertices[j];
			positionBufferData[j].position = { (half)vertex.x, (half)vertex.y, (half)vertex.z };

			minVertex = min(minVertex, float3(vertex.x, vertex.y, vertex.z));
			maxVertex = max(maxVertex, float3(vertex.x, vertex.y, vertex.z));

			// Normals
			if (hasNormals)
			{
				const aiVector3D& normal = mesh->mNormals[j];
				additionalBufferData[j].normal = 
				{
					(uint8_t)((normal.x * 0.5f + 0.5f) * 255.0f), 
					(uint8_t)((normal.y * 0.5f + 0.5f) * 255.0f), 
					(uint8_t)((normal.z * 0.5f + 0.5f) * 255.0f), 
					0
				};
			}
			else
			{
				additionalBufferData[j].normal = { 0, 255, 0, 0 };
			}

			if (hasTangentsBitangents)
			{
				const aiVector3D& tangent = mesh->mTangents[j];
				additionalBufferData[j].tangent = 
				{ 
					(uint8_t)((tangent.x * 0.5f + 0.5f) * 255.0f), 
					(uint8_t)((tangent.y * 0.5f + 0.5f) * 255.0f), 
					(uint8_t)((tangent.z * 0.5f + 0.5f) * 255.0f),
					0
				};
			}
			else
			{
				additionalBufferData[j].tangent = { 255, 0, 0, 0 };
			}

			// UV coordinates
			if (hasTextureCoords)
			{
				const aiVector3D& texCoord = mesh->mTextureCoords[0][j];
				additionalBufferData[j].uv = { (half)texCoord.x, (half)texCoord.y };
			}
			else
			{
				additionalBufferData[j].uv = { 0.0_h, 0.0_h };
			}
		}
	}
	positionBuffer->Unlock();
	additionalBuffer->Unlock();

	renderMesh->AddVertexBuffer(positionBuffer);
	renderMesh->AddVertexBuffer(additionalBuffer);

	renderMesh->SetBoundingBox(CrBoundingBox((maxVertex + minVertex) * 0.5f, (maxVertex - minVertex) * 0.5f));

	CrIndexBufferSharedHandle indexBuffer = ICrRenderSystem::GetRenderDevice()->CreateIndexBuffer(cr3d::MemoryAccess::CPUStreamToGPU, cr3d::DataFormat::R16_Uint, (uint32_t)mesh->mNumFaces * 3);

	size_t index = 0;
	uint16_t* indexData = (uint16_t*)indexBuffer->Lock();
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
	indexBuffer->Unlock();

	renderMesh->SetIndexBuffer(indexBuffer);

	return renderMesh;
}

CrMaterialSharedHandle CrModelDecoderASSIMP::LoadMaterial(const aiMaterial* aiMaterial, const CrPath& materialPath)
{
	CrMaterialDescriptor materialDescriptor;
	CrMaterialSharedHandle material = CrMaterialCompiler::Get().CompileMaterial(materialDescriptor);

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

			CrPath imagePath = materialPath.parent_path() / aiTexturePath.C_Str();
			CrImageHandle image = CrResourceManager::LoadImageFromDisk(imagePath);

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
