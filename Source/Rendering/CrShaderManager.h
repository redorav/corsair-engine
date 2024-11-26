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

	static const char* GetShaderBytecodeExtension(cr3d::GraphicsApi::T graphicsApi);

	static void Initialize(ICrRenderDevice* renderDevice);

	static void Deinitialize();

	ICrRenderDevice* GetRenderDevice() const { return m_renderDevice; }

	CrShaderBytecodeHandle CompileShaderBytecode(const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor) const;

	CrShaderBytecodeHandle CompileShaderBytecode(const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor, const CrShaderCompilerDefines& defines) const;

	CrGraphicsShaderHandle CompileGraphicsShader(const CrShaderCompilationDescriptor& bytecodeLoadDescriptor) const;

	CrComputeShaderHandle CompileComputeShader(const CrShaderCompilationDescriptor& bytecodeLoadDescriptor) const;

	CrFixedPath GetCompiledShadersPath(cr::Platform::T platform, cr3d::GraphicsApi::T graphicsApi) const;

protected:

	CrShaderManager(ICrRenderDevice* renderDevice);

	ICrRenderDevice* m_renderDevice;
};

extern CrShaderManager* ShaderManager;