#pragma once

#include "Graphics/ISampler.h"

#include "CrD3D12.h"
#include "CrDescriptorHeapD3D12.h"

namespace crgfx
{
	class IDevice;

	class SamplerD3D12 final : public ISampler
	{
	public:

		SamplerD3D12(crgfx::IDevice* renderDevice, const CrSamplerDescriptor& descriptor);

		~SamplerD3D12();

		D3D12_CPU_DESCRIPTOR_HANDLE GetD3D12Descriptor() const { return m_d3d12Descriptor; }

	private:

		D3D12_CPU_DESCRIPTOR_HANDLE m_d3d12Descriptor;
	};
}