#pragma once

#include "ShaderResources.h"

class CrShaderMetadata
{
public:

	static const ConstantBufferMetadata& GetConstantBuffer(const CrString& name)
	{
		auto cBuffer = ConstantBufferTable.find(name);

		if (cBuffer != ConstantBufferTable.end())
		{
			return (*cBuffer).second;
		}

		return InvalidConstantBufferMetaInstance;
	}

	static const ConstantBufferMetadata& GetConstantBuffer(ConstantBuffers::T id)
	{
		return ConstantBufferMetaTable[id];
	}

	static const TextureMetadata& GetTexture(const CrString& name)
	{
		auto textureMetadata = TextureTable.find(name);

		if (textureMetadata != TextureTable.end())
		{
			return (*textureMetadata).second;
		}

		return InvalidTextureMetaInstance;
	}

	static const TextureMetadata& GetTexture(Textures::T id)
	{
		return TextureMetaTable[id];
	}

	static const SamplerMetadata& GetSampler(const CrString& name)
	{
		auto samplerMetadata = SamplerTable.find(name);

		if (samplerMetadata != SamplerTable.end())
		{
			return (*samplerMetadata).second;
		}

		return InvalidSamplerMetaInstance;
	}

	static const SamplerMetadata& GetSampler(Samplers::T id)
	{
		return SamplerMetaTable[id];
	}
};