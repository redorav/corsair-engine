#pragma once

#include "Rendering/ICrSampler.h"

#include "CrD3D12.h"
#include "CrDescriptorHeapD3D12.h"

class ICrRenderDevice;

class CrSamplerD3D12 final : public ICrSampler
{
public:

	CrSamplerD3D12(ICrRenderDevice* renderDevice, const CrSamplerDescriptor& descriptor);

	~CrSamplerD3D12();

	D3D12_CPU_DESCRIPTOR_HANDLE GetD3D12Descriptor() const { return m_d3d12Descriptor; }

private:

	D3D12_CPU_DESCRIPTOR_HANDLE m_d3d12Descriptor;
};