#pragma once

#include "Core/CrCoreForwardDeclarations.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

class ICrRenderDevice;

struct CrShaderCompilationDescriptor;
struct CrShaderBytecodeCompilationDescriptor;
class ICrShaderBindingLayout;

class CrShaderManager
{
public:

	static CrShaderManager& Get();

	static const char* GetShaderBytecodeExtension(cr3d::GraphicsApi::T graphicsApi);

	void Initialize(ICrRenderDevice* renderDevice);

	ICrRenderDevice* GetRenderDevice() const { return m_renderDevice; }

	CrShaderBytecodeSharedHandle CompileShaderBytecode(const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor) const;

	CrShaderBytecodeSharedHandle CompileShaderBytecode(const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor, const CrShaderCompilerDefines& defines) const;

	CrGraphicsShaderHandle CompileGraphicsShader(const CrShaderCompilationDescriptor& bytecodeLoadDescriptor) const;

	CrComputeShaderHandle CompileComputeShader(const CrShaderCompilationDescriptor& bytecodeLoadDescriptor) const;

	CrPath GetCompiledShadersPath(cr::Platform::T platform, cr3d::GraphicsApi::T graphicsApi) const;

protected:

	ICrRenderDevice* m_renderDevice;
};
