#include "Resource/CrResource_pch.h"

#include "CrModelDecoderASSIMP.h"

#include "Core/FileSystem/CrFixedPath.h"
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

#include "crstl/filesystem.h"

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

static void ProcessNode(const aiScene* scene, const aiNode* parentNode, const aiMatrix4x4& cumulativeTransform, CrRenderModelDescriptor& modelDescriptor)
{
	for (uint32_t c = 0; c < parentNode->mNumChildren; ++c)
	{
		const aiNode* childNode = parentNode->mChildren[c];
		aiMatrix4x4 childTransform = cumulativeTransform * childNode->mTransformation;
		//CrLog("Child %s with %i meshes", childNode->mName.C_Str(), childNode->mNumMeshes);

		for (uint32_t m = 0; m < childNode->mNumMeshes; ++m)
		{
			const aiMesh* mesh = scene->mMeshes[childNode->mMeshes[m]];
			CrRenderMeshHandle renderMesh = CrModelDecoderASSIMP::LoadMesh(scene, mesh, childTransform);
			modelDescriptor.AddRenderMesh(renderMesh, (uint8_t)mesh->mMaterialIndex);
		}

		ProcessNode(scene, childNode, childTransform, modelDescriptor);
	}
}

CrRenderModelHandle CrModelDecoderASSIMP::Decode(const crstl::file& file)
{
	// Read the raw data:
	uint64_t fileSize = file.get_size();
	void* fileRawData = malloc(fileSize);
	if (file.read(fileRawData, fileSize) != fileSize)
	{
		free(fileRawData);
		return nullptr;
	}

	//bool bakeTransforms = true;

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

	ProcessNode(scene, scene->mRootNode, scene->mRootNode->mTransformation, modelDescriptor);

	// Load all materials contained in the mesh. The loading of materials will trigger loading of associated resources too
	// TODO Rework this with path_view
	const CrFixedPath filePath = file.get_path().c_str();

	for (size_t m = 0; m < scene->mNumMaterials; ++m)
	{
		CrMaterialHandle material = LoadMaterial(scene->mMaterials[m], filePath);
		modelDescriptor.AddMaterial(material);
	}

	importer.FreeScene();

	return CrRenderModelHandle(new CrRenderModel(modelDescriptor));
}

CrRenderMeshHandle CrModelDecoderASSIMP::LoadMesh(const aiScene* scene, const aiMesh* mesh, const aiMatrix4x4& transform)
{
	CrRenderMeshHandle renderMesh = CrRenderMeshHandle(new CrRenderMesh());

	bool hasTextureCoords      = mesh->HasTextureCoords(0);
	bool hasNormals            = mesh->HasNormals();
	bool hasTangentsBitangents = mesh->HasTangentsAndBitangents();
	bool hasVertexColors       = mesh->HasVertexColors(0);
	const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	aiColor4D materialColor(1.0f, 1.0f, 1.0f, 1.0f);
	aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &materialColor);

	const CrRenderDeviceHandle& renderDevice = RenderSystem->GetRenderDevice();

	CrVertexBufferHandle positionBuffer   = renderDevice->CreateVertexBuffer(cr3d::MemoryAccess::GPUOnlyRead, PositionVertexDescriptor, (uint32_t)mesh->mNumVertices);
	CrVertexBufferHandle additionalBuffer = renderDevice->CreateVertexBuffer(cr3d::MemoryAccess::GPUOnlyRead, AdditionalVertexDescriptor, (uint32_t)mesh->mNumVertices);

	float3 minVertex = float3( FLT_MAX);
	float3 maxVertex = float3(-FLT_MAX);

	aiMatrix4x4 inverseTransform = transform;
	inverseTransform.Inverse().Transpose();

	ComplexVertexPosition* positionBufferData = (ComplexVertexPosition*)renderDevice->BeginBufferUpload(positionBuffer->GetHardwareBuffer());
	ComplexVertexAdditional* additionalBufferData = (ComplexVertexAdditional*)renderDevice->BeginBufferUpload(additionalBuffer->GetHardwareBuffer());
	{
		for (size_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex)
		{
			const aiVector3D& vertex = transform * mesh->mVertices[vertexIndex];
			positionBufferData[vertexIndex].position = { (half)vertex.x, (half)vertex.y, (half)vertex.z };

			minVertex = min(minVertex, float3(vertex.x, vertex.y, vertex.z));
			maxVertex = max(maxVertex, float3(vertex.x, vertex.y, vertex.z));

			// Normals
			if (hasNormals)
			{
				aiVector3D normal = ((aiMatrix3x3)inverseTransform) * mesh->mNormals[vertexIndex];
				normal.Normalize();
				additionalBufferData[vertexIndex].normal = 
				{
					(int8_t)(normal.x * 127.0f),
					(int8_t)(normal.y * 127.0f),
					(int8_t)(normal.z * 127.0f),
					0
				};
			}
			else
			{
				additionalBufferData[vertexIndex].normal = { 0, 127, 0, 0 };
			}

			if (hasTangentsBitangents)
			{
				aiVector3D tangent = ((aiMatrix3x3)inverseTransform) * mesh->mTangents[vertexIndex];
				tangent.Normalize();
				additionalBufferData[vertexIndex].tangent = 
				{ 
					(uint8_t)((tangent.x * 0.5f + 0.5f) * 255.0f), 
					(uint8_t)((tangent.y * 0.5f + 0.5f) * 255.0f), 
					(uint8_t)((tangent.z * 0.5f + 0.5f) * 255.0f),
					0
				};
			}
			else
			{
				additionalBufferData[vertexIndex].tangent = { 255, 0, 0, 0 };
			}

			// UV coordinates
			if (hasTextureCoords)
			{
				const aiVector3D& texCoord = mesh->mTextureCoords[0][vertexIndex];
				additionalBufferData[vertexIndex].uv = { (half)texCoord.x, (half)texCoord.y };
			}
			else
			{
				additionalBufferData[vertexIndex].uv = { 0.0_h, 0.0_h };
			}

			// Vertex colors
			if (hasVertexColors)
			{
				const aiColor4D& vertexColor = mesh->mColors[0][vertexIndex] * 255.0f;
				additionalBufferData[vertexIndex].color = { (uint8_t)vertexColor.r, (uint8_t)vertexColor.g, (uint8_t)vertexColor.b, (uint8_t)vertexColor.a };
			}
			else
			{
				const aiColor4D& vertexColor = materialColor * 255.0f;
				additionalBufferData[vertexIndex].color = { (uint8_t)vertexColor.r, (uint8_t)vertexColor.g, (uint8_t)vertexColor.b, (uint8_t)vertexColor.a };
			}
		}
	}
	renderDevice->EndBufferUpload(positionBuffer->GetHardwareBuffer());
	renderDevice->EndBufferUpload(additionalBuffer->GetHardwareBuffer());

	renderMesh->AddVertexBuffer(positionBuffer);
	renderMesh->AddVertexBuffer(additionalBuffer);

	renderMesh->SetBoundingBox(CrBoundingBox((maxVertex + minVertex) * 0.5f, (maxVertex - minVertex) * 0.5f));

	CrIndexBufferHandle indexBuffer = renderDevice->CreateIndexBuffer(cr3d::MemoryAccess::GPUOnlyRead, cr3d::DataFormat::R16_Uint, (uint32_t)mesh->mNumFaces * 3);

	size_t index = 0;
	uint16_t* indexData = (uint16_t*)renderDevice->BeginBufferUpload(indexBuffer->GetHardwareBuffer());
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
	renderDevice->EndBufferUpload(indexBuffer->GetHardwareBuffer());

	renderMesh->SetIndexBuffer(indexBuffer);

	return renderMesh;
}

CrMaterialHandle CrModelDecoderASSIMP::LoadMaterial(const aiMaterial* aiMaterial, const CrFixedPath& materialPath)
{
	CrMaterialDescriptor materialDescriptor;
	CrMaterialHandle material = MaterialCompiler->CompileMaterial(materialDescriptor);

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

			CrFixedPath imagePath = materialPath.parent_path() / aiTexturePath.C_Str();
			CrImageHandle image = CrResourceManager::LoadImageFromDisk(imagePath);

			CrTextureDescriptor textureParams;
			textureParams.width = image->GetWidth();
			textureParams.height = image->GetHeight();
			textureParams.format = image->GetFormat();
			textureParams.initialData = image->GetData();
			textureParams.initialDataSize = image->GetDataSize();
			textureParams.mipmapCount = image->m_mipmapCount;
			textureParams.usage = cr3d::TextureUsage::Default;

			CrTextureHandle texture = RenderSystem->GetRenderDevice()->CreateTexture(textureParams);

			if (!texture)
			{
				CrAssertMsg(false, "Texture not loaded");
			}

			material->AddTexture(texture, GetTextureSemantic(textureType));
		}
	}

	return material;
}
