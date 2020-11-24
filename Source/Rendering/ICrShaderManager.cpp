﻿#include "CrRendering_pch.h"

#include "Rendering/ShaderCompiler/CrShaderCompilerCommandLine.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/ICrShaderManager.h"
#include "Rendering/ICrShaderReflection.h"
#include "Rendering/ICrShader.h"
#include "CrResourceManager.h"
#include "ShaderResources.h"

#include "Core/CrMacros.h"
#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/FileSystem/ICrFile.h"
#include "Core/Logging/ICrDebug.h"
#include "Core/Process/CrProcess.h"
#include "Core/CrPlatform.h"

#include "GlobalVariables.h"

// TODO Delete
#include "vulkan/CrShaderReflection_vk.h"
#include "vulkan/CrShaderManager_vk.h"

// TODO Delete
static CrShaderManagerVulkan g_shaderManager;

CrShaderResource CrShaderResource::Invalid = {};

ICrShaderManager* ICrShaderManager::Get()
{
	return &g_shaderManager;
}

CrShaderBytecodeSharedHandle ICrShaderManager::LoadShaderBytecode(const CrPath& path, const CrShaderBytecodeDescriptor& bytecodeDescriptor) const
{
	CrFileSharedHandle file = ICrFile::Create(path, FileOpenFlags::Read);
	return LoadShaderBytecode(file, bytecodeDescriptor);
}

CrShaderBytecodeSharedHandle ICrShaderManager::LoadShaderBytecode(const CrFileSharedHandle& file, const CrShaderBytecodeDescriptor& bytecodeDescriptor) const
{
	switch (bytecodeDescriptor.format)
	{
		case cr3d::ShaderCodeFormat::Binary:
		{
			CrShaderBytecodeSharedHandle bytecode;
			{
				CrVector<unsigned char> bytecodeData;
				bytecodeData.resize(file->GetSize());
				file->Read(bytecodeData.data(), bytecodeData.size());
				bytecode = CrShaderBytecodeSharedHandle(new CrShaderBytecode(std::move(bytecodeData), bytecodeDescriptor.entryPoint, bytecodeDescriptor.stage));
			}
			return bytecode;
		}
		case cr3d::ShaderCodeFormat::SourceHLSL:
		{
			return CompileShaderBytecode(bytecodeDescriptor);
		}
	}

	return nullptr;
}

CrGraphicsShaderHandle ICrShaderManager::LoadGraphicsShader(const CrBytecodeLoadDescriptor& bytecodeLoadDescriptor) const
{
	CrShaderReflectionVulkan reflection; // TODO Remove this, make platform-independent

	// Create the graphics shader descriptor
	CrGraphicsShaderDescriptor graphicsShaderDescriptor;
	graphicsShaderDescriptor.m_bytecodes.reserve(bytecodeLoadDescriptor.GetBytecodeDescriptors().size());

	// Load all the relevant shader bytecodes
	for (const CrShaderBytecodeDescriptor& bytecodeDescriptor : bytecodeLoadDescriptor.GetBytecodeDescriptors())
	{
		CrShaderBytecodeSharedHandle bytecode = LoadShaderBytecode(bytecodeDescriptor.path, bytecodeDescriptor);

		graphicsShaderDescriptor.m_bytecodes.push_back(bytecode);

		// 4. Add to the reflection structure (we'll build the necessary resource tables using this later)
		reflection.AddBytecode(bytecode);
	}

	CrGraphicsShaderHandle graphicsShader = m_renderDevice->CreateGraphicsShader(graphicsShaderDescriptor);

	// TODO the shader itself can create the shader resource set after we've mangled the SPIR-V bytecode
	CreateShaderResourceSet(graphicsShader->GetStages(), reflection, graphicsShader->m_resourceSet);

	return graphicsShader;
}

ConstantBufferMetadata& ICrShaderManager::GetConstantBufferMetadata(const CrString& name)
{
	auto cBuffer = ConstantBufferTable.find(name);

	if (cBuffer != ConstantBufferTable.end())
	{
		return (*cBuffer).second;
	}

	return InvalidConstantBufferMetaInstance;
}

ConstantBufferMetadata& ICrShaderManager::GetConstantBufferMetadata(ConstantBuffers::T id)
{
	return ConstantBufferMetaTable[id];
}

TextureMetadata& ICrShaderManager::GetTextureMetadata(const CrString& name)
{
	auto textureMetadata = TextureTable.find(name);

	if (textureMetadata != TextureTable.end())
	{
		return (*textureMetadata).second;
	}

	return InvalidTextureMetaInstance;
}

TextureMetadata& ICrShaderManager::GetTextureMetadata(Textures::T id)
{
	return TextureMetaTable[id];
}

SamplerMetadata& ICrShaderManager::GetSamplerMetadata(const CrString& name)
{
	auto samplerMetadata = SamplerTable.find(name);

	if (samplerMetadata != SamplerTable.end())
	{
		return (*samplerMetadata).second;
	}

	return InvalidSamplerMetaInstance;
}

SamplerMetadata& ICrShaderManager::GetSamplerMetadata(Samplers::T id)
{
	return SamplerMetaTable[id];
}

void ICrShaderManager::CreateShaderResourceSet(const CrVector<CrShaderStageInfo>& shaderStageInfo, const CrShaderReflectionVulkan& reflection, CrShaderResourceSet& resourceSet) const
{
	for (const CrShaderStageInfo& stageInfo : shaderStageInfo)
	{
		cr3d::ShaderStage::T stage = stageInfo.stage;

		uint32_t numConstantBuffers = reflection.GetResourceCount(stage, cr3d::ShaderResourceType::ConstantBuffer);

		for (uint32_t i = 0; i < numConstantBuffers; ++i)
		{
			CrShaderResource res = reflection.GetResource(stage, cr3d::ShaderResourceType::ConstantBuffer, i);
			ConstantBufferMetadata& metadata = GetConstantBufferMetadata(res.name);
			resourceSet.AddConstantBuffer(stage, metadata.id, res.bindPoint);
		}

		uint32_t numTextures = reflection.GetResourceCount(stage, cr3d::ShaderResourceType::Texture);

		for (uint32_t i = 0; i < numTextures; ++i)
		{
			CrShaderResource res = reflection.GetResource(stage, cr3d::ShaderResourceType::Texture, i);
			TextureMetadata& metadata = GetTextureMetadata(res.name);
			resourceSet.AddTexture(stage, metadata.id, res.bindPoint);
		}

		uint32_t numSamplers = reflection.GetResourceCount(stage, cr3d::ShaderResourceType::Sampler);

		for (uint32_t i = 0; i < numSamplers; ++i)
		{
			CrShaderResource res = reflection.GetResource(stage, cr3d::ShaderResourceType::Sampler, i);
			SamplerMetadata& metadata = GetSamplerMetadata(res.name);
			resourceSet.AddSampler(stage, metadata.id, res.bindPoint);
		}
	}

	CreateShaderResourceSetPS(shaderStageInfo, reflection, resourceSet);
}

void ICrShaderManager::Init(const ICrRenderDevice* renderDevice)
{
	m_renderDevice = renderDevice;
	InitPS();
}

CrShaderBytecodeSharedHandle ICrShaderManager::CompileShaderBytecode(const CrShaderBytecodeDescriptor& bytecodeDescriptor) const
{
	CrProcessDescriptor processDescriptor;

	// TODO We need a searching policy here. If we were to distribute this as a build we'd
	// want the shader compiler in a known directory, or several directories that we search
	// The platform-specific compilers also need to be in directories relative to the main one
	const char* shaderCompilerPath = ShaderCompilerPath;
	processDescriptor.executablePath = shaderCompilerPath;
	processDescriptor.waitForCompletion = true;

	// Build command line for shader compiler
	processDescriptor.commandLine += "-input ";
	processDescriptor.commandLine += bytecodeDescriptor.path.string().c_str();
	processDescriptor.commandLine += " ";
		
	processDescriptor.commandLine += "-entrypoint ";
	processDescriptor.commandLine += bytecodeDescriptor.entryPoint.c_str();
	processDescriptor.commandLine += " ";

	processDescriptor.commandLine += "-stage ";
	processDescriptor.commandLine += CrShaderCompilerCommandLine::GetShaderStage(bytecodeDescriptor.stage);
	processDescriptor.commandLine += " ";

	CrPath outputPath = bytecodeDescriptor.path;
	outputPath.replace_extension("");
	outputPath += "_";
	outputPath += bytecodeDescriptor.entryPoint.c_str();
	outputPath.replace_extension(".spv");
	processDescriptor.commandLine += "-output ";
	processDescriptor.commandLine += outputPath.string().c_str();
	processDescriptor.commandLine += " ";

	processDescriptor.commandLine += "-platform ";
	processDescriptor.commandLine += CrShaderCompilerCommandLine::GetPlatform(bytecodeDescriptor.platform);
	processDescriptor.commandLine += " ";

	processDescriptor.commandLine += "-graphicsapi ";
	processDescriptor.commandLine += CrShaderCompilerCommandLine::GetGraphicsApi(bytecodeDescriptor.graphicsApi);
	processDescriptor.commandLine += " ";

	// Launch compilation process and wait
	CrProcess::RunExecutable(processDescriptor);

	CrFileSharedHandle compilationOutput = ICrFile::Create(outputPath, FileOpenFlags::Read);

	// Generate the SPIR-V bytecode
	CrVector<unsigned char> spirvBytecodeBytes;
	spirvBytecodeBytes.resize(compilationOutput->GetSize());

	compilationOutput->Read(spirvBytecodeBytes.data(), spirvBytecodeBytes.size());

	CrShaderBytecodeSharedHandle bytecode = CrShaderBytecodeSharedHandle(new CrShaderBytecode
	(
		std::move(spirvBytecodeBytes),
		bytecodeDescriptor.entryPoint,
		bytecodeDescriptor.stage
	));

	return bytecode;
}