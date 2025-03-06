#include "Rendering/CrRendering_pch.h"

#include "CrShaderResourceMetadata.h"
#include "GeneratedShaders/ShaderMetadata.h"

const ConstantBufferMetadata& CrShaderMetadata::GetConstantBuffer(const crstl::string& name)
{
	auto cBuffer = ConstantBufferTable.find(name);

	if (cBuffer != ConstantBufferTable.end())
	{
		return (*cBuffer).second;
	}

	return InvalidConstantBufferMetaInstance;
}

const ConstantBufferMetadata& CrShaderMetadata::GetConstantBuffer(ConstantBuffers::T id)
{
	return ConstantBufferMetaTable[id];
}

const SamplerMetadata& CrShaderMetadata::GetSampler(const crstl::string& name)
{
	auto samplerMetadata = SamplerTable.find(name);

	if (samplerMetadata != SamplerTable.end())
	{
		return (*samplerMetadata).second;
	}

	return InvalidSamplerMetaInstance;
}

const SamplerMetadata& CrShaderMetadata::GetSampler(Samplers::T id)
{
	return SamplerMetaTable[id];
}

const TextureMetadata& CrShaderMetadata::GetTexture(const crstl::string& name)
{
	auto textureMetadata = TextureTable.find(name);

	if (textureMetadata != TextureTable.end())
	{
		return (*textureMetadata).second;
	}

	return InvalidTextureMetaInstance;
}

const TextureMetadata& CrShaderMetadata::GetTexture(Textures::T id)
{
	return TextureMetaTable[id];
}

const RWTextureMetadata& CrShaderMetadata::GetRWTexture(const crstl::string& name)
{
	auto rwTextureMetadata = RWTextureTable.find(name);

	if (rwTextureMetadata != RWTextureTable.end())
	{
		return (*rwTextureMetadata).second;
	}

	return InvalidRWTextureMetaInstance;
}

const RWTextureMetadata& CrShaderMetadata::GetRWTexture(RWTextures::T id)
{
	return RWTextureMetaTable[id];
}

const StorageBufferMetadata& CrShaderMetadata::GetStorageBuffer(const crstl::string& name)
{
	auto storageBufferMetadata = StorageBufferTable.find(name);

	if (storageBufferMetadata != StorageBufferTable.end())
	{
		return (*storageBufferMetadata).second;
	}

	return InvalidStorageBufferMetaInstance;
}

const StorageBufferMetadata& CrShaderMetadata::GetStorageBuffer(StorageBuffers::T id)
{
	return StorageBufferMetaTable[id];
}

const RWStorageBufferMetadata& CrShaderMetadata::GetRWStorageBuffer(const crstl::string& name)
{
	auto rwStorageBufferMetadata = RWStorageBufferTable.find(name);

	if (rwStorageBufferMetadata != RWStorageBufferTable.end())
	{
		return (*rwStorageBufferMetadata).second;
	}

	return InvalidRWStorageBufferMetaInstance;
}

const RWStorageBufferMetadata& CrShaderMetadata::GetRWStorageBuffer(RWStorageBuffers::T id)
{
	return RWStorageBufferMetaTable[id];
}

const RWTypedBufferMetadata& CrShaderMetadata::GetRWTypedBuffer(const crstl::string& name)
{
	auto rwTypedBufferMetadata = RWTypedBufferTable.find(name);

	if (rwTypedBufferMetadata != RWTypedBufferTable.end())
	{
		return (*rwTypedBufferMetadata).second;
	}

	return InvalidRWTypedBufferMetaInstance;
}

const RWTypedBufferMetadata& CrShaderMetadata::GetRWTypedBuffer(RWTypedBuffers::T id)
{
	return RWTypedBufferMetaTable[id];
}