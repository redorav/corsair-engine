#pragma once

#include "Core/CrCoreForwardDeclarations.h"

#include "Graphics/CrGraphicsForwardDeclarations.h"

class IDevice;

struct CrShaderCompilationDescriptor;
struct CrShaderBytecodeCompilationDescriptor;
class ICrShaderBindingLayout;

class CrShaderManager
{
public:

	static const char* GetShaderBytecodeExtension(crgfx::GraphicsApi::T graphicsApi);

	static void Initialize();

	static void Deinitialize();

	CrShaderBytecodeHandle CompileShaderBytecode(const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor) const;

	CrShaderBytecodeHandle CompileShaderBytecode(const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor, const CrShaderCompilerDefines& defines) const;

	CrGraphicsShaderHandle CompileGraphicsShader(const CrShaderCompilationDescriptor& bytecodeLoadDescriptor) const;

	CrComputeShaderHandle CompileComputeShader(const CrShaderCompilationDescriptor& bytecodeLoadDescriptor) const;

	CrFixedPath GetCompiledShadersPath(cr::Platform::T platform, crgfx::GraphicsApi::T graphicsApi) const;
};

extern CrShaderManager* ShaderManager;