#include "Resource/CrResource_pch.h"

#include "CrModelDecoderUFBX.h"

#include "Core/FileSystem/ICrFile.h"
#include "Core/Containers/CrVector.h"

#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/CrRenderModel.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/CrImage.h"
#include "Rendering/CrMaterialCompiler.h"
#include "Rendering/CrCommonVertexLayouts.h"

#include "GeneratedShaders/ShaderMetadata.h"

warnings_off
// Make sure to compile the static library with this define as well
#define UFBX_REAL_IS_FLOAT
#include <ufbx.h>

#include <meshoptimizer.h>

#include <mikktspace.h>

warnings_on

struct FBXTextureTranslation
{
	ufbx_material_pbr_map ufbxPbrMap;
	ufbx_material_fbx_map ufbxFbxMap;
};

// A unique vertex in the mesh
struct CrImportVertex
{
	float3 position;
	float3 normal;
	float3 tangent;
	float2 uv;
	float4 color;
};

struct CrImportTriangle
{
	uint32_t indices[3];
};

// An importer-independent view of a mesh
struct CrImportMesh
{
	CrVector<CrImportVertex> vertices;

	CrVector<CrImportTriangle> triangles;

	CrVector<uint32_t> indices;
};

static Textures::T GetTextureSemantic(const FBXTextureTranslation& textureType)
{
	switch (textureType.ufbxPbrMap)
	{
		case UFBX_MATERIAL_PBR_BASE_COLOR:
			return Textures::DiffuseTexture0;
		case UFBX_MATERIAL_PBR_NORMAL_MAP:
			return Textures::NormalTexture0;
		case UFBX_MATERIAL_PBR_SPECULAR_COLOR:
			return Textures::SpecularTexture0;
		case UFBX_MATERIAL_PBR_EMISSION_COLOR:
			return Textures::EmissiveTexture0;
		case UFBX_MATERIAL_PBR_DISPLACEMENT_MAP:
			return Textures::DisplacementTexture0;
		default:
			return Textures::DiffuseTexture0;
	}
}

int MikkTSpaceGetNumFaces(const SMikkTSpaceContext* pContext)
{
	CrImportMesh* importMesh = (CrImportMesh*)pContext->m_pUserData;
	return (int)importMesh->triangles.size();
}

int MikkTSpaceGetNumVerticesOfFace(const SMikkTSpaceContext* /*pContext*/, const int /*iFace*/)
{
	return 3;
}

void MikkTSpaceGetPosition(const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert)
{
	CrImportMesh* importMesh = (CrImportMesh*)pContext->m_pUserData;
	const CrImportTriangle& importTriangle = importMesh->triangles[iFace];
	const CrImportVertex& importVertex = importMesh->vertices[importTriangle.indices[iVert]];
	fvPosOut[0] = importVertex.position.x;
	fvPosOut[1] = importVertex.position.y;
	fvPosOut[2] = importVertex.position.z;
}

void MikkTSpaceGetNormal(const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert)
{
	CrImportMesh* importMesh = (CrImportMesh*)pContext->m_pUserData;
	const CrImportTriangle& importTriangle = importMesh->triangles[iFace];
	const CrImportVertex& importVertex = importMesh->vertices[importTriangle.indices[iVert]];
	fvNormOut[0] = importVertex.normal.x;
	fvNormOut[1] = importVertex.normal.y;
	fvNormOut[2] = importVertex.normal.z;
}

void MikkTSpaceGetTexCoord(const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert)
{
	CrImportMesh* importMesh = (CrImportMesh*)pContext->m_pUserData;
	const CrImportTriangle& importTriangle = importMesh->triangles[iFace];
	const CrImportVertex& importVertex = importMesh->vertices[importTriangle.indices[iVert]];
	fvTexcOut[0] = importVertex.uv.x;
	fvTexcOut[1] = importVertex.uv.y;
}

void MikkTSpaceSetTSpaceBasic(const SMikkTSpaceContext* pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert)
{
	CrImportMesh* importMesh = (CrImportMesh*)pContext->m_pUserData;
	const CrImportTriangle& importTriangle = importMesh->triangles[iFace];
	CrImportVertex& importVertex = importMesh->vertices[importTriangle.indices[iVert]];
	importVertex.tangent.x = fvTangent[0];
	importVertex.tangent.y = fvTangent[1];
	importVertex.tangent.z = fvTangent[2];
	importVertex.tangent *= fSign;
}

//void MikkTSpaceSetTSpace(const SMikkTSpaceContext* pContext, const float fvTangent[], const float fvBiTangent[], const float fMagS, const float fMagT,
//	const tbool bIsOrientationPreserving, const int iFace, const int iVert)
//{
//
//}

static CrMaterialHandle LoadMaterial(const ufbx_material* ufbxMaterial, const CrFixedPath& materialPath)
{
	(void)materialPath;

	CrMaterialDescriptor materialDescriptor;
	CrMaterialHandle material = CrMaterialCompiler::Get().CompileMaterial(materialDescriptor);

	for (size_t p = 0; p < ufbxMaterial->props.props.count; ++p)
	{
		const ufbx_prop& ufbxProp = ufbxMaterial->props.props[p];

		CrFixedString128 ufbxPropName(ufbxProp.name.data, ufbxProp.name.length);

		// Colors in the FBX format are assumed to come in as sRGB, so we convert to linear directly
		if (ufbxPropName.compare("DiffuseColor") == 0)
		{
			material->m_color = float4(pow(float3(ufbxProp.value_vec4.x, ufbxProp.value_vec4.y, ufbxProp.value_vec4.z), 2.2f), ufbxProp.value_vec4.w);
		}
	}

	CrString materialName(ufbxMaterial->name.data, ufbxMaterial->name.length);

	FBXTextureTranslation textureTypes[] =
	{
		{ UFBX_MATERIAL_PBR_BASE_COLOR, UFBX_MATERIAL_FBX_DIFFUSE_COLOR },
		{ UFBX_MATERIAL_PBR_NORMAL_MAP, UFBX_MATERIAL_FBX_NORMAL_MAP },
		{ UFBX_MATERIAL_PBR_SPECULAR_COLOR, UFBX_MATERIAL_FBX_SPECULAR_COLOR },
		{ UFBX_MATERIAL_PBR_EMISSION_COLOR, UFBX_MATERIAL_FBX_EMISSION_COLOR },
		{ UFBX_MATERIAL_PBR_DISPLACEMENT_MAP, UFBX_MATERIAL_FBX_DISPLACEMENT }
	};

	for (const FBXTextureTranslation& textureType : textureTypes)
	{
		const ufbx_material_map& pbrMaterialMap = ufbxMaterial->pbr.maps[textureType.ufbxPbrMap];
		const ufbx_material_map& fbxMaterialMap = ufbxMaterial->fbx.maps[textureType.ufbxPbrMap];

		ufbx_texture* availableTexture = pbrMaterialMap.texture ? pbrMaterialMap.texture : fbxMaterialMap.texture;

		if (availableTexture)
		{
			CrFixedPath imagePath = materialPath.parent_path() / availableTexture->filename.data;
			CrImageHandle image = CrResourceManager::LoadImageFromDisk(imagePath);

			CrTextureDescriptor textureParams;
			textureParams.width = image->GetWidth();
			textureParams.height = image->GetHeight();
			textureParams.format = image->GetFormat();
			textureParams.initialData = image->GetData();
			textureParams.initialDataSize = image->GetDataSize();
			textureParams.mipmapCount = image->m_mipmapCount;
			textureParams.usage = cr3d::TextureUsage::Default;

			CrTextureHandle texture = ICrRenderSystem::GetRenderDevice()->CreateTexture(textureParams);

			if (!texture)
			{
				CrAssertMsg(false, "Texture not loaded");
			}

			material->AddTexture(texture, GetTextureSemantic(textureType));
		}
	}

	return material;
}

static bool IsRightHandedCoordinateSystem(ufbx_coordinate_axes axes)
{
	switch (axes.up)
	{
		case UFBX_COORDINATE_AXIS_POSITIVE_Y:
			return
			(axes.right == UFBX_COORDINATE_AXIS_POSITIVE_X && axes.front == UFBX_COORDINATE_AXIS_POSITIVE_Z) || 
			(axes.right == UFBX_COORDINATE_AXIS_NEGATIVE_X && axes.front == UFBX_COORDINATE_AXIS_NEGATIVE_Z);
		case UFBX_COORDINATE_AXIS_POSITIVE_Z:
			return
			(axes.right == UFBX_COORDINATE_AXIS_POSITIVE_X && axes.front == UFBX_COORDINATE_AXIS_NEGATIVE_Y) ||
			(axes.right == UFBX_COORDINATE_AXIS_NEGATIVE_X && axes.front == UFBX_COORDINATE_AXIS_POSITIVE_Y);
		case UFBX_COORDINATE_AXIS_POSITIVE_X:
			return
			(axes.right == UFBX_COORDINATE_AXIS_POSITIVE_Y && axes.front == UFBX_COORDINATE_AXIS_NEGATIVE_Z) ||
			(axes.right == UFBX_COORDINATE_AXIS_NEGATIVE_Y && axes.front == UFBX_COORDINATE_AXIS_POSITIVE_Z);
	}

	return false;
}

CrRenderModelHandle CrModelDecoderUFBX::Decode(const CrFileHandle& file)
{
	// Read the raw data
	uint64_t fileSize = file->GetSize();

	CrVector<uint8_t> fileRawData;
	fileRawData.resize_uninitialized(fileSize);

	if (file->Read(fileRawData.data(), fileSize) != fileSize)
	{
		return nullptr;
	}

	ufbx_load_opts ufbxOptions = {};
	//ufbxOptions.generate_missing_normals = true;
	//ufbxOptions.normalize_normals = true;
	//ufbxOptions.target_unit_meters = 0.01f;
	ufbxOptions.target_axes.up = UFBX_COORDINATE_AXIS_POSITIVE_Y;
	ufbxOptions.target_axes.right = UFBX_COORDINATE_AXIS_POSITIVE_X;

	// Forward is the opposite of front. We are forward positive Z
	ufbxOptions.target_axes.front = UFBX_COORDINATE_AXIS_NEGATIVE_Z;
	ufbx_error ufbxError = {};

	ufbx_scene* ufbxScene = ufbx_load_memory(fileRawData.data(), fileRawData.size(), &ufbxOptions, &ufbxError);

	if (ufbxError.type != UFBX_ERROR_NONE || !ufbxScene)
	{
		return nullptr;
	}

	bool needsWindingOrderFlip = IsRightHandedCoordinateSystem(ufbxScene->settings.axes);

	CrRenderModelDescriptor modelDescriptor;

	CrHashMap<ufbx_material*, uint32_t> materialMap;

	// Load all materials contained in the mesh. The loading of materials will trigger loading of associated resources too
	const CrFixedPath filePath = file->GetFilePath();

	for (size_t m = 0; m < ufbxScene->materials.count; ++m)
	{
		ufbx_material* ufbxMaterial = ufbxScene->materials[m];
		CrMaterialHandle material = LoadMaterial(ufbxMaterial, filePath);
		modelDescriptor.AddMaterial(material);
		materialMap.insert({ ufbxMaterial, m });
	}

	CrArray<uint32_t, 128 * 3> tempIndices;

	for (size_t nodeIndex = 0; nodeIndex < ufbxScene->nodes.count; nodeIndex++)
	{
		ufbx_node* ufbxNode = ufbxScene->nodes[nodeIndex];

		ufbx_matrix ufbxNodeToWorld = ufbxNode->node_to_world;

		ufbx_mesh* ufbxMesh = ufbxNode->mesh;

		// TODO This might not be unique per node. Need a map/set that determines whether a mesh
		// has been created already, if we want internal instancing
		if (ufbxMesh)
		{
			CrAssertMsg(ufbxMesh->instances.count == 1, "Only one instance supported");

			// There is only one material per rendered mesh. Ufbx has a list of indices that use a certain material
			// so we'll use that to build our meshes
			for (size_t materialIndex = 0; materialIndex < ufbxMesh->materials.count; ++materialIndex)
			{
				ufbx_mesh_material* ufbxMeshMaterial = &ufbxMesh->materials.data[materialIndex];

				if (ufbxMeshMaterial->num_faces == 0)
				{
					continue;
				}

				CrRenderMeshHandle renderMesh = CrRenderMeshHandle(new CrRenderMesh());

				const CrRenderDeviceHandle& renderDevice = ICrRenderSystem::GetRenderDevice();

				bool hasUVs = ufbxMesh->vertex_uv.exists;
				bool hasNormals = ufbxMesh->vertex_normal.exists;
				bool hasColors = ufbxMesh->vertex_color.exists;
				bool hasTangents = ufbxMesh->vertex_tangent.exists;

				// Allocate necessary data. Note that the number of vertices and indices is the same, as we load the mesh
				// without deduplicating vertices so we can later generate tangents, etc. After the import mesh is loaded
				// we'll preprocess it appropriately
				CrImportMesh importMesh;
				importMesh.vertices.resize_uninitialized(ufbxMeshMaterial->num_triangles * 3);
				importMesh.triangles.resize_uninitialized(ufbxMeshMaterial->num_triangles);
				importMesh.indices.resize_uninitialized(ufbxMeshMaterial->num_triangles * 3);

				uint32_t currentVertex = 0;
				uint32_t currentTriangle = 0;

				// This remapping table allows us to efficiently remap the indices when we change handedness
				uint32_t vertexIndexRemap[3];
				vertexIndexRemap[0] = needsWindingOrderFlip ? 1 : 0;
				vertexIndexRemap[1] = needsWindingOrderFlip ? 0 : 1;
				vertexIndexRemap[2] = 2;

				// Loop through every face, triangulating if necessary
				for (size_t faceIndex = 0; faceIndex < ufbxMeshMaterial->num_faces; ++faceIndex)
				{
					ufbx_face face = ufbxMesh->faces[ufbxMeshMaterial->face_indices.data[faceIndex]];

					size_t triangleCount = ufbx_triangulate_face(tempIndices.data(), tempIndices.size(), ufbxMesh, face);

					for (uint32_t triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
					{
						CrImportTriangle& triangle = importMesh.triangles[currentTriangle];

						for (size_t i = 0; i < 3; ++i)
						{
							size_t triangleVertexIndex = vertexIndexRemap[i];

							size_t ufbxVertexIndex = tempIndices[triangleIndex * 3 + triangleVertexIndex];

							CrImportVertex importVertex;

							ufbx_vec3 ufbxPosition = ufbx_get_vertex_vec3(&ufbxMesh->vertex_position, ufbxVertexIndex);
							ufbx_vec3 ufbxTransformedPosition = ufbx_transform_position(&ufbxNodeToWorld, ufbxPosition);
							importVertex.position = float3(ufbxTransformedPosition.x, ufbxTransformedPosition.y, ufbxTransformedPosition.z);

							if (hasNormals)
							{
								ufbx_vec3 ufbxNormal = ufbx_get_vertex_vec3(&ufbxMesh->vertex_normal, ufbxVertexIndex);
								ufbx_vec3 ufbxTransformedNormal = ufbx_transform_direction(&ufbxNodeToWorld, ufbxNormal);
								importVertex.normal = normalize(float3(ufbxTransformedNormal.x, ufbxTransformedNormal.y, ufbxTransformedNormal.z));
							}

							if (hasTangents)
							{
								ufbx_vec3 ufbxTangent = ufbx_get_vertex_vec3(&ufbxMesh->vertex_tangent, ufbxVertexIndex);
								importVertex.tangent = normalize(float3(ufbxTangent.x, ufbxTangent.y, ufbxTangent.z));
							}

							if (hasUVs)
							{
								ufbx_vec2 ufbxUV = ufbx_get_vertex_vec2(&ufbxMesh->uv_sets[0].vertex_uv, ufbxVertexIndex);
								importVertex.uv = float2(ufbxUV.x, 1.0f - ufbxUV.y);
							}

							if (hasColors)
							{
								ufbx_vec4 ufbxColor = ufbx_get_vertex_vec4(&ufbxMesh->color_sets[0].vertex_color, ufbxVertexIndex);
								importVertex.color = float4(ufbxColor.x, ufbxColor.y, ufbxColor.z, ufbxColor.w);
							}

							importMesh.vertices[currentVertex] = importVertex;
							triangle.indices[triangleVertexIndex] = currentVertex;

							currentVertex++;
						}

						currentTriangle++;
					}
				}

				CrAssertMsg(currentVertex > 0, "No vertices present");
				CrAssertMsg(currentTriangle > 0, "No triangles present");

				CrAssertMsg(currentVertex == importMesh.vertices.size(), "Incorrect vertex count");
				CrAssertMsg(currentTriangle == importMesh.triangles.size(), "Incorrect triangle count");

				// Compute Tangent space if needed
				if (!hasTangents)
				{
					SMikkTSpaceInterface mikkTSpaceInterface;
					mikkTSpaceInterface.m_getNumFaces = MikkTSpaceGetNumFaces;
					mikkTSpaceInterface.m_getNumVerticesOfFace = MikkTSpaceGetNumVerticesOfFace;
					mikkTSpaceInterface.m_getPosition = MikkTSpaceGetPosition;
					mikkTSpaceInterface.m_getNormal = MikkTSpaceGetNormal;
					mikkTSpaceInterface.m_getTexCoord = MikkTSpaceGetTexCoord;
					mikkTSpaceInterface.m_setTSpaceBasic = MikkTSpaceSetTSpaceBasic;
					mikkTSpaceInterface.m_setTSpace = nullptr;

					SMikkTSpaceContext mikktSpaceContext;
					mikktSpaceContext.m_pInterface = &mikkTSpaceInterface;
					mikktSpaceContext.m_pUserData = (void*)&importMesh;

					genTangSpaceDefault(&mikktSpaceContext);
				}

				// 1. Remove duplicated vertices
				{
					CrVector<uint32_t> remappingTable;
					size_t indexCount = importMesh.triangles.size() * 3;
					size_t unindexedVertexCount = importMesh.vertices.size();
					remappingTable.resize_uninitialized(indexCount);
					size_t indexedVertexCount = meshopt_generateVertexRemap(remappingTable.data(), nullptr, indexCount, importMesh.vertices.data(), indexCount, sizeof(CrImportVertex));

					meshopt_remapIndexBuffer(importMesh.indices.data(), nullptr, indexCount, remappingTable.data());
					meshopt_remapVertexBuffer(importMesh.vertices.data(), importMesh.vertices.data(), unindexedVertexCount, sizeof(CrImportVertex), remappingTable.data());

					CrAssertMsg(importMesh.vertices.size() >= indexedVertexCount, "Vertex buffer remapping failed");

					// Resize vertices to correct size after deduplication
					importMesh.vertices.resize(indexedVertexCount);
				}

				// 2. Optimize vertex cache

				meshopt_optimizeVertexCache(importMesh.indices.data(), importMesh.indices.data(), importMesh.indices.size(), importMesh.vertices.size());

				float3 minVertex = float3(FLT_MAX);
				float3 maxVertex = float3(-FLT_MAX);

				CrVertexBufferHandle positionBuffer = renderDevice->CreateVertexBuffer(cr3d::MemoryAccess::GPUOnlyRead, PositionVertexDescriptor, (uint32_t)importMesh.vertices.size());
				CrVertexBufferHandle additionalBuffer = renderDevice->CreateVertexBuffer(cr3d::MemoryAccess::GPUOnlyRead, AdditionalVertexDescriptor, (uint32_t)importMesh.vertices.size());

				ComplexVertexPosition* positionBufferData = (ComplexVertexPosition*)renderDevice->BeginBufferUpload(positionBuffer->GetHardwareBuffer());
				ComplexVertexAdditional* additionalBufferData = (ComplexVertexAdditional*)renderDevice->BeginBufferUpload(additionalBuffer->GetHardwareBuffer());
				{
					for (size_t vertexIndex = 0; vertexIndex < importMesh.vertices.size(); ++vertexIndex)
					{
						const CrImportVertex& importVertex = importMesh.vertices[vertexIndex];

						minVertex = min(minVertex, importVertex.position);
						maxVertex = max(maxVertex, importVertex.position);

						positionBufferData[vertexIndex].position = { (half)importVertex.position.x, (half)importVertex.position.y, (half)importVertex.position.z };

						if (hasNormals)
						{
							float3 normal = importVertex.normal * 127.0f;
							additionalBufferData[vertexIndex].normal =
							{
								(int8_t)normal.x,
								(int8_t)normal.y,
								(int8_t)normal.z,
								0
							};
						}
						else
						{
							additionalBufferData[vertexIndex].normal = { 0, 127, 0, 0 };
						}

						if (hasTangents)
						{
							float3 tangent = (importVertex.tangent * 0.5f + 0.5f) * 255.0f;
							additionalBufferData[vertexIndex].tangent =
							{
								(uint8_t)tangent.x,
								(uint8_t)tangent.y,
								(uint8_t)tangent.z,
								0
							};
						}
						else
						{
							additionalBufferData[vertexIndex].tangent = { 255, 0, 0, 0 };
						}

						if (hasUVs)
						{
							additionalBufferData[vertexIndex].uv = { (half)importVertex.uv.x, (half)importVertex.uv.y };
						}
						else
						{
							additionalBufferData[vertexIndex].uv = { 0.0_h, 0.0_h };
						}

						if (hasColors)
						{
							additionalBufferData[vertexIndex].color =
							{
								(uint8_t)(importVertex.color.x * 255.0f + 0.5f),
								(uint8_t)(importVertex.color.y * 255.0f + 0.5f),
								(uint8_t)(importVertex.color.z * 255.0f + 0.5f),
								(uint8_t)(importVertex.color.w * 255.0f + 0.5f)
							};
						}
						else
						{
							additionalBufferData[vertexIndex].color =
							{
								(uint8_t)255.0f,
								(uint8_t)255.0f,
								(uint8_t)255.0f,
								(uint8_t)255.0f
							};
						}
					}
				}
				renderDevice->EndBufferUpload(positionBuffer->GetHardwareBuffer());
				renderDevice->EndBufferUpload(additionalBuffer->GetHardwareBuffer());

				renderMesh->AddVertexBuffer(positionBuffer);
				renderMesh->AddVertexBuffer(additionalBuffer);

				renderMesh->SetBoundingBox(CrBoundingBox((maxVertex + minVertex) * 0.5f, (maxVertex - minVertex) * 0.5f));

				CrIndexBufferHandle indexBuffer = renderDevice->CreateIndexBuffer(cr3d::MemoryAccess::GPUOnlyRead, cr3d::DataFormat::R16_Uint, (uint32_t)importMesh.triangles.size() * 3);

				uint16_t* indexData = (uint16_t*)renderDevice->BeginBufferUpload(indexBuffer->GetHardwareBuffer());
				{
					//size_t index = 0;
					//for (size_t triangleIndex = 0; triangleIndex < importMesh.triangles.size(); ++triangleIndex)
					//{
					//	const CrImportTriangle& triangle = importMesh.triangles[triangleIndex];
					//	indexData[index++] = (uint16_t)triangle.indices[0];
					//	indexData[index++] = (uint16_t)triangle.indices[1];
					//	indexData[index++] = (uint16_t)triangle.indices[2];
					//}

					for (size_t vertexIndex = 0; vertexIndex < importMesh.indices.size(); ++vertexIndex)
					{
						indexData[vertexIndex] = (uint16_t)importMesh.indices[vertexIndex];
					}
				}
				renderDevice->EndBufferUpload(indexBuffer->GetHardwareBuffer());

				renderMesh->SetIndexBuffer(indexBuffer);

				ufbx_material* ufbxMaterial = ufbxMeshMaterial->material;
				
				auto materialIter = materialMap.find(ufbxMaterial);
				
				if (materialIter != materialMap.end())
				{
					modelDescriptor.AddRenderMesh(renderMesh, (uint8_t)materialIter->second);
				}
				else
				{
					CrLog("Material not found");
				}
			}
		}
	}

	return CrRenderModelHandle(new CrRenderModel(modelDescriptor));
}