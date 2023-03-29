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
#include "Core/FileSystem/ICrFile.h"

CrString CrShaderHeaderGenerator::HashDefine = "#define ";

static CrMaterialCompiler MaterialCompiler;

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

CrMaterialCompiler& CrMaterialCompiler::Get()
{
	return MaterialCompiler;
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
	m_header += HashDefine + define + " " + CrString(value) + "\n";
}

void CrMaterialCompiler::Initialize()
{
	const CrShaderSources& shaderSources = CrShaderSources::Get();
	m_bytecodeDiskCache = CrShaderDiskCache(shaderSources.GetUbershaderTempDirectory() / "Bytecode Cache", "Ubershader.hash", shaderSources.GetUbershaderHash());
}

void CrMaterialCompiler::CreateMaterialShaderDefines(const CrMaterialShaderDescriptor& materialShaderDescriptor, CrShaderCompilerDefines& defines)
{
	CrString materialShaderVariantDefine = GetMaterialShaderEnum(materialShaderDescriptor.shaderVariant);
	defines.AddDefine(materialShaderVariantDefine);
}

CrShaderBytecodeHandle CrMaterialCompiler::GetDiskCachedOrCompileShaderBytecode
(const CrPath& shaderSourcePath, const CrString& entryPoint, const CrHash& shaderHash, const CrMaterialShaderDescriptor& materialShaderDescriptor)
{
	// Try to load bytecode from cache
	CrShaderBytecodeHandle shaderBytecode = m_bytecodeDiskCache.LoadFromCache(shaderHash, materialShaderDescriptor.graphicsApi);
	
	// If we couldn't find the bytecode in the cache, compile and cache here
	if (!shaderBytecode)
	{
		// Create defines
		CrShaderCompilerDefines defines;
		CreateMaterialShaderDefines(materialShaderDescriptor, defines);

		CrShaderBytecodeCompilationDescriptor compilationDescriptor(shaderSourcePath, CrFixedString128(entryPoint.c_str()),
		materialShaderDescriptor.shaderStage, materialShaderDescriptor.graphicsApi, materialShaderDescriptor.platform);

		// Compile bytecode
		shaderBytecode = CrShaderManager::Get().CompileShaderBytecode(compilationDescriptor, defines);

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

	cr3d::GraphicsApi::T graphicsApi = ICrRenderSystem::Get()->GetGraphicsApi();
	cr::Platform::T platform = cr::Platform::Windows;

	// Generate header with defines
	CrShaderHeaderGenerator shaderHeaderGenerator;
	shaderHeaderGenerator.Define("TEXTURED");

	CrString patchedShaderSource;
	patchedShaderSource += shaderHeaderGenerator.GetString();
	patchedShaderSource += CrShaderSources::Get().GetUbershaderSource();

	// Create directory for ubershader shaders
	CrPath patchedShaderSourcePath = CrShaderSources::Get().GetUbershaderTempDirectory();

	// Make sure directory exists
	ICrFile::CreateDirectories(patchedShaderSourcePath.c_str());

	patchedShaderSourcePath /= "ShaderTemp.hlsl";

	// Save patched code. We overwrite any existing file (hence ForceCreate)
	CrFileHandle patchedSourceFile = ICrFile::OpenFile(patchedShaderSourcePath.c_str(), FileOpenFlags::ForceCreate | FileOpenFlags::Write);
	patchedSourceFile->Write((void*)patchedShaderSource.c_str(), patchedShaderSource.length());
	patchedSourceFile = nullptr;

	CrMaterialHandle material = CrMaterialHandle(new CrMaterial());

	CrMaterialShaderDescriptor baseShaderDescriptor;
	baseShaderDescriptor.blendMode = CrMaterialBlendMode::Opaque;
	baseShaderDescriptor.alphaTestMode = CrAlphaTestMode::Always;
	baseShaderDescriptor.platform = platform;
	baseShaderDescriptor.graphicsApi = graphicsApi;

	CrString entryPoints[cr3d::ShaderStage::GraphicsStageCount] = 
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

		material->m_shaders[variant] = CrShaderManager::Get().GetRenderDevice()->CreateGraphicsShader(shaderDescriptor);
	}

	return material;
}