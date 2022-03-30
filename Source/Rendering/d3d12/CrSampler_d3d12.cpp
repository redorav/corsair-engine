#include "CrRendering_pch.h"
#include "CrSampler_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrD3D12.h"

#include "Core/Logging/ICrDebug.h"

CrSamplerD3D12::CrSamplerD3D12(ICrRenderDevice* renderDevice, const CrSamplerDescriptor& descriptor) : ICrSampler(renderDevice)
{
	m_d3d12Sampler.Filter = crd3d::GetD3DFilter(descriptor.minFilter, descriptor.magFilter, descriptor.mipmapFilter, descriptor.enableAnisotropy, descriptor.enableCompare);
	m_d3d12Sampler.AddressU = crd3d::GetD3DAddressMode(descriptor.addressModeU);
	m_d3d12Sampler.AddressV = crd3d::GetD3DAddressMode(descriptor.addressModeV);
	m_d3d12Sampler.AddressW = crd3d::GetD3DAddressMode(descriptor.addressModeW);
	m_d3d12Sampler.MipLODBias = descriptor.mipLodBias;
	m_d3d12Sampler.MaxAnisotropy = descriptor.maxAnisotropy;
	m_d3d12Sampler.ComparisonFunc = crd3d::GetD3DCompareOp(descriptor.compareOp);

	switch (descriptor.borderColor)
	{
		case cr3d::BorderColor::TransparentBlack:
			m_d3d12Sampler.BorderColor[0] = 0.0f;
			m_d3d12Sampler.BorderColor[1] = 0.0f;
			m_d3d12Sampler.BorderColor[2] = 0.0f;
			m_d3d12Sampler.BorderColor[3] = 0.0f;
			break;
		case cr3d::BorderColor::OpaqueBlack:
			m_d3d12Sampler.BorderColor[0] = 0.0f;
			m_d3d12Sampler.BorderColor[1] = 0.0f;
			m_d3d12Sampler.BorderColor[2] = 0.0f;
			m_d3d12Sampler.BorderColor[3] = 1.0f;
			break;
		case cr3d::BorderColor::OpaqueWhite:
			m_d3d12Sampler.BorderColor[0] = 1.0f;
			m_d3d12Sampler.BorderColor[1] = 1.0f;
			m_d3d12Sampler.BorderColor[2] = 1.0f;
			m_d3d12Sampler.BorderColor[3] = 1.0f;
			break;
	}

	m_d3d12Sampler.MinLOD = descriptor.minLod;
	m_d3d12Sampler.MaxLOD = descriptor.maxLod;
}

CrSamplerD3D12::~CrSamplerD3D12()
{
	
}
