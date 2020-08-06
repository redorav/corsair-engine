#include "CrRendering_pch.h"

#include "ICrShader.h"
#include "ShaderResources.h"

#include "Core/Logging/ICrDebug.h"

CrShaderResourceSet::CrShaderResourceSet()
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

uint32_t CrShaderResourceSet::GetConstantBufferTotalCount() const
{
	return m_usedConstantBufferTotalCount;
}

uint32_t CrShaderResourceSet::GetConstantBufferCount(cr3d::ShaderStage::T stage) const
{
	return m_usedConstantBufferCount[stage];
}

ConstantBuffers::T CrShaderResourceSet::GetConstantBufferID(cr3d::ShaderStage::T stage, uint32_t index) const
{
	CrAssert(index < CrShaderResourceSet::MaxStageConstantBuffers);
	return m_usedConstantBuffers[stage][index];
}

bindpoint_t CrShaderResourceSet::GetConstantBufferBindPoint(cr3d::ShaderStage::T stage, uint32_t index) const
{
	CrAssert(index < CrShaderResourceSet::MaxStageConstantBuffers);
	return m_usedConstantBufferBindPoints[stage][index];
}

uint32_t CrShaderResourceSet::GetTextureTotalCount() const
{
	return m_usedTextureTotalCount;
}

uint32_t CrShaderResourceSet::GetTextureCount(cr3d::ShaderStage::T stage) const
{
	return m_usedTextureCount[stage];
}

Textures::T CrShaderResourceSet::GetTextureID(cr3d::ShaderStage::T stage, uint32_t index) const
{
	CrAssert(index < CrShaderResourceSet::MaxStageConstantBuffers);
	return m_usedTextures[stage][index];
}

bindpoint_t CrShaderResourceSet::GetTextureBindPoint(cr3d::ShaderStage::T stage, uint32_t index) const
{
	CrAssert(index < CrShaderResourceSet::MaxStageTextures);
	return m_usedTextureBindPoints[stage][index];
}

uint32_t CrShaderResourceSet::GetSamplerTotalCount() const
{
	return m_usedSamplerTotalCount;
}

uint32_t CrShaderResourceSet::GetSamplerCount(cr3d::ShaderStage::T stage) const
{
	return m_usedSamplerCount[stage];
}

Samplers::T CrShaderResourceSet::GetSamplerID(cr3d::ShaderStage::T stage, uint32_t index) const
{
	CrAssert(index < CrShaderResourceSet::MaxStageConstantBuffers);
	return m_usedSamplers[stage][index];
}

bindpoint_t CrShaderResourceSet::GetSamplerBindPoint(cr3d::ShaderStage::T stage, uint32_t index) const
{
	CrAssert(index < CrShaderResourceSet::MaxStageSamplers);
	return m_usedSamplerBindPoints[stage][index];
}

uint32_t CrShaderResourceSet::GetBufferCount() const
{
	return m_usedBufferTotalCount;
}

uint32_t CrShaderResourceSet::GetImageCount() const
{
	return m_usedImageTotalCount;
}

void CrShaderResourceSet::AddConstantBuffer(cr3d::ShaderStage::T stage, ConstantBuffers::T id, bindpoint_t bindPoint)
{
	CrAssert(m_usedConstantBufferCount[stage] < MaxStageConstantBuffers);

	m_usedConstantBuffers[stage][m_usedConstantBufferCount[stage]] = id;
	m_usedConstantBufferBindPoints[stage][m_usedConstantBufferCount[stage]] = bindPoint;
	m_usedConstantBufferCount[stage]++;
	m_usedConstantBufferTotalCount++;
	m_usedBufferTotalCount++;
}

void CrShaderResourceSet::AddTexture(cr3d::ShaderStage::T stage, Textures::T id, bindpoint_t bindPoint)
{
	CrAssert(m_usedTextureCount[stage] < MaxStageTextures);

	m_usedTextures[stage][m_usedTextureCount[stage]] = id;
	m_usedTextureBindPoints[stage][m_usedTextureCount[stage]] = bindPoint;
	m_usedTextureCount[stage]++;
	m_usedTextureTotalCount++;
	m_usedImageTotalCount++;
}

void CrShaderResourceSet::AddSampler(cr3d::ShaderStage::T stage, Samplers::T index, bindpoint_t bindPoint)
{
	CrAssert(m_usedSamplerCount[stage] < MaxStageSamplers);

	m_usedSamplers[stage][m_usedSamplerCount[stage]] = index;
	m_usedSamplerBindPoints[stage][m_usedSamplerCount[stage]] = bindPoint;
	m_usedSamplerCount[stage]++;
	m_usedSamplerTotalCount++;
	m_usedImageTotalCount++;
}

const CrHash& ICrShader::GetHash() const
{
	return m_hash;
}

const CrShaderResourceSet& ICrShader::GetResourceSet() const
{
	return m_resourceSet;
}