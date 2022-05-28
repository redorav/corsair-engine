#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Rendering/CrRenderingForwardDeclarations.h"

class CrShaderMetadata
{
public:

	static const ConstantBufferMetadata& GetConstantBuffer(const CrString& name);

	static const ConstantBufferMetadata& GetConstantBuffer(ConstantBuffers::T id);

	static const SamplerMetadata& GetSampler(const CrString& name);

	static const SamplerMetadata& GetSampler(Samplers::T id);

	static const TextureMetadata& GetTexture(const CrString& name);

	static const TextureMetadata& GetTexture(Textures::T id);

	static const RWTextureMetadata& GetRWTexture(const CrString& name);

	static const RWTextureMetadata& GetRWTexture(RWTextures::T id);

	static const StorageBufferMetadata& GetStorageBuffer(const CrString& name);

	static const StorageBufferMetadata& GetStorageBuffer(StorageBuffers::T id);

	static const RWStorageBufferMetadata& GetRWStorageBuffer(const CrString& name);

	static const RWStorageBufferMetadata& GetRWStorageBuffer(RWStorageBuffers::T id);

	static const RWDataBufferMetadata& GetRWDataBuffer(const CrString& name);

	static const RWDataBufferMetadata& GetRWDataBuffer(RWDataBuffers::T id);
};