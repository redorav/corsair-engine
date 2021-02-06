#pragma once

#include "Rendering/ICrSampler.h"
#include <d3d12.h>

class ICrRenderDevice;

class CrSamplerD3D12 final : public ICrSampler
{
public:

	~CrSamplerD3D12();

	D3D12_CPU_DESCRIPTOR_HANDLE GetD3D12Sampler() const { return m_d3d12Sampler; }

	CrSamplerD3D12(ICrRenderDevice* renderDevice, const CrSamplerDescriptor& descriptor);

private:

	ID3D12Device* m_d3dDevice;

	D3D12_CPU_DESCRIPTOR_HANDLE m_d3d12Sampler;
};