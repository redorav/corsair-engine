#pragma once

#include "Rendering/ICrTexture.h"

#include "CrD3D12.h"

#include "Core/Containers/CrArray.h"
#include "Core/Containers/CrVector.h"

#include <d3d12.h>

class ICrRenderDevice;

struct CrD3D12AdditionalTextureViews
{
	// SRVs
	CrArray<CrVector<D3D12_SHADER_RESOURCE_VIEW_DESC>, cr3d::MaxMipmaps> m_d3d12SRVSingleMipSlice;
	CrArray<D3D12_SHADER_RESOURCE_VIEW_DESC, cr3d::MaxMipmaps> m_d3d12SRVSingleMipAllSlices;

	// RTVs
	CrArray<CrVector<crd3d::DescriptorD3D12>, cr3d::MaxMipmaps> m_d3d12RTVSingleMipSlice; // Each mipmap can have a variable amount of slices
	CrArray<crd3d::DescriptorD3D12, cr3d::MaxMipmaps> m_d3d12RTVSingleMipAllSlices; // Each mipmap can see all slices (via SV_RenderTargetArrayIndex)

	// DSVs
	crd3d::DescriptorD3D12 m_d3d12DSVSingleMipSlice;

	// UAVs
	CrArray<D3D12_UNORDERED_ACCESS_VIEW_DESC, cr3d::MaxMipmaps>	m_d3d12UAVSingleMipAllSlices; // Each mipmap can see all slices
};

class CrTextureD3D12 final : public ICrTexture
{
public:

	CrTextureD3D12(ICrRenderDevice* renderDevice, const CrTextureDescriptor& descriptor);

	~CrTextureD3D12();

	crd3d::DescriptorD3D12 GetD3D12RenderTargetView(uint32_t mip, uint32_t slice) const
	{
		return m_additionalViews->m_d3d12RTVSingleMipSlice[mip][slice];
	}

	crd3d::DescriptorD3D12 GetD3D12DepthStencilView() const
	{
		return m_additionalViews->m_d3d12DSVSingleMipSlice;
	}

	ID3D12Resource* GetD3D12Resource() const { return m_d3d12Resource; }

	D3D12_RESOURCE_STATES GetDefaultResourceState() const { return m_d3d12InitialState; }

	const D3D12_SHADER_RESOURCE_VIEW_DESC& GetD3D12ShaderResourceView() const { return m_d3d12ShaderResourceView; }

	const D3D12_UNORDERED_ACCESS_VIEW_DESC& GetD3D12UnorderedAccessView(uint32_t mip) const { return m_additionalViews->m_d3d12UAVSingleMipAllSlices[mip]; }

	uint32_t GetD3D12SubresourceCount() const { return m_d3d12SubresourceCount; }

private:

	// Main resource view, can access all mips and slices
	D3D12_SHADER_RESOURCE_VIEW_DESC m_d3d12ShaderResourceView;

	CrUniquePtr<CrD3D12AdditionalTextureViews> m_additionalViews;

	uint32_t m_d3d12SubresourceCount;

	D3D12_RESOURCE_STATES m_d3d12InitialState;

	ID3D12Resource* m_d3d12Resource = nullptr;
};
