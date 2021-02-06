#pragma once

#include "GeneratedShaders/ShaderMetadata.h"

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

	static const RWTextureMetadata& GetRWTexture(const CrString& name)
	{
		auto rwTextureMetadata = RWTextureTable.find(name);

		if (rwTextureMetadata != RWTextureTable.end())
		{
			return (*rwTextureMetadata).second;
		}

		return InvalidRWTextureMetaInstance;
	}

	static const RWTextureMetadata& GetRWTexture(RWTextures::T id)
	{
		return RWTextureMetaTable[id];
	}

	static const RWStorageBufferMetadata& GetRWStorageBuffer(const CrString& name)
	{
		auto rwStorageBufferMetadata = RWStorageBufferTable.find(name);

		if (rwStorageBufferMetadata != RWStorageBufferTable.end())
		{
			return (*rwStorageBufferMetadata).second;
		}

		return InvalidRWStorageBufferMetaInstance;
	}

	static const RWStorageBufferMetadata& GetRWStorageBuffer(RWStorageBuffers::T id)
	{
		return RWStorageBufferMetaTable[id];
	}

	static const RWDataBufferMetadata& GetRWDataBuffer(const CrString& name)
	{
		auto rwDataBufferMetadata = RWDataBufferTable.find(name);

		if (rwDataBufferMetadata != RWDataBufferTable.end())
		{
			return (*rwDataBufferMetadata).second;
		}

		return InvalidRWDataBufferMetaInstance;
	}

	static const RWDataBufferMetadata& GetRWDataBuffer(RWDataBuffers::T id)
	{
		return RWDataBufferMetaTable[id];
	}
};