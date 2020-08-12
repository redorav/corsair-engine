#pragma once

#include "ICrShaderReflection.h"

#include "Core/CrCoreForwardDeclarations.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

class ICrRenderDevice;
struct CrGraphicsShaderCreate;
class CrShaderReflectionVulkan;
class CrShaderResourceSet;
struct ConstantBufferMetadata;
struct TextureMetadata;
struct SamplerMetadata;

struct CrGraphicsShaderCreate;

struct CrShaderBytecodeDescriptor;

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

	CrShaderBytecodeSharedHandle CompileShaderBytecode(const CrFileSharedHandle& file, const CrShaderBytecodeDescriptor& bytecodeDescriptor) const;

	CrGraphicsShaderHandle LoadGraphicsShader(const CrGraphicsShaderCreate& shaderCreateInfo) const;

	// Metadata query functions

	static ConstantBufferMetadata& GetConstantBufferMetadata(const CrString& name);

	static ConstantBufferMetadata& GetConstantBufferMetadata(ConstantBuffers::T id);

	static TextureMetadata& GetTextureMetadata(const CrString& name);

	static TextureMetadata& GetTextureMetadata(Textures::T id);

	static SamplerMetadata& GetSamplerMetadata(const CrString& name);

	static SamplerMetadata& GetSamplerMetadata(Samplers::T id);

protected:

	// TODO Remove this reference to shader reflection vulkan
	void CreateShaderResourceSet(const CrGraphicsShaderCreate& shaderCreateInfo, const CrShaderReflectionVulkan& reflection, CrShaderResourceSet& resourceSet) const;

	virtual void InitPS() = 0;

	virtual void CreateShaderResourceSetPS(const CrGraphicsShaderCreate& shaderCreateInfo, const CrShaderReflectionVulkan& reflection, CrShaderResourceSet& resourceSet) const = 0;

	const ICrRenderDevice* m_renderDevice;
};
