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

	static CrShaderManager& Get();

	static const char* GetShaderBytecodeExtension(cr3d::GraphicsApi::T graphicsApi);

	void Initialize(const ICrRenderDevice* renderDevice);

	const ICrRenderDevice* GetRenderDevice() const { return m_renderDevice; }

	CrShaderBytecodeSharedHandle CompileShaderBytecode(const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor) const;

	CrShaderBytecodeSharedHandle CompileShaderBytecode(const CrShaderBytecodeCompilationDescriptor& bytecodeDescriptor, const CrShaderDefines& defines) const;

	CrGraphicsShaderHandle CompileGraphicsShader(const CrShaderCompilationDescriptor& bytecodeLoadDescriptor) const;

	CrComputeShaderHandle CompileComputeShader(const CrShaderCompilationDescriptor& bytecodeLoadDescriptor) const;

	CrPath GetShaderCachePath(cr::Platform::T platform, cr3d::GraphicsApi::T graphicsApi) const;

protected:

	const ICrRenderDevice* m_renderDevice;
};
