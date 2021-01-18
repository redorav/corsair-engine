#pragma once

#include "ICrShaderReflection.h"

#include "Core/CrCoreForwardDeclarations.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

class ICrRenderDevice;

struct ConstantBufferMetadata;
struct TextureMetadata;
struct SamplerMetadata;

struct CrBytecodeLoadDescriptor;
struct CrShaderBytecodeDescriptor;
class CrShaderReflectionVulkan;
class CrShaderResourceTable;

class CrShaderManagerVulkan;

namespace ConstantBuffers { enum T : uint8_t; }
namespace Textures { enum T : uint8_t; }
namespace Samplers { enum T : uint8_t; }

class ICrShaderManager
{
public:

	static ICrShaderManager* Get();

	void Init(const ICrRenderDevice* renderDevice);

	CrShaderBytecodeSharedHandle LoadShaderBytecode(const CrPath& path, const CrShaderBytecodeDescriptor& bytecodeDescriptor) const;

	CrShaderBytecodeSharedHandle LoadShaderBytecode(const CrFileSharedHandle& file, const CrShaderBytecodeDescriptor& bytecodeDescriptor) const;

	CrShaderBytecodeSharedHandle CompileShaderBytecode(const CrShaderBytecodeDescriptor& bytecodeDescriptor) const;

	CrGraphicsShaderHandle LoadGraphicsShader(const CrBytecodeLoadDescriptor& shaderCreateInfo) const;

	// Metadata query functions

	static const ConstantBufferMetadata& GetConstantBufferMetadata(const CrString& name);

	static const ConstantBufferMetadata& GetConstantBufferMetadata(ConstantBuffers::T id);

	static const TextureMetadata& GetTextureMetadata(const CrString& name);

	static const TextureMetadata& GetTextureMetadata(Textures::T id);

	static const SamplerMetadata& GetSamplerMetadata(const CrString& name);

	static const SamplerMetadata& GetSamplerMetadata(Samplers::T id);

protected:

	// TODO Remove this reference to shader reflection vulkan
	void CreateShaderResourceTable(const CrShaderReflectionVulkan& reflection, CrShaderResourceTable& resourceTable) const;

	virtual void InitPS() = 0;

	virtual void CreateShaderResourceTablePS(const CrShaderReflectionVulkan& reflection, CrShaderResourceTable& resourceTable) const = 0;

	const ICrRenderDevice* m_renderDevice;
};
