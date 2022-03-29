#include "CrRendering_pch.h"

#include "ICrShader.h"

#include "Core/Logging/ICrDebug.h"

CrShaderCompilerDefines CrShaderCompilerDefines::Dummy;

template<typename ResourcesT>
void ICrShaderBindingLayout::ProcessResourceArray(cr3d::ShaderResourceType::T resourceType, const ResourcesT& resources)
{
	m_resourceOffsets[resourceType].offset = (uint8_t)m_bindings.size();
	m_resourceOffsets[resourceType].count = (uint8_t)resources.size();

	cr3d::ShaderStage::T currentStage = cr3d::ShaderStage::Count;
	uint32_t currentStageIndex = 0;
	uint8_t currentOffset = 0;

	for (const CrShaderBinding& shaderBinding : resources)
	{
		if (shaderBinding.stage != currentStage)
		{
			currentStage = (cr3d::ShaderStage::T)shaderBinding.stage;
			currentStageIndex = GetStageIndex(currentStage);
			m_stageResourceOffsets[resourceType][currentStageIndex].offset = currentOffset;
		}

		m_stageResourceOffsets[resourceType][currentStageIndex].count++;
		m_bindings.push_back(shaderBinding);
	}
}

ICrShaderBindingLayout::ICrShaderBindingLayout(const CrShaderBindingLayoutResources& resources)
{
	// We assume shader stages are sequential at this point. That is, any resources are packed
	// by shader stage (i.e. all vertex shader constant buffers together)
	ProcessResourceArray(cr3d::ShaderResourceType::ConstantBuffer, resources.constantBuffers);
	ProcessResourceArray(cr3d::ShaderResourceType::Sampler, resources.samplers);
	ProcessResourceArray(cr3d::ShaderResourceType::Texture, resources.textures);
	ProcessResourceArray(cr3d::ShaderResourceType::RWTexture, resources.rwTextures);
	ProcessResourceArray(cr3d::ShaderResourceType::StorageBuffer, resources.storageBuffers);
	ProcessResourceArray(cr3d::ShaderResourceType::RWStorageBuffer, resources.rwStorageBuffers);
	ProcessResourceArray(cr3d::ShaderResourceType::RWDataBuffer, resources.rwDataBuffers);
}

ICrGraphicsShader::ICrGraphicsShader(ICrRenderDevice* /*renderDevice*/, const CrGraphicsShaderDescriptor& graphicsShaderDescriptor)
{
	for (const CrShaderBytecodeSharedHandle& bytecode : graphicsShaderDescriptor.m_bytecodes)
	{
		m_bytecodes.push_back(bytecode);
		m_hash <<= bytecode->GetHash();
		m_debugName = graphicsShaderDescriptor.m_debugName;
	}
}
