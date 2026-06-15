#include "Graphics/CrRendering_pch.h"
#include "SamplerD3D12.h"
#include "DeviceD3D12.h"
#include "CrD3D12.h"

#include "Core/Logging/ICrDebug.h"

namespace crgfx
{
	SamplerD3D12::SamplerD3D12(crgfx::IDevice* device, const crgfx::CrSamplerDescriptor& descriptor) : ISampler(device)
	{
		crgfx::DeviceD3D12* d3d12RenderDevice = static_cast<crgfx::DeviceD3D12*>(device);

		D3D12_SAMPLER_DESC samplerDescriptor;
		samplerDescriptor.Filter = crd3d::GetD3DFilter(descriptor.minFilter, descriptor.magFilter, descriptor.mipmapFilter, descriptor.enableAnisotropy, descriptor.enableCompare);
		samplerDescriptor.AddressU = crd3d::GetD3DAddressMode(descriptor.addressModeU);
		samplerDescriptor.AddressV = crd3d::GetD3DAddressMode(descriptor.addressModeV);
		samplerDescriptor.AddressW = crd3d::GetD3DAddressMode(descriptor.addressModeW);
		samplerDescriptor.MipLODBias = descriptor.mipLodBias;
		samplerDescriptor.MaxAnisotropy = descriptor.maxAnisotropy;
		samplerDescriptor.ComparisonFunc = crd3d::GetD3DCompareOp(descriptor.compareOp);

		switch (descriptor.borderColor)
		{
		case crgfx::BorderColor::TransparentBlack:
			samplerDescriptor.BorderColor[0] = 0.0f;
			samplerDescriptor.BorderColor[1] = 0.0f;
			samplerDescriptor.BorderColor[2] = 0.0f;
			samplerDescriptor.BorderColor[3] = 0.0f;
			break;
		case crgfx::BorderColor::OpaqueBlack:
			samplerDescriptor.BorderColor[0] = 0.0f;
			samplerDescriptor.BorderColor[1] = 0.0f;
			samplerDescriptor.BorderColor[2] = 0.0f;
			samplerDescriptor.BorderColor[3] = 1.0f;
			break;
		case crgfx::BorderColor::OpaqueWhite:
			samplerDescriptor.BorderColor[0] = 1.0f;
			samplerDescriptor.BorderColor[1] = 1.0f;
			samplerDescriptor.BorderColor[2] = 1.0f;
			samplerDescriptor.BorderColor[3] = 1.0f;
			break;
		}

		samplerDescriptor.MinLOD = descriptor.minLod;
		samplerDescriptor.MaxLOD = descriptor.maxLod;

		m_d3d12Descriptor = d3d12RenderDevice->AllocateSamplerDescriptor();
		d3d12RenderDevice->GetD3D12Device()->CreateSampler(&samplerDescriptor, m_d3d12Descriptor);
	}

	SamplerD3D12::~SamplerD3D12()
	{
		crgfx::DeviceD3D12* d3d12RenderDevice = static_cast<crgfx::DeviceD3D12*>(m_renderDevice);
		d3d12RenderDevice->FreeSamplerDescriptor(m_d3d12Descriptor);
	}
};
