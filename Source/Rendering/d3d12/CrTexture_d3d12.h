#pragma once

#include "Rendering/ICrTexture.h"

#include "CrD3D12.h"

#include "Core/Containers/CrArray.h"
#include "Core/Containers/CrVector.h"

#include <d3d12.h>

class ICrRenderDevice;

class CrTextureD3D12 final : public ICrTexture
{
public:

	CrTextureD3D12(ICrRenderDevice* renderDevice, const CrTextureDescriptor& descriptor);

	~CrTextureD3D12();

private:

	// Main resource view, can view all mips and slices
	D3D12_CPU_DESCRIPTOR_HANDLE m_shaderResourceView;

	// Main resource view, can render to all mips and slices (via SV_RenderTargetArrayIndex)
	D3D12_CPU_DESCRIPTOR_HANDLE m_renderTargetView;

	CrArray<CrVector<D3D12_CPU_DESCRIPTOR_HANDLE>, MaxMipmaps> m_shaderResourceViews; // One per mip, per slice

	CrArray<CrVector<crd3d::DescriptorD3D12>, MaxMipmaps> m_renderTargetViews; // One per mip, per slice

	CrArray<D3D12_CPU_DESCRIPTOR_HANDLE, MaxMipmaps> m_unorderedAccessViews; // One per mip

	ID3D12Resource* m_d3d12TextureResource;

	// Main view, can access all mips and slices
	D3D12_CPU_DESCRIPTOR_HANDLE m_d3d12ShaderResourceView;
};
