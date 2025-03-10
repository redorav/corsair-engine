#include "Rendering/CrRendering_pch.h"

#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/CrMaterialCompiler.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/CrShaderManager.h"
#include "Rendering/ICrShader.h"
#include "Rendering/CrShaderSources.h"

#include "Core/CrGlobalPaths.h"
#include "Core/CrPlatform.h"

crstl::string CrShaderHeaderGenerator::HashDefine = "#define ";

CrMaterialCompiler* MaterialCompiler = nullptr;

template<typename Enum>
const char* GetMaterialShaderEnum(Enum);

template<>
const char* GetMaterialShaderEnum(CrMaterialShaderVariant::T materialShaderVariant)
{
	switch (materialShaderVariant)
	{
		case CrMaterialShaderVariant::Depth:   return "EMaterialShaderVariant_Depth";
		case CrMaterialShaderVariant::GBuffer: return "EMaterialShaderVariant_GBuffer";
		case CrMaterialShaderVariant::Forward: return "EMaterialShaderVariant_Forward";
		case CrMaterialShaderVariant::Debug:   return "EMaterialShaderVariant_Debug";
		default: return "Invalid";
	}
}

void CrShaderHeaderGenerator::Define(const char* define)
{
	m_header += HashDefine + define + "\n";
}

void CrShaderHeaderGenerator::DefineString(const char* define, const char* string)
{
	m_header += HashDefine + define + string + "\n";
}

void CrShaderHeaderGenerator::DefineInt(const char* define, int value)
{
	m_header += HashDefine + define + " " + crstl::string(value) + "\n";
}

void CrMaterialCompiler::Initialize()
{
	CrAssert(MaterialCompiler == nullptr);
	MaterialCompiler = new CrMaterialCompiler();
}

void CrMaterialCompiler::Deinitialize()
{
	CrAssert(MaterialCompiler != nullptr);
	delete MaterialCompiler;
}

CrMaterialCompiler::CrMaterialCompiler()
{
	CrAssert(ShaderSources != nullptr);
	m_bytecodeDiskCache = CrShaderDiskCache(ShaderSources->GetUbershaderTempDirectory() / "Bytecode Cache", "Ubershader.hash", ShaderSources->GetUbershaderHash());
}

void CrMaterialCompiler::CreateMaterialShaderDefines(const CrMaterialShaderDescriptor& materialShaderDescriptor, CrShaderCompilerDefines& defines)
{
	crstl::string materialShaderVariantDefine = GetMaterialShaderEnum(materialShaderDescriptor.shaderVariant);
	defines.AddDefine(materialShaderVariantDefine);
}

CrShaderBytecodeHandle CrMaterialCompiler::GetDiskCachedOrCompileShaderBytecode
(const CrFixedPath& shaderSourcePath, const crstl::string& entryPoint, const CrHash& shaderHash, const CrMaterialShaderDescriptor& materialShaderDescriptor)
{
	// Try to load bytecode from cache
	CrShaderBytecodeHandle shaderBytecode = m_bytecodeDiskCache.LoadFromCache(shaderHash, materialShaderDescriptor.graphicsApi);
	
	// If we couldn't find the bytecode in the cache, compile and cache here
	if (!shaderBytecode)
	{
		// Create defines
		CrShaderCompilerDefines defines;
		CreateMaterialShaderDefines(materialShaderDescriptor, defines);

		CrShaderBytecodeCompilationDescriptor compilationDescriptor(shaderSourcePath, crstl::fixed_string128(entryPoint.c_str()),
		materialShaderDescriptor.shaderStage, materialShaderDescriptor.graphicsApi, materialShaderDescriptor.platform);

		// Compile bytecode
		shaderBytecode = ShaderManager->CompileShaderBytecode(compilationDescriptor, defines);

		CrAssertMsg(shaderBytecode != nullptr, "Bytecode compilation failed.");

		// Save to cache if compilation was successful
		if (shaderBytecode)
		{
			m_bytecodeDiskCache.SaveToCache(shaderHash, materialShaderDescriptor.graphicsApi, shaderBytecode);
		}
	}

	return shaderBytecode;
}

CrMaterialHandle CrMaterialCompiler::CompileMaterial(const CrMaterialDescriptor& descriptor)
{
	unused_parameter(descriptor); // TODO

	cr3d::GraphicsApi::T graphicsApi = RenderSystem->GetGraphicsApi();
	cr::Platform::T platform = cr::Platform::Windows;

	// Generate header with defines
	CrShaderHeaderGenerator shaderHeaderGenerator;
	shaderHeaderGenerator.Define("TEXTURED");

	crstl::string patchedShaderSource;
	patchedShaderSource += shaderHeaderGenerator.GetString();
	patchedShaderSource += ShaderSources->GetUbershaderSource();

	// Create directory for ubershader shaders
	CrFixedPath patchedShaderSourcePath = ShaderSources->GetUbershaderTempDirectory();

	// Make sure directory exists
	crstl::create_directories(patchedShaderSourcePath.c_str());

	patchedShaderSourcePath /= "ShaderTemp.hlsl";

	// Save patched code. We overwrite any existing file
	if (crstl::file patchedSourceFile = crstl::file(patchedShaderSourcePath.c_str(), crstl::file_flags::force_create | crstl::file_flags::write))
	{
		patchedSourceFile.write((void*)patchedShaderSource.c_str(), patchedShaderSource.length());
	}

	CrMaterialHandle material = CrMaterialHandle(new CrMaterial());

	CrMaterialShaderDescriptor baseShaderDescriptor;
	baseShaderDescriptor.blendMode = CrMaterialBlendMode::Opaque;
	baseShaderDescriptor.alphaTestMode = CrAlphaTestMode::Always;
	baseShaderDescriptor.platform = platform;
	baseShaderDescriptor.graphicsApi = graphicsApi;

	crstl::string entryPoints[cr3d::ShaderStage::GraphicsStageCount] =
	{
		"UbershaderVS",
		"UbershaderPS",
		"UbershaderHS",
		"UbershaderDS",
		"UbershaderGS",
	};

	for (CrMaterialShaderVariant::T variant = CrMaterialShaderVariant::First; variant < CrMaterialShaderVariant::Count; ++variant)
	{
		CrGraphicsShaderDescriptor shaderDescriptor;

		CrMaterialShaderDescriptor materialShaderDescriptor = baseShaderDescriptor;
		materialShaderDescriptor.shaderVariant = variant;

		for (cr3d::ShaderStage::T stage = cr3d::ShaderStage::Vertex; stage <= cr3d::ShaderStage::Pixel; ++stage)
		{
			materialShaderDescriptor.shaderStage = stage;

			CrShaderBytecodeHandle bytecode = GetDiskCachedOrCompileShaderBytecode
			(
				patchedShaderSourcePath, 
				entryPoints[stage], 
				materialShaderDescriptor.ComputeHash(), 
				materialShaderDescriptor
			);

			CrAssertMsg(bytecode != nullptr, "Bytecode is null. Compilation failed");

			shaderDescriptor.m_bytecodes.push_back(bytecode);
		}

		material->m_shaders[variant] = ShaderManager->GetRenderDevice()->CreateGraphicsShader(shaderDescriptor);
	}

	return material;
}