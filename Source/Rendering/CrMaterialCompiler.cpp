#include "CrRendering_pch.h"

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
	m_header += HashDefine + define + eastl::to_string(value) + "\n";
}

void CrMaterialCompiler::Initialize()
{
	m_bytecodeDiskCache = CrShaderDiskCache(CrShaderSources::Get().GetUbershaderTempDirectory() / "BytecodeCache");
}

CrShaderBytecodeSharedHandle CrMaterialCompiler::GetDiskCachedOrCompileShaderBytecode
(const CrPath& shaderSourcePath, const CrString& entryPoint, const CrHash& shaderHash, const CrMaterialShaderDescriptor& materialShaderDescriptor)
{
	// Try to load bytecode from cache
	CrShaderBytecodeSharedHandle shaderBytecode = m_bytecodeDiskCache.LoadFromCache
	(shaderHash, entryPoint.c_str(), materialShaderDescriptor.graphicsApi, materialShaderDescriptor.shaderStage);
	
	// If we couldn't find the bytecode in the cache, compile and cache here
	if (!shaderBytecode)
	{
		CrShaderBytecodeCompilationDescriptor compilationDescriptor(shaderSourcePath, entryPoint.c_str(),
		materialShaderDescriptor.shaderStage, materialShaderDescriptor.graphicsApi, materialShaderDescriptor.platform);

		// Compile bytecode
		shaderBytecode = CrShaderManager::Get().CompileShaderBytecode(compilationDescriptor);
	
		// Save to cache if compilation was successful
		if (shaderBytecode)
		{
			m_bytecodeDiskCache.SaveToCache(shaderHash, materialShaderDescriptor.graphicsApi, shaderBytecode);
		}
	}

	return shaderBytecode;
}

CrMaterialSharedHandle CrMaterialCompiler::CompileMaterial(const CrMaterialDescriptor& descriptor)
{
	descriptor; // TODO

	cr3d::GraphicsApi::T graphicsApi = ICrRenderSystem::Get()->GetGraphicsApi();
	cr::Platform::T platform = cr::Platform::Windows;

	CrPath shaderEntryPointPath = CrPath(CrGlobalPaths::GetShaderSourceDirectory()) / "Ubershader.hlsl";

	// Generate header with defines
	CrShaderHeaderGenerator shaderHeaderGenerator;
	shaderHeaderGenerator.Define("PERRY");

	CrString patchedShaderSource;
	patchedShaderSource += shaderHeaderGenerator.GetString();
	patchedShaderSource += CrShaderSources::Get().GetUbershaderSource();

	// Create directory for ubershader shaders
	CrPath patchedShaderSourcePath = CrShaderSources::Get().GetUbershaderTempDirectory();

	// Make sure directory exists
	ICrFile::CreateDirectories(patchedShaderSourcePath.c_str());

	patchedShaderSourcePath /= "ShaderTemp.hlsl";

	// Save patched code. We overwrite any existing file (hence ForceCreate)
	{
		CrFileSharedHandle patchedSourceFile = ICrFile::OpenFile(patchedShaderSourcePath.c_str(), FileOpenFlags::ForceCreate | FileOpenFlags::Write);
		patchedSourceFile->Write((void*)patchedShaderSource.c_str(), patchedShaderSource.length());
	}

	CrMaterialSharedHandle material = CrMakeShared<CrMaterial>();

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

		for (cr3d::ShaderStage::T stage = cr3d::ShaderStage::Vertex; stage <= cr3d::ShaderStage::Pixel; ++stage)
		{
			// TODO Store per-variant patched shader on disk

			CrMaterialShaderDescriptor materialShaderDescriptor = baseShaderDescriptor;
			materialShaderDescriptor.shaderVariant = variant;
			materialShaderDescriptor.shaderStage = stage;
			CrShaderBytecodeSharedHandle bytecode = GetDiskCachedOrCompileShaderBytecode(patchedShaderSourcePath, entryPoints[stage], materialShaderDescriptor.ComputeHash(), materialShaderDescriptor);
			shaderDescriptor.m_bytecodes.push_back(bytecode);
		}

		material->m_shaders[variant] = CrShaderManager::Get().GetRenderDevice()->CreateGraphicsShader(shaderDescriptor);
	}

	return material;
}