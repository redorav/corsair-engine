#pragma once

#include "Graphics/CrShaderResourceMetadata.h"
#include "GeneratedShaders/ShaderMetadata.h"

namespace crgfx
{
	template<typename FunctionT>
	inline void ShaderBindingLayout::AddResources(const CrShaderReflectionHeader& reflectionHeader, crgfx::ShaderBindingLayoutResources& resources, const FunctionT& function)
	{
		for (const CrShaderReflectionResource& resource : reflectionHeader.constantBuffers)
		{
			const ConstantBufferMetadata& metadata = CrShaderMetadata::GetConstantBuffer(resource.name);
			resources.constantBuffers.push_back(crgfx::ShaderBinding(resource.bindPoint, reflectionHeader.shaderStage, metadata.id));
			function(reflectionHeader.shaderStage, resource);
		}

		for (const CrShaderReflectionResource& resource : reflectionHeader.samplers)
		{
			const SamplerMetadata& metadata = CrShaderMetadata::GetSampler(resource.name);
			resources.samplers.push_back(crgfx::ShaderBinding(resource.bindPoint, reflectionHeader.shaderStage, metadata.id));
			function(reflectionHeader.shaderStage, resource);
		}

		for (const CrShaderReflectionResource& resource : reflectionHeader.textures)
		{
			const TextureMetadata& metadata = CrShaderMetadata::GetTexture(resource.name);
			resources.textures.push_back(crgfx::ShaderBinding(resource.bindPoint, reflectionHeader.shaderStage, metadata.id));
			function(reflectionHeader.shaderStage, resource);
		}

		for (const CrShaderReflectionResource& resource : reflectionHeader.rwTextures)
		{
			const RWTextureMetadata& metadata = CrShaderMetadata::GetRWTexture(resource.name);
			resources.rwTextures.push_back(crgfx::ShaderBinding(resource.bindPoint, reflectionHeader.shaderStage, metadata.id));
			function(reflectionHeader.shaderStage, resource);
		}

		for (const CrShaderReflectionResource& resource : reflectionHeader.storageBuffers)
		{
			const StorageBufferMetadata& metadata = CrShaderMetadata::GetStorageBuffer(resource.name);
			resources.storageBuffers.push_back(crgfx::ShaderBinding(resource.bindPoint, reflectionHeader.shaderStage, metadata.id));
			function(reflectionHeader.shaderStage, resource);
		}

		for (const CrShaderReflectionResource& resource : reflectionHeader.rwStorageBuffers)
		{
			const RWStorageBufferMetadata& metadata = CrShaderMetadata::GetRWStorageBuffer(resource.name);
			resources.rwStorageBuffers.push_back(crgfx::ShaderBinding(resource.bindPoint, reflectionHeader.shaderStage, metadata.id));
			function(reflectionHeader.shaderStage, resource);
		}

		for (const CrShaderReflectionResource& resource : reflectionHeader.rwTypedBuffers)
		{
			const RWTypedBufferMetadata& metadata = CrShaderMetadata::GetRWTypedBuffer(resource.name);
			resources.rwTypedBuffers.push_back(crgfx::ShaderBinding(resource.bindPoint, reflectionHeader.shaderStage, metadata.id));
			function(reflectionHeader.shaderStage, resource);
		}
	}
};