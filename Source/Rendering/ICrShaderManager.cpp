#include "CrRendering_pch.h"

#include "ICrShaderManager.h"
#include "ICrShaderReflection.h"
#include "ICrShader.h"
#include "CrResourceManager.h"
#include "ShaderResources.h"

#include "Core/CrMacros.h"
#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/FileSystem/ICrFile.h"

// TODO Delete
#include "vulkan/CrShaderReflection_vk.h"
#include "vulkan/CrShaderManager_vk.h"

static CrShaderManagerVulkan g_shaderManager;

CrShaderResource CrShaderResource::Invalid = {};

ICrShaderManager* ICrShaderManager::Get()
{
	return &g_shaderManager;
}

CrGraphicsShaderHandle ICrShaderManager::LoadGraphicsShader(CrGraphicsShaderCreate& shaderCreateInfo)
{
	CrGraphicsShaderHandle graphicsShader(new CrGraphicsShader());
	CrShaderReflectionVulkan reflection;

	for (CrGraphicsShaderStageCreate& stageCreate : shaderCreateInfo.m_stages)
	{
		// 1. Load the bytecode from disk
		LoadShaderBytecode(stageCreate);

		// 2. Create the shader stage (vertex, pixel, etc.)
		CrShaderStageInfo shaderStage;
		shaderStage.m_shader = CreateGraphicsShaderStage(stageCreate.bytecode.data(), stageCreate.bytecode.size(), stageCreate.stage);
		shaderStage.m_entryPointName = stageCreate.entryPoint;
		shaderStage.m_stage = stageCreate.stage;

		// 3. Compute hashes based on bytecode
		CrHash bytecodeHash = CrHash(stageCreate.bytecode.data(), stageCreate.bytecode.size());
		graphicsShader->m_hash <<= bytecodeHash;

		graphicsShader->m_shaderStages.push_back(shaderStage);

		// 4. Add to the reflection structure (we'll build the necessary resource tables using this later)
		reflection.AddShaderStage(stageCreate.stage, stageCreate.bytecode);
	}

	CreateShaderResourceSet(shaderCreateInfo, reflection, graphicsShader->m_resourceSet);

	return graphicsShader;
}

void ICrShaderManager::LoadShaderBytecode(CrGraphicsShaderStageCreate& shaderStage)
{
	CrFileSharedHandle file = ICrFile::Create(shaderStage.path, FileOpenFlags::Read);

	switch (shaderStage.format)
	{
		case cr3d::ShaderCodeFormat::Binary:
		{
			shaderStage.bytecode.resize(file->GetSize());
			file->Read(shaderStage.bytecode.data(), shaderStage.bytecode.size());
			break;
		}
		case cr3d::ShaderCodeFormat::Source:
		{
			shaderStage.source.resize(file->GetSize());
			file->Read(shaderStage.source.data(), shaderStage.source.size());
			CompileStage(shaderStage);
			break;
		}
	}
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

void ICrShaderManager::CreateShaderResourceSet(const CrGraphicsShaderCreate& shaderCreateInfo, const CrShaderReflectionVulkan& reflection, CrShaderResourceSet& resourceSet)
{
	for (const CrGraphicsShaderStageCreate& shaderStageCreate : shaderCreateInfo.m_stages)
	{
		cr3d::ShaderStage::T stage = shaderStageCreate.stage;

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

	CreateShaderResourceSetPS(shaderCreateInfo, reflection, resourceSet);
}

void ICrShaderManager::Init(const ICrRenderDevice* renderDevice)
{
	m_renderDevice = renderDevice;
	InitPS();
}

void ICrShaderManager::CompileStage(CrGraphicsShaderStageCreate& shaderStageInfo)
{
	CompileStagePS(shaderStageInfo);
}

VkShaderModule ICrShaderManager::CreateGraphicsShaderStage(const unsigned char* byteCode, size_t codeSize, cr3d::ShaderStage::T stage)
{
	return CreateGraphicsShaderStagePS(byteCode, codeSize, stage);
}
