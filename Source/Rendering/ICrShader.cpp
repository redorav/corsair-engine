#include "CrRendering_pch.h"

#include "ICrShader.h"
#include "ShaderResources.h"

#include "Core/Logging/ICrDebug.h"

ICrShaderResourceTable::ICrShaderResourceTable(const CrShaderResourceCount& resourceCount)
{
	// Resize bindings vector and compute relevant offsets
	uint8_t totalResourceCount = 0;
	{
		CrAssert(resourceCount.constantBuffers < MaxStageConstantBuffers);
		CrAssert(resourceCount.samplers < MaxStageSamplers);

		m_constantBufferOffset = totalResourceCount;
		totalResourceCount += resourceCount.constantBuffers;

		m_samplerOffset = totalResourceCount;
		totalResourceCount += resourceCount.samplers;

		m_textureOffset = totalResourceCount;
		totalResourceCount += resourceCount.textures;

		m_bindings.resize(totalResourceCount);
	}
}

void ICrShaderResourceTable::AddConstantBuffer(cr3d::ShaderStage::T stage, ConstantBuffers::T id, bindpoint_t bindPoint)
{
	m_bindings[m_constantBufferOffset + m_constantBufferCount] = CrShaderBinding(bindPoint, stage, id);
	m_constantBufferCount++;
}

void ICrShaderResourceTable::AddTexture(cr3d::ShaderStage::T stage, Textures::T id, bindpoint_t bindPoint)
{
	m_bindings[m_textureOffset + m_textureCount] = CrShaderBinding(bindPoint, stage, id);
	m_textureCount++;
}

void ICrShaderResourceTable::AddSampler(cr3d::ShaderStage::T stage, Samplers::T id, bindpoint_t bindPoint)
{
	m_bindings[m_samplerOffset + m_samplerCount] = CrShaderBinding(bindPoint, stage, id);
	m_samplerCount++;
}
