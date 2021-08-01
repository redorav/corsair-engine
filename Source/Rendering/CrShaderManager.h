#pragma once

#include "Core/CrCoreForwardDeclarations.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

class ICrRenderDevice;

struct ConstantBufferMetadata;
struct TextureMetadata;
struct SamplerMetadata;

struct CrShaderCompilationDescriptor;
struct CrShaderBytecodeCompilationDescriptor;
class ICrShaderBindingTable;

class CrShaderManager
{
public:

	static CrShaderManager* Get();

	void Init(const ICrRenderDevice* renderDevice);

	CrShaderBytecodeSharedHandle CompileShaderBytecode(const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor) const;

	CrGraphicsShaderHandle CompileGraphicsShader(const CrShaderCompilationDescriptor& bytecodeLoadDescriptor) const;

	CrComputeShaderHandle CompileComputeShader(const CrShaderCompilationDescriptor& bytecodeLoadDescriptor) const;

protected:

	const ICrRenderDevice* m_renderDevice;
};
