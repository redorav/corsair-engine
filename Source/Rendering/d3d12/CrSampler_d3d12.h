#pragma once

#include "Rendering/ICrSampler.h"

#include "CrD3D12.h"

#include <d3d12.h>

class ICrRenderDevice;

class CrSamplerD3D12 final : public ICrSampler
{
public:

	~CrSamplerD3D12();

	crd3d::DescriptorD3D12 GetD3D12Sampler() const { return m_d3d12Sampler; }

	CrSamplerD3D12(ICrRenderDevice* renderDevice, const CrSamplerDescriptor& descriptor);

private:

	crd3d::DescriptorD3D12 m_d3d12Sampler;
};