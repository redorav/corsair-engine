#include "Resource/CrResource_pch.h"

#include "CrModelDecoderCGLTF.h"

#include "Core/FileSystem/CrFixedPath.h"
#include "Core/FileSystem/ICrFile.h"
#include "Core/Containers/CrHashMap.h"

#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/CrMaterialCompiler.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/CrRenderModel.h"
#include "Rendering/CrRenderMesh.h"
#include "Rendering/ICrTexture.h"
#include "Rendering/CrImage.h"
#include "Rendering/CrCommonVertexLayouts.h"
#include "Rendering/CrRenderingForwardDeclarations.h"

#include "GeneratedShaders/ShaderMetadata.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

cr3d::DataFormat::T ToDataFormat(cgltf_component_type componentType)
{
	switch (componentType)
	{
		case cgltf_component_type_r_8:		return cr3d::DataFormat::R8_Sint;
		case cgltf_component_type_r_8u:		return cr3d::DataFormat::R8_Uint;
		case cgltf_component_type_r_16:		return cr3d::DataFormat::R16_Sint;
		case cgltf_component_type_r_16u:	return cr3d::DataFormat::R16_Uint;
		case cgltf_component_type_r_32u:	return cr3d::DataFormat::R32_Uint;
		case cgltf_component_type_r_32f:	return cr3d::DataFormat::R32_Float;
		default: break;
	}
	CrAssertMsg(false, "Failed to convert data format: %i", componentType);
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

CrRenderMeshHandle LoadMesh(const cgltf_primitive& gltfPrimitive)
{
	CrRenderMeshHandle mesh = CrRenderMeshHandle(new CrRenderMesh());

	// Index data
	if (gltfPrimitive.indices != nullptr)
	{
		const cgltf_accessor* gltfIndexAccessor = gltfPrimitive.indices;
	
		// Create the buffer
		cr3d::DataFormat::T format = ToDataFormat(gltfIndexAccessor->component_type);
		CrIndexBufferHandle indexBuffer = RenderSystem->GetRenderDevice()->CreateIndexBuffer(cr3d::MemoryAccess::CPUStreamToGPU, format, (uint32_t)gltfIndexAccessor->count);

		// Use the buffer view to copy the data
		const cgltf_buffer_view* gltfBufferView = gltfIndexAccessor->buffer_view;
		const unsigned char* data = (const unsigned char*)gltfBufferView->buffer->data;
		data = data + (gltfIndexAccessor->offset + gltfBufferView->offset);
	
		CrAssertMsg(gltfBufferView->stride == 0, "Invalid stride");
		void* indexData = indexBuffer->Lock();
		memcpy(indexData, data, gltfBufferView->size);
		indexBuffer->Unlock();
	
		mesh->SetIndexBuffer(indexBuffer);
	}
	
	// Vertex data
	if (gltfPrimitive.attributes_count > 0)
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
		CrVector<GLTFFloat3> colors;
		CrVector<GLTFFloat2> texCoords;

		for (uint32_t i = 0; i < gltfPrimitive.attributes_count; ++i)
		{
			const cgltf_attribute& gltfAttribute = gltfPrimitive.attributes[i];
			const cgltf_buffer_view* gltfBufferView = gltfAttribute.data->buffer_view;
			const unsigned char* data = (const unsigned char*)gltfBufferView->buffer->data;
			data = data + (gltfAttribute.data->offset + gltfBufferView->offset);

			uint32_t componentSize = (uint32_t)(cgltf_num_components(gltfAttribute.data->type) * cgltf_component_size(gltfAttribute.data->component_type));

			if (gltfAttribute.type == cgltf_attribute_type_position)
			{
				CrAssert(gltfAttribute.data->type == cgltf_type_vec3 && gltfAttribute.data->component_type == cgltf_component_type_r_32f);
				positions.resize(gltfAttribute.data->count);
				LoadAttribute<GLTFFloat3>(positions, data, gltfAttribute.data->count, componentSize, gltfBufferView->stride);
			}
			else if (gltfAttribute.type == cgltf_attribute_type_normal)
			{
				CrAssert(gltfAttribute.data->type == cgltf_type_vec3 && gltfAttribute.data->component_type == cgltf_component_type_r_32f);
				normals.resize(gltfAttribute.data->count);
				LoadAttribute<GLTFFloat3>(normals, data, gltfAttribute.data->count, componentSize, gltfBufferView->stride);
			}
			else if (gltfAttribute.type == cgltf_attribute_type_texcoord)
			{
				CrAssert(gltfAttribute.data->type == cgltf_type_vec2 && gltfAttribute.data->component_type == cgltf_component_type_r_32f);
				texCoords.resize(gltfAttribute.data->count);
				LoadAttribute<GLTFFloat2>(texCoords, data, gltfAttribute.data->count, componentSize, gltfBufferView->stride);
			}
			else if (gltfAttribute.type == cgltf_attribute_type_color)
			{
				CrAssert(gltfAttribute.data->type == cgltf_type_vec3 && gltfAttribute.data->component_type == cgltf_component_type_r_32f);
				colors.resize(gltfAttribute.data->count);
				LoadAttribute<GLTFFloat3>(colors, data, gltfAttribute.data->count, componentSize, gltfBufferView->stride);
			}
		}

		bool hasPositions = !positions.empty();
		bool hasNormals = !normals.empty();
		bool hasTextureCoords = !texCoords.empty();
		bool hasColor = !colors.empty();

		float3 minVertex = float3(FLT_MAX);
		float3 maxVertex = float3(-FLT_MAX);

		// Create the vertex buffer
		CrVertexBufferHandle positionBuffer = RenderSystem->GetRenderDevice()->CreateVertexBuffer(cr3d::MemoryAccess::CPUStreamToGPU, PositionVertexDescriptor, (uint32_t)positions.size());
		CrVertexBufferHandle additionalBuffer = RenderSystem->GetRenderDevice()->CreateVertexBuffer(cr3d::MemoryAccess::CPUStreamToGPU, AdditionalVertexDescriptor, (uint32_t)positions.size());

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

				if (hasColor)
				{
					const GLTFFloat3& color = normals[vertexIndex];

					additionalBufferData[vertexIndex].color =
					{
						(uint8_t)(color.x * 255.0f),
						(uint8_t)(color.y * 255.0f),
						(uint8_t)(color.z * 255.0f),
						0
					};
				}
				else
				{
					additionalBufferData[vertexIndex].color = { 255, 255, 255, 255 };
				}
			}
		}
		positionBuffer->Unlock();
		additionalBuffer->Unlock();

		mesh->AddVertexBuffer(positionBuffer);
		mesh->AddVertexBuffer(additionalBuffer);

		mesh->SetBoundingBox(CrBoundingBox((maxVertex + minVertex) * 0.5f, (maxVertex - minVertex) * 0.5f));
	}

	return mesh;
}

struct CgltfUserData
{
	CrFixedPath parentPath;
};

static cgltf_result cgltfFileRead(const struct cgltf_memory_options* memory_options, const struct cgltf_file_options* file_options, const char* path, cgltf_size* size, void** data)
{
	const CgltfUserData& userData = *(const CgltfUserData*)(file_options->user_data);
	CrFixedPath resourcePath = userData.parentPath / path;

	void* (*memory_alloc)(void*, cgltf_size) = memory_options->alloc ? memory_options->alloc : &cgltf_default_alloc;
	void (*memory_free)(void*, void*) = memory_options->free ? memory_options->free : &cgltf_default_free;

	CrFileHandle file = ICrFile::OpenFile(resourcePath.c_str(), FileOpenFlags::Read);

	if (!file)
	{
		return cgltf_result_file_not_found;
	}

	cgltf_size fileSize = size ? *size : file->GetSize();

	if (fileSize == 0)
	{
		return cgltf_result_io_error;
	}

	char* fileData = (char*)memory_alloc(memory_options->user_data, fileSize);
	if (!fileData)
	{
		return cgltf_result_out_of_memory;
	}

	cgltf_size readSize = file->Read(fileData, fileSize);

	if (readSize != fileSize)
	{
		memory_free(memory_options->user_data, fileData);
		return cgltf_result_io_error;
	}

	if (size)
	{
		*size = fileSize;
	}
	if (data)
	{
		*data = fileData;
	}

	return cgltf_result_success;
}

static void cgltfFileRelease(const struct cgltf_memory_options* memory_options, const struct cgltf_file_options* file_options, void* data)
{
	(void)file_options;
	void (*memfree)(void*, void*) = memory_options->free ? memory_options->free : &cgltf_default_free;
	memfree(memory_options->user_data, data);
}

CrRenderModelHandle CrModelDecoderCGLTF::Decode(const CrFileHandle& file)
{
	uint64_t fileSize = file->GetSize();
	CrUniquePtr<uint8_t[]> fileData = CrUniquePtr<uint8_t[]>(new uint8_t[fileSize]);
	file->Read(fileData.get(), fileSize);

	CgltfUserData userData;
	userData.parentPath = CrFixedPath(file->GetFilePath()).parent_path();

	cgltf_options gltfOptions = {};
	gltfOptions.file.read = cgltfFileRead;
	gltfOptions.file.release = cgltfFileRelease;
	gltfOptions.file.user_data = &userData;

	cgltf_data* gltfData = nullptr;
	cgltf_result gltfResult = cgltf_parse(&gltfOptions, fileData.get(), fileSize, &gltfData);
	if (gltfResult == cgltf_result_success)
	{
		gltfResult = cgltf_load_buffers(&gltfOptions, gltfData, "");

		CrRenderModelDescriptor modelDescriptor;

		CrHashMap<void*, CrTextureHandle> textureMap;

		for (uint32_t t = 0; t < gltfData->textures_count; ++t)
		{
			const cgltf_texture& gltfTexture = gltfData->textures[t];

			CrFixedPath imagePath = userData.parentPath / gltfTexture.image->uri;

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

			textureMap.insert(&gltfData->textures[t], texture);
		}

		const auto ProcessMaterialTexture = [&](const CrMaterialHandle& material, void* cgltfKey, Textures::T semantic)
		{
			const auto textureIter = textureMap.find(cgltfKey);
			if (textureIter != textureMap.end())
			{
				const CrTextureHandle& texture = textureIter->second;
				material->AddTexture(texture, semantic);
			}
		};

		CrHashMap<void*, uint32_t> materialMap;

		// Load materials. Store materials in table to meshes can index into them
		for (uint32_t m = 0; m < gltfData->materials_count; ++m)
		{
			const cgltf_material& cgltfMaterial = gltfData->materials[m];
			CrMaterialDescriptor materialDescriptor;
			CrMaterialHandle material = MaterialCompiler->CompileMaterial(materialDescriptor);

			ProcessMaterialTexture(material, cgltfMaterial.pbr_metallic_roughness.base_color_texture.texture, Textures::DiffuseTexture0);
			ProcessMaterialTexture(material, cgltfMaterial.pbr_metallic_roughness.metallic_roughness_texture.texture, Textures::SpecularTexture0);
			ProcessMaterialTexture(material, cgltfMaterial.normal_texture.texture, Textures::NormalTexture0);
			//ProcessMaterialTexture(material, cgltfMaterial.pbr_metallic_roughness.base_color_texture.texture, Textures::DiffuseTexture0);
			//ProcessMaterialTexture(material, cgltfMaterial.pbr_metallic_roughness.base_color_texture.texture, Textures::DiffuseTexture0);

			modelDescriptor.AddMaterial(material);

			materialMap.insert(&gltfData->materials[m], m);
		}

		// Create the render model and setup the material. Note that by now, images have been loaded
		// and textures created within the loader callback
		for (uint32_t m = 0; m < gltfData->meshes_count; ++m)
		{
			const cgltf_mesh& gltfMesh = gltfData->meshes[m];

			for (uint32_t p = 0; p < gltfMesh.primitives_count; ++p)
			{
				const cgltf_primitive& cgltfPrimitive = gltfMesh.primitives[p];
				CrRenderMeshHandle renderMesh = LoadMesh(cgltfPrimitive);

				// Find material
				const auto materialIndexIter = materialMap.find(cgltfPrimitive.material);
				uint8_t materialIndex = 0;

				if (materialIndexIter != materialMap.end())
				{
					materialIndex = (uint8_t)materialIndexIter->second;
				}

				modelDescriptor.AddRenderMesh(renderMesh, materialIndex);
			}
		}

		cgltf_free(gltfData);

		return CrRenderModelHandle(new CrRenderModel(modelDescriptor));
	}
	else
	{
		return nullptr;
	}
}