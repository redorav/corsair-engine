#include "CrRendering_pch.h"

#include "ICrShader.h"
#include "ShaderResources.h"

#include "Core/Logging/ICrDebug.h"

CrShaderResourceTable::CrShaderResourceTable()
{
	for (uint32_t i = 0; i < cr3d::ShaderStage::GraphicsStageCount; ++i)
	{
		for (uint32_t j = 0; j < MaxStageConstantBuffers; ++j)
		{
			m_usedConstantBuffers[i][j] = ConstantBuffers::Count;
		}

		for (uint32_t j = 0; j < MaxStageTextures; ++j)
		{
			m_usedTextures[i][j] = Textures::Count;
		}

		for (uint32_t j = 0; j < MaxStageSamplers; ++j)
		{
			m_usedSamplers[i][j] = Samplers::Count;
		}
	}
}

uint32_t CrShaderResourceTable::GetConstantBufferTotalCount() const
{
	return m_usedConstantBufferTotalCount;
}

uint32_t CrShaderResourceTable::GetConstantBufferCount(cr3d::ShaderStage::T stage) const
{
	return m_usedConstantBufferCount[stage];
}

ConstantBuffers::T CrShaderResourceTable::GetConstantBufferID(cr3d::ShaderStage::T stage, uint32_t index) const
{
	CrAssert(index < CrShaderResourceTable::MaxStageConstantBuffers);
	return m_usedConstantBuffers[stage][index];
}

bindpoint_t CrShaderResourceTable::GetConstantBufferBindPoint(cr3d::ShaderStage::T stage, uint32_t index) const
{
	CrAssert(index < CrShaderResourceTable::MaxStageConstantBuffers);
	return m_usedConstantBufferBindPoints[stage][index];
}

uint32_t CrShaderResourceTable::GetTextureTotalCount() const
{
	return m_usedTextureTotalCount;
}

uint32_t CrShaderResourceTable::GetTextureCount(cr3d::ShaderStage::T stage) const
{
	return m_usedTextureCount[stage];
}

Textures::T CrShaderResourceTable::GetTextureID(cr3d::ShaderStage::T stage, uint32_t index) const
{
	CrAssert(index < CrShaderResourceTable::MaxStageConstantBuffers);
	return m_usedTextures[stage][index];
}

bindpoint_t CrShaderResourceTable::GetTextureBindPoint(cr3d::ShaderStage::T stage, uint32_t index) const
{
	CrAssert(index < CrShaderResourceTable::MaxStageTextures);
	return m_usedTextureBindPoints[stage][index];
}

uint32_t CrShaderResourceTable::GetSamplerTotalCount() const
{
	return m_usedSamplerTotalCount;
}

uint32_t CrShaderResourceTable::GetSamplerCount(cr3d::ShaderStage::T stage) const
{
	return m_usedSamplerCount[stage];
}

Samplers::T CrShaderResourceTable::GetSamplerID(cr3d::ShaderStage::T stage, uint32_t index) const
{
	CrAssert(index < CrShaderResourceTable::MaxStageConstantBuffers);
	return m_usedSamplers[stage][index];
}

bindpoint_t CrShaderResourceTable::GetSamplerBindPoint(cr3d::ShaderStage::T stage, uint32_t index) const
{
	CrAssert(index < CrShaderResourceTable::MaxStageSamplers);
	return m_usedSamplerBindPoints[stage][index];
}

void CrShaderResourceTable::AddConstantBuffer(cr3d::ShaderStage::T stage, ConstantBuffers::T id, bindpoint_t bindPoint)
{
	CrAssert(m_usedConstantBufferCount[stage] < MaxStageConstantBuffers);

	m_usedConstantBuffers[stage][m_usedConstantBufferCount[stage]] = id;
	m_usedConstantBufferBindPoints[stage][m_usedConstantBufferCount[stage]] = bindPoint;
	m_usedConstantBufferCount[stage]++;
	m_usedConstantBufferTotalCount++;
}

void CrShaderResourceTable::AddTexture(cr3d::ShaderStage::T stage, Textures::T id, bindpoint_t bindPoint)
{
	CrAssert(m_usedTextureCount[stage] < MaxStageTextures);

	m_usedTextures[stage][m_usedTextureCount[stage]] = id;
	m_usedTextureBindPoints[stage][m_usedTextureCount[stage]] = bindPoint;
	m_usedTextureCount[stage]++;
	m_usedTextureTotalCount++;
}

void CrShaderResourceTable::AddSampler(cr3d::ShaderStage::T stage, Samplers::T index, bindpoint_t bindPoint)
{
	CrAssert(m_usedSamplerCount[stage] < MaxStageSamplers);

	m_usedSamplers[stage][m_usedSamplerCount[stage]] = index;
	m_usedSamplerBindPoints[stage][m_usedSamplerCount[stage]] = bindPoint;
	m_usedSamplerCount[stage]++;
	m_usedSamplerTotalCount++;
}

const CrHash& ICrShader::GetHash() const
{
	return m_hash;
}

const CrShaderResourceTable& ICrShader::GetResourceTable() const
{
	return m_resourceTable;
}