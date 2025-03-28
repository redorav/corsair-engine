#pragma once

#include "Rendering/ICrTexture.h"

#include "CrD3D12.h"

#include "crstl/array.h"
#include "crstl/vector.h"

class ICrRenderDevice;

struct CrD3D12AdditionalTextureViews
{
	// RTVs
	crstl::array<crstl::vector<crd3d::DescriptorD3D12>, cr3d::MaxMipmaps> m_d3d12RTVSingleMipSlice; // Each mipmap can have a variable amount of slices
	crstl::array<crd3d::DescriptorD3D12, cr3d::MaxMipmaps> m_d3d12RTVSingleMipAllSlices; // Each mipmap can see all slices (via SV_RenderTargetArrayIndex)

	// DSVs
	crd3d::DescriptorD3D12 m_d3d12DSVSingleMipSlice;

	crd3d::DescriptorD3D12 m_d3d12DSVSingleMipSliceReadOnlyDepth;

	crd3d::DescriptorD3D12 m_d3d12DSVSingleMipSliceReadOnlyStencil;

	// UAVs
	crstl::array<D3D12_UNORDERED_ACCESS_VIEW_DESC, cr3d::MaxMipmaps>	m_d3d12UAVSingleMipAllSlices; // Each mipmap can see all slices

	D3D12_SHADER_RESOURCE_VIEW_DESC m_d3d12StencilSRVDescriptor;
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

	D3D12_RESOURCE_STATES GetD3D12DefaultLegacyResourceState() const { return m_d3d12LegacyInitialState; }

	D3D12_BARRIER_LAYOUT GetD3D12DefaultTextureLayout() const { return m_d3d12InitialLayout; }

	const D3D12_SHADER_RESOURCE_VIEW_DESC& GetD3D12SRVDescriptor() const { return m_d3d12SRVDescriptor; }

	const D3D12_SHADER_RESOURCE_VIEW_DESC& GetD3D12StencilSRVDescriptor() const { return m_additionalViews->m_d3d12StencilSRVDescriptor; }

	const D3D12_UNORDERED_ACCESS_VIEW_DESC& GetD3D12UAVDescriptor(uint32_t mip) const { return m_additionalViews->m_d3d12UAVSingleMipAllSlices[mip]; }

	uint32_t GetD3D12SubresourceCount() const { return m_d3d12SubresourceCount; }

private:

	// Main resource view, can access all mips and slices
	D3D12_SHADER_RESOURCE_VIEW_DESC m_d3d12SRVDescriptor;

	crstl::unique_ptr<CrD3D12AdditionalTextureViews> m_additionalViews;

	uint32_t m_d3d12SubresourceCount;

	D3D12_RESOURCE_STATES m_d3d12LegacyInitialState;
	D3D12_BARRIER_LAYOUT m_d3d12InitialLayout;

	ID3D12Resource* m_d3d12Resource = nullptr;
};
