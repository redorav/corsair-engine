#include "CrRendering_pch.h"

#include "CrCommandQueue_d3d12.h"
#include "CrCommandBuffer_d3d12.h"
#include "CrTexture_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrD3D12.h"

#include "Core/Logging/ICrDebug.h"

CrTextureD3D12::CrTextureD3D12(ICrRenderDevice* renderDevice, const CrTextureCreateParams& params)
	: ICrTexture(renderDevice, params)
{
	CrRenderDeviceD3D12* renderDeviceD3D12 = static_cast<CrRenderDeviceD3D12*>(renderDevice);
	ID3D12Device* d3d12Device = renderDeviceD3D12->GetD3D12Device();

	DXGI_FORMAT dxgiFormat = crd3d::GetD3DFormat(params.format);

	D3D12_RESOURCE_DESC resourceDescriptor = {};
	resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDescriptor.Width = m_width;
	resourceDescriptor.Height = m_height;
	resourceDescriptor.MipLevels = (UINT16)m_numMipmaps;
	resourceDescriptor.Format = dxgiFormat;
	resourceDescriptor.SampleDesc.Count = crd3d::GetD3D12SampleCount(params.sampleCount);
	resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	if (IsDepth())
	{
		resourceDescriptor.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}
	
	if (IsRenderTarget())
	{
		resourceDescriptor.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}

	if (IsUnorderedAccess())
	{
		resourceDescriptor.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	if (IsCubemap())
	{
		resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDescriptor.DepthOrArraySize = (UINT16)(6 * m_arraySize);
	}
	else if(IsVolumeTexture())
	{
		resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		resourceDescriptor.DepthOrArraySize = (UINT16)m_depth;
	}
	else if (Is1DTexture())
	{
		resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
		resourceDescriptor.DepthOrArraySize = (UINT16)m_arraySize;
	}
	else
	{
		resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDescriptor.DepthOrArraySize = (UINT16)m_arraySize;
	}

	if (IsSwapchain())
	{
		m_d3d12TextureResource = (ID3D12Resource*)params.extraDataPtr;
	}
	else
	{
		D3D12_RESOURCE_ALLOCATION_INFO resourceAllocationInfo = d3d12Device->GetResourceAllocationInfo(0, 1, &resourceDescriptor);

		// TODO Create resource heap in render device to use CreatePlacedResource
		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

		HRESULT hResult = d3d12Device->CreateCommittedResource
		(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDescriptor,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_d3d12TextureResource)
		);

		CrAssertMsg(SUCCEEDED(hResult), "Failed to create texture");
	}

	// Create Render Target views
	if (IsSwapchain() || IsRenderTarget())
	{
		D3D12_RENDER_TARGET_VIEW_DESC rtvDescriptor = {};
		rtvDescriptor.Format = dxgiFormat;

		// Set the view dimensions depending on the texture type
		if (IsVolumeTexture())
		{
			rtvDescriptor.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
		}
		else if (IsCubemap())
		{
			rtvDescriptor.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		}
		else if(Is1DTexture())
		{
			rtvDescriptor.ViewDimension = m_arraySize > 1 ? D3D12_RTV_DIMENSION_TEXTURE1DARRAY : D3D12_RTV_DIMENSION_TEXTURE1D;
		}
		else
		{
			rtvDescriptor.ViewDimension = m_arraySize > 1 ? D3D12_RTV_DIMENSION_TEXTURE2DARRAY : D3D12_RTV_DIMENSION_TEXTURE2D;
		}

		// Create one render target view for each mip/slice combination
		for (uint32_t mip = 0; mip < m_numMipmaps; ++mip)
		{
			m_renderTargetViews[mip].resize(m_arraySize);
			rtvDescriptor.Texture2D.MipSlice = mip;

			for (uint32_t slice = 0; slice < m_arraySize; ++slice)
			{
				// Allocate RTV descriptor from render device
				rtvDescriptor.Texture2DArray.FirstArraySlice = slice;
				rtvDescriptor.Texture2DArray.ArraySize = 1;
				m_renderTargetViews[mip][slice] = renderDeviceD3D12->AllocateRTVDescriptor();
				d3d12Device->CreateRenderTargetView(m_d3d12TextureResource, &rtvDescriptor, m_renderTargetViews[mip][slice].cpuHandle);
			}
		}
	}

	if (IsDepth())
	{
		// Map texture formats to depth stencil formats
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDescriptor = {};
		dsvDescriptor.Flags = D3D12_DSV_FLAG_NONE;
		dsvDescriptor.Format = dxgiFormat;

		if (dxgiFormat == DXGI_FORMAT_R32G8X24_TYPELESS)
		{
			dsvDescriptor.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		}
		else if (dxgiFormat == DXGI_FORMAT_R32_TYPELESS)
		{
			dsvDescriptor.Format = DXGI_FORMAT_D32_FLOAT;
		}

		// Set the view dimensions depending on the texture type
		if (IsCubemap())
		{
			dsvDescriptor.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		}
		else if (Is1DTexture())
		{
			dsvDescriptor.ViewDimension = m_arraySize > 1 ? D3D12_DSV_DIMENSION_TEXTURE1DARRAY : D3D12_DSV_DIMENSION_TEXTURE1D;
		}
		else
		{
			dsvDescriptor.ViewDimension = m_arraySize > 1 ? D3D12_DSV_DIMENSION_TEXTURE2DARRAY : D3D12_DSV_DIMENSION_TEXTURE2D;
		}

		// Create one render target view for each mip/slice combination
		for (uint32_t mip = 0; mip < m_numMipmaps; ++mip)
		{
			m_renderTargetViews[mip].resize(m_arraySize);
			dsvDescriptor.Texture2D.MipSlice = mip;

			for (uint32_t slice = 0; slice < m_arraySize; ++slice)
			{
				// Allocate RTV descriptor from render device
				dsvDescriptor.Texture2DArray.FirstArraySlice = slice;
				dsvDescriptor.Texture2DArray.ArraySize = 1;
				m_renderTargetViews[mip][slice] = renderDeviceD3D12->AllocateDSVDescriptor();
				d3d12Device->CreateDepthStencilView(m_d3d12TextureResource, &dsvDescriptor, m_renderTargetViews[mip][slice].cpuHandle);
			}
		}
	}
}

CrTextureD3D12::~CrTextureD3D12()
{
	CrRenderDeviceD3D12* renderDeviceD3D12 = static_cast<CrRenderDeviceD3D12*>(m_renderDevice);

	// TODO Returns descriptors to their pools
	if (IsRenderTarget())
	{
		for (const auto& sliceArray : m_renderTargetViews)
		{
			for (const auto& descriptor : sliceArray)
			{
				renderDeviceD3D12->FreeRTVDescriptor(descriptor);
			}
		}
	}

	// Don't release resources we don't manage. The swapchain resource was handed to us by the OS
	if (!IsSwapchain())
	{
		m_d3d12TextureResource->Release();
	}
}