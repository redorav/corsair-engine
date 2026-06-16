#pragma once

#include "Core/CrCoreForwardDeclarations.h"

#include "Graphics/CrGraphicsForwardDeclarations.h"

class IDevice;

struct CrShaderCompilationDescriptor;
struct CrShaderBytecodeCompilationDescriptor;

class CrShaderManager
{
public:

	static const char* GetShaderBytecodeExtension(crgfx::GraphicsApi::T graphicsApi);

	static void Initialize();

	static void Deinitialize();

	crgfx::ShaderBytecodeHandle CompileShaderBytecode(const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor) const;

	crgfx::ShaderBytecodeHandle CompileShaderBytecode(const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor, const CrShaderCompilerDefines& defines) const;

	crgfx::CrGraphicsShaderHandle CompileGraphicsShader(const CrShaderCompilationDescriptor& bytecodeLoadDescriptor) const;

	crgfx::CrComputeShaderHandle CompileComputeShader(const CrShaderCompilationDescriptor& bytecodeLoadDescriptor) const;

	CrFixedPath GetCompiledShadersPath(cr::Platform::T platform, crgfx::GraphicsApi::T graphicsApi) const;
};

extern CrShaderManager* ShaderManager;