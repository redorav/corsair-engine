#include "CrRendering_pch.h"
#include "CrSampler_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrD3D12.h"

#include "Core/Logging/ICrDebug.h"

CrSamplerD3D12::CrSamplerD3D12(ICrRenderDevice* renderDevice, const CrSamplerDescriptor& descriptor) : ICrSampler(renderDevice)
{
	CrRenderDeviceD3D12* renderDeviceD3D12 = static_cast<CrRenderDeviceD3D12*>(renderDevice);
	ID3D12Device* d3d12RenderDevice = renderDeviceD3D12->GetD3D12Device();

	D3D12_SAMPLER_DESC d3d12SamplerDescriptor = {};
	
	d3d12SamplerDescriptor.Filter = crd3d::GetD3DFilter(descriptor.minFilter, descriptor.magFilter, descriptor.mipmapFilter, descriptor.enableAnisotropy, descriptor.enableCompare);
	d3d12SamplerDescriptor.AddressU = crd3d::GetD3DAddressMode(descriptor.addressModeU);
	d3d12SamplerDescriptor.AddressV = crd3d::GetD3DAddressMode(descriptor.addressModeV);
	d3d12SamplerDescriptor.AddressW = crd3d::GetD3DAddressMode(descriptor.addressModeW);
	d3d12SamplerDescriptor.MipLODBias = descriptor.mipLodBias;
	d3d12SamplerDescriptor.MaxAnisotropy = descriptor.maxAnisotropy;
	d3d12SamplerDescriptor.ComparisonFunc = crd3d::GetD3DCompareOp(descriptor.compareOp);

	switch (descriptor.borderColor)
	{
		case cr3d::BorderColor::TransparentBlack:
			d3d12SamplerDescriptor.BorderColor[0] = 0.0f;
			d3d12SamplerDescriptor.BorderColor[1] = 0.0f;
			d3d12SamplerDescriptor.BorderColor[2] = 0.0f;
			d3d12SamplerDescriptor.BorderColor[3] = 0.0f;
			break;
		case cr3d::BorderColor::OpaqueBlack:
			d3d12SamplerDescriptor.BorderColor[0] = 0.0f;
			d3d12SamplerDescriptor.BorderColor[1] = 0.0f;
			d3d12SamplerDescriptor.BorderColor[2] = 0.0f;
			d3d12SamplerDescriptor.BorderColor[3] = 1.0f;
			break;
		case cr3d::BorderColor::OpaqueWhite:
			d3d12SamplerDescriptor.BorderColor[0] = 1.0f;
			d3d12SamplerDescriptor.BorderColor[1] = 1.0f;
			d3d12SamplerDescriptor.BorderColor[2] = 1.0f;
			d3d12SamplerDescriptor.BorderColor[3] = 1.0f;
			break;
	}

	d3d12SamplerDescriptor.MinLOD = descriptor.minLod;
	d3d12SamplerDescriptor.MaxLOD = descriptor.maxLod;

	m_d3d12Sampler = renderDeviceD3D12->AllocateSamplerDescriptor();
	d3d12RenderDevice->CreateSampler(&d3d12SamplerDescriptor, m_d3d12Sampler.cpuHandle);
}

CrSamplerD3D12::~CrSamplerD3D12()
{
	CrRenderDeviceD3D12* renderDeviceD3D12 = static_cast<CrRenderDeviceD3D12*>(m_renderDevice);
	renderDeviceD3D12->FreeSamplerDescriptor(m_d3d12Sampler);
}
