#pragma once

#include "Core/CrCoreForwardDeclarations.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

class ICrRenderDevice;

struct ConstantBufferMetadata;
struct TextureMetadata;
struct SamplerMetadata;

struct CrBytecodeLoadDescriptor;
struct CrShaderBytecodeDescriptor;
class ICrShaderBindingTable;

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

	CrComputeShaderHandle LoadComputeShader(const CrBytecodeLoadDescriptor& bytecodeLoadDescriptor) const;

protected:

	const ICrRenderDevice* m_renderDevice;
};
