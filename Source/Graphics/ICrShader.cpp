#include "Graphics/CrRendering_pch.h"

#include "ICrShader.h"

#include "Core/Logging/ICrDebug.h"

CrShaderCompilerDefines CrShaderCompilerDefines::Dummy;

template<typename ResourcesT>
void ICrShaderBindingLayout::ProcessResourceArray(crgfx::ShaderResourceType::T resourceType, const ResourcesT& resources)
{
	m_resourceOffsets[resourceType].offset = (uint8_t)m_bindings.size();
	m_resourceOffsets[resourceType].count = (uint8_t)resources.size();

	crgfx::ShaderStage::T currentStage = crgfx::ShaderStage::Count;
	uint32_t currentStageIndex = 0;
	uint32_t currentOffset = m_resourceOffsets[resourceType].offset;

	for (const CrShaderBinding& shaderBinding : resources)
	{
		if (currentStage != shaderBinding.stage)
		{
			currentStage = (crgfx::ShaderStage::T)shaderBinding.stage;
			currentStageIndex = GetStageIndex(currentStage);
			m_stageResourceOffsets[resourceType][currentStageIndex].offset = (uint8_t)currentOffset;
		}
			
		m_stageResourceOffsets[resourceType][currentStageIndex].count++;
		currentOffset++;
		m_bindings.push_back(shaderBinding);
	}

	m_totalResourceCount += m_resourceOffsets[resourceType].count;
}

ICrShaderBindingLayout::ICrShaderBindingLayout(const CrShaderBindingLayoutResources& resources)
{
	// We assume shader stages are sequential at this point. That is, any resources are packed
	// by shader stage (i.e. all vertex shader constant buffers together)
	ProcessResourceArray(crgfx::ShaderResourceType::ConstantBuffer, resources.constantBuffers);
	ProcessResourceArray(crgfx::ShaderResourceType::Sampler, resources.samplers);
	ProcessResourceArray(crgfx::ShaderResourceType::Texture, resources.textures);
	ProcessResourceArray(crgfx::ShaderResourceType::RWTexture, resources.rwTextures);
	ProcessResourceArray(crgfx::ShaderResourceType::StorageBuffer, resources.storageBuffers);
	ProcessResourceArray(crgfx::ShaderResourceType::RWStorageBuffer, resources.rwStorageBuffers);
	ProcessResourceArray(crgfx::ShaderResourceType::RWTypedBuffer, resources.rwTypedBuffers);
}

ICrGraphicsShader::ICrGraphicsShader(crgfx::IDevice* /*renderDevice*/, const CrGraphicsShaderDescriptor& graphicsShaderDescriptor)
{
	for (const CrShaderBytecodeHandle& bytecode : graphicsShaderDescriptor.m_bytecodes)
	{
		m_bytecodes.push_back(bytecode);
		m_hash << bytecode->GetHash();
		m_debugName = graphicsShaderDescriptor.m_debugName;
	}
}
