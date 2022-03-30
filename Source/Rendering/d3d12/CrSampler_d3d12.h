#pragma once

#include "Rendering/ICrSampler.h"

#include "CrD3D12.h"

#include <d3d12.h>

class ICrRenderDevice;

class CrSamplerD3D12 final : public ICrSampler
{
public:

	CrSamplerD3D12(ICrRenderDevice* renderDevice, const CrSamplerDescriptor& descriptor);

	~CrSamplerD3D12();

	const D3D12_SAMPLER_DESC& GetD3D12Sampler() const { return m_d3d12Sampler; }

private:

	D3D12_SAMPLER_DESC m_d3d12Sampler;
};