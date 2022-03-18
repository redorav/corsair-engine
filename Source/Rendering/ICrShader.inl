#pragma once

#include "Rendering/CrShaderResourceMetadata.h"

template<typename FunctionT>
inline void ICrShaderBindingLayout::AddResources(const CrShaderReflectionHeader& reflectionHeader, CrShaderBindingLayoutResources& resources, const FunctionT& function)
{
	for (uint32_t i = 0; i < reflectionHeader.resources.size(); ++i)
	{
		const CrShaderReflectionResource& resource = reflectionHeader.resources[i];

		cr3d::ShaderStage::T stage = reflectionHeader.shaderStage;

		switch (resource.type)
		{
			case cr3d::ShaderResourceType::ConstantBuffer:
			{
				const ConstantBufferMetadata& metadata = CrShaderMetadata::GetConstantBuffer(resource.name);
				resources.constantBuffers.push_back(CrShaderBinding(resource.bindPoint, stage, metadata.id));
				break;
			}
			case cr3d::ShaderResourceType::Sampler:
			{
				const SamplerMetadata& metadata = CrShaderMetadata::GetSampler(resource.name);
				resources.samplers.push_back(CrShaderBinding(resource.bindPoint, stage, metadata.id));
				break;
			}
			case cr3d::ShaderResourceType::Texture:
			{
				const TextureMetadata& metadata = CrShaderMetadata::GetTexture(resource.name);
				resources.textures.push_back(CrShaderBinding(resource.bindPoint, stage, metadata.id));
				break;
			}
			case cr3d::ShaderResourceType::RWTexture:
			{
				const RWTextureMetadata& metadata = CrShaderMetadata::GetRWTexture(resource.name);
				resources.rwTextures.push_back(CrShaderBinding(resource.bindPoint, stage, metadata.id));
				break;
			}
			case cr3d::ShaderResourceType::StorageBuffer:
			{
				const StorageBufferMetadata& metadata = CrShaderMetadata::GetStorageBuffer(resource.name);
				resources.storageBuffers.push_back(CrShaderBinding(resource.bindPoint, stage, metadata.id));
				break;
			}
			case cr3d::ShaderResourceType::RWStorageBuffer:
			{
				const RWStorageBufferMetadata& metadata = CrShaderMetadata::GetRWStorageBuffer(resource.name);
				resources.rwStorageBuffers.push_back(CrShaderBinding(resource.bindPoint, stage, metadata.id));
				break;
			}
			case cr3d::ShaderResourceType::RWDataBuffer:
			{
				const RWDataBufferMetadata& metadata = CrShaderMetadata::GetRWDataBuffer(resource.name);
				resources.rwDataBuffers.push_back(CrShaderBinding(resource.bindPoint, stage, metadata.id));
				break;
			}
		}

		function(stage, resource);
	}
}
