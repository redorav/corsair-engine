#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Rendering/CrRenderingForwardDeclarations.h"

class CrShaderMetadata
{
public:

	static const ConstantBufferMetadata& GetConstantBuffer(const crstl::string& name);

	static const ConstantBufferMetadata& GetConstantBuffer(ConstantBuffers::T id);

	static const SamplerMetadata& GetSampler(const crstl::string& name);

	static const SamplerMetadata& GetSampler(Samplers::T id);

	static const TextureMetadata& GetTexture(const crstl::string& name);

	static const TextureMetadata& GetTexture(Textures::T id);

	static const RWTextureMetadata& GetRWTexture(const crstl::string& name);

	static const RWTextureMetadata& GetRWTexture(RWTextures::T id);

	static const StorageBufferMetadata& GetStorageBuffer(const crstl::string& name);

	static const StorageBufferMetadata& GetStorageBuffer(StorageBuffers::T id);

	static const RWStorageBufferMetadata& GetRWStorageBuffer(const crstl::string& name);

	static const RWStorageBufferMetadata& GetRWStorageBuffer(RWStorageBuffers::T id);

	static const RWTypedBufferMetadata& GetRWTypedBuffer(const crstl::string& name);

	static const RWTypedBufferMetadata& GetRWTypedBuffer(RWTypedBuffers::T id);
};