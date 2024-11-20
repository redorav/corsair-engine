#include "Rendering/CrRendering_pch.h"

#include "CrCommandBuffer_d3d12.h"
#include "CrTexture_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrD3D12.h"

#include "Core/CrAlignment.h"
#include "Core/Logging/ICrDebug.h"

#include "Math/CrMath.h"

CrTextureD3D12::CrTextureD3D12(ICrRenderDevice* renderDevice, const CrTextureDescriptor& descriptor)
	: ICrTexture(renderDevice, descriptor)
{
	CrRenderDeviceD3D12* d3d12RenderDevice = static_cast<CrRenderDeviceD3D12*>(renderDevice);
	ID3D12Device* d3d12Device = d3d12RenderDevice->GetD3D12Device();
	ID3D12Device10* d3d12Device10 = d3d12RenderDevice->GetD3D12Device10();

	m_d3d12LegacyInitialState = crd3d::GetD3D12LegacyResourceState(m_defaultState);
	m_d3d12InitialLayout = crd3d::GetD3D12BarrierTextureLayout(m_defaultState.layout);

	DXGI_FORMAT dxgiFormat = crd3d::GetDXGIFormat(descriptor.format);

	D3D12_RESOURCE_DESC1 d3d12ResourceDescriptor = {};
	d3d12ResourceDescriptor.Width = m_width;
	d3d12ResourceDescriptor.Height = m_height;
	d3d12ResourceDescriptor.MipLevels = (UINT16)m_mipmapCount;
	d3d12ResourceDescriptor.Format = dxgiFormat;
	d3d12ResourceDescriptor.SampleDesc.Count = crd3d::GetD3D12SampleCount(descriptor.sampleCount);
	d3d12ResourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	if (IsDepthStencil())
	{
		d3d12ResourceDescriptor.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}
	
	if (IsRenderTarget())
	{
		d3d12ResourceDescriptor.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}

	if (IsUnorderedAccess())
	{
		d3d12ResourceDescriptor.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	if (IsCubemap())
	{
		d3d12ResourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		d3d12ResourceDescriptor.DepthOrArraySize = (UINT16)(6 * m_arraySize);
	}
	else if(IsVolumeTexture())
	{
		d3d12ResourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		d3d12ResourceDescriptor.DepthOrArraySize = (UINT16)m_depth;
	}
	else if (Is1DTexture())
	{
		d3d12ResourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
		d3d12ResourceDescriptor.DepthOrArraySize = (UINT16)m_arraySize;
	}
	else
	{
		d3d12ResourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		d3d12ResourceDescriptor.DepthOrArraySize = (UINT16)m_arraySize;
	}

	if (IsSwapchain())
	{
		m_d3d12Resource = (ID3D12Resource*)descriptor.extraDataPtr;
	}
	else
	{
		// TODO Create resource heap in render device to use CreatePlacedResource
		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

		bool useOptimizedClearValue = IsRenderTarget();

		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = dxgiFormat;

		if (cr3d::IsDepthFormat(descriptor.format))
		{
			clearValue.DepthStencil.Depth = descriptor.depthClear;
			clearValue.DepthStencil.Stencil = descriptor.stencilClear;
		}
		else
		{
			clearValue.Color[0] = descriptor.colorClear[0];
			clearValue.Color[1] = descriptor.colorClear[1];
			clearValue.Color[2] = descriptor.colorClear[2];
			clearValue.Color[3] = descriptor.colorClear[3];
		}

		HRESULT hResult = S_FALSE;

		if (d3d12RenderDevice->GetIsEnhancedBarriersSupported())
		{
			hResult = d3d12Device10->CreateCommittedResource3
			(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&d3d12ResourceDescriptor,
				m_d3d12InitialLayout,
				useOptimizedClearValue ? &clearValue : nullptr,
				nullptr,
				0,
				nullptr,
				IID_PPV_ARGS(&m_d3d12Resource)
			);
		}
		else
		{
			hResult = d3d12Device->CreateCommittedResource
			(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				(D3D12_RESOURCE_DESC*)&d3d12ResourceDescriptor,
				m_d3d12LegacyInitialState,
				useOptimizedClearValue ? &clearValue : nullptr,
				IID_PPV_ARGS(&m_d3d12Resource)
			);
		}

		CrAssertMsg(SUCCEEDED(hResult), "Failed to create texture");
	}

	if (IsRenderTarget() || IsDepthStencil() || IsUnorderedAccess() || IsSwapchain())
	{
		m_additionalViews = CrUniquePtr<CrD3D12AdditionalTextureViews>(new CrD3D12AdditionalTextureViews());
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
		for (uint32_t mip = 0; mip < m_mipmapCount; ++mip)
		{
			rtvDescriptor.Texture2D.MipSlice = mip;

			m_additionalViews->m_d3d12RTVSingleMipSlice[mip].resize(m_arraySize);

			for (uint32_t slice = 0; slice < m_arraySize; ++slice)
			{
				// Allocate RTV descriptor from render device
				rtvDescriptor.Texture2DArray.FirstArraySlice = slice;
				rtvDescriptor.Texture2DArray.ArraySize = 1;
				m_additionalViews->m_d3d12RTVSingleMipSlice[mip][slice] = d3d12RenderDevice->AllocateRTVDescriptor();
				d3d12Device->CreateRenderTargetView(m_d3d12Resource, &rtvDescriptor, m_additionalViews->m_d3d12RTVSingleMipSlice[mip][slice].cpuHandle);
			}
		}
	}

	// Create depth stencil views
	// https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_depth_stencil_view_desc
	if (IsDepthStencil())
	{
		// Map texture formats to depth stencil formats
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDescriptor = {};
		dsvDescriptor.Flags = D3D12_DSV_FLAG_NONE;
		dsvDescriptor.Format = DXGI_FORMAT_UNKNOWN;

		switch (descriptor.format)
		{
			case cr3d::DataFormat::D16_Unorm: dsvDescriptor.Format = DXGI_FORMAT_D16_UNORM; break;
			case cr3d::DataFormat::D32_Float: dsvDescriptor.Format = DXGI_FORMAT_D32_FLOAT; break;
			case cr3d::DataFormat::D24_Unorm_S8_Uint: dsvDescriptor.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; break;
			case cr3d::DataFormat::D32_Float_S8_Uint: dsvDescriptor.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT; break;
			default: break;
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

		m_additionalViews->m_d3d12DSVSingleMipSlice = d3d12RenderDevice->AllocateDSVDescriptor();
		d3d12Device->CreateDepthStencilView(m_d3d12Resource, &dsvDescriptor, m_additionalViews->m_d3d12DSVSingleMipSlice.cpuHandle);

		if (cr3d::IsDepthStencilFormat(descriptor.format))
		{
			dsvDescriptor.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
			m_additionalViews->m_d3d12DSVSingleMipSliceReadOnlyDepth = d3d12RenderDevice->AllocateDSVDescriptor();
			d3d12Device->CreateDepthStencilView(m_d3d12Resource, &dsvDescriptor, m_additionalViews->m_d3d12DSVSingleMipSliceReadOnlyDepth.cpuHandle);

			dsvDescriptor.Flags = D3D12_DSV_FLAG_READ_ONLY_STENCIL;
			m_additionalViews->m_d3d12DSVSingleMipSliceReadOnlyStencil = d3d12RenderDevice->AllocateDSVDescriptor();
			d3d12Device->CreateDepthStencilView(m_d3d12Resource, &dsvDescriptor, m_additionalViews->m_d3d12DSVSingleMipSliceReadOnlyStencil.cpuHandle);
		}
	}

	// Create unordered access views
	if (IsUnorderedAccess())
	{
		for (uint32_t i = 0; i < m_additionalViews->m_d3d12UAVSingleMipAllSlices.size(); ++i)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC& uavDescriptor = m_additionalViews->m_d3d12UAVSingleMipAllSlices[i];
			uavDescriptor.Format = dxgiFormat;
			uavDescriptor.Texture2DArray.MipSlice = i;

			if (IsCubemap())
			{
				uavDescriptor.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				uavDescriptor.Texture2DArray.FirstArraySlice = 0;
				uavDescriptor.Texture2DArray.ArraySize = m_arraySize * 6;
				uavDescriptor.Texture2DArray.PlaneSlice = 0;
			}
			else if (IsVolumeTexture())
			{
				uavDescriptor.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
				uavDescriptor.Texture3D.FirstWSlice = 0;
				uavDescriptor.Texture3D.WSize = (UINT)-1;
			}
			else if (Is1DTexture())
			{
				if (m_arraySize > 1)
				{
					uavDescriptor.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
					uavDescriptor.Texture1DArray.FirstArraySlice = 0;
					uavDescriptor.Texture1DArray.ArraySize = m_arraySize;
				}
				else
				{
					uavDescriptor.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
				}
			}
			else
			{
				if (m_arraySize > 1)
				{
					uavDescriptor.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
					uavDescriptor.Texture2DArray.FirstArraySlice = 0;
					uavDescriptor.Texture2DArray.ArraySize = m_arraySize;
					uavDescriptor.Texture2DArray.PlaneSlice = 0;
				}
				else
				{
					uavDescriptor.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
					uavDescriptor.Texture2D.PlaneSlice = 0;
				}
			}
		}
	}

	// Create shader resource views

	DXGI_FORMAT srvFormat = dxgiFormat;

	switch (descriptor.format)
	{
		case cr3d::DataFormat::D16_Unorm: srvFormat = DXGI_FORMAT_R16_UNORM; break;
		case cr3d::DataFormat::D32_Float: srvFormat = DXGI_FORMAT_R32_FLOAT; break;
		case cr3d::DataFormat::D24_Unorm_S8_Uint: srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; break;
		case cr3d::DataFormat::D32_Float_S8_Uint: srvFormat = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS; break;
		default: break;
	}

	m_d3d12SRVDescriptor = {};
	m_d3d12SRVDescriptor.Format = srvFormat;
	m_d3d12SRVDescriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	if (IsCubemap())
	{
		m_d3d12SRVDescriptor.TextureCube.MostDetailedMip = 0;
		m_d3d12SRVDescriptor.TextureCube.MipLevels = m_mipmapCount;

		if (m_arraySize > 1)
		{
			m_d3d12SRVDescriptor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
			m_d3d12SRVDescriptor.TextureCubeArray.First2DArrayFace = 0;
			m_d3d12SRVDescriptor.TextureCubeArray.NumCubes = m_arraySize;
		}
		else
		{
			m_d3d12SRVDescriptor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		}
	}
	else if (IsVolumeTexture())
	{
		m_d3d12SRVDescriptor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		m_d3d12SRVDescriptor.Texture3D.MostDetailedMip = 0;
		m_d3d12SRVDescriptor.Texture3D.MipLevels = m_mipmapCount;
	}
	else if (Is1DTexture())
	{
		m_d3d12SRVDescriptor.Texture1D.MostDetailedMip = 0;
		m_d3d12SRVDescriptor.Texture1D.MipLevels = m_mipmapCount;

		if (m_arraySize > 1)
		{
			m_d3d12SRVDescriptor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
			m_d3d12SRVDescriptor.Texture1DArray.FirstArraySlice = 0;
			m_d3d12SRVDescriptor.Texture1DArray.ArraySize = m_arraySize;
		}
		else
		{
			m_d3d12SRVDescriptor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
		}
	}
	else
	{
		m_d3d12SRVDescriptor.Texture2D.MostDetailedMip = 0;
		m_d3d12SRVDescriptor.Texture2D.MipLevels = m_mipmapCount;

		if (m_arraySize > 1)
		{
			m_d3d12SRVDescriptor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			m_d3d12SRVDescriptor.Texture2DArray.FirstArraySlice = 0;
			m_d3d12SRVDescriptor.Texture2DArray.ArraySize = m_arraySize;
			m_d3d12SRVDescriptor.Texture2DArray.PlaneSlice = 0;
		}
		else
		{
			m_d3d12SRVDescriptor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			m_d3d12SRVDescriptor.Texture2D.PlaneSlice = 0;
		}
	}

	if (IsDepthStencil())
	{
		m_additionalViews->m_d3d12StencilSRVDescriptor = m_d3d12SRVDescriptor;
		m_additionalViews->m_d3d12StencilSRVDescriptor.Format = DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
		if (m_arraySize > 1)
		{
			m_additionalViews->m_d3d12StencilSRVDescriptor.Texture2DArray.PlaneSlice = 1;
		}
		else
		{
			m_additionalViews->m_d3d12StencilSRVDescriptor.Texture2D.PlaneSlice = 1;
		}
	}

	d3d12RenderDevice->SetD3D12ObjectName(m_d3d12Resource, descriptor.name);

	// Calculate number of subresources by computing the last subresource in the resource
	m_d3d12SubresourceCount = crd3d::CalculateSubresource(m_mipmapCount - 1, m_arraySize - 1, 0, m_mipmapCount, m_arraySize) + 1;

	D3D12_RESOURCE_ALLOCATION_INFO d3d12ResourceAllocationInfo = d3d12RenderDevice->GetD3D12Device()->GetResourceAllocationInfo(0, 1, (D3D12_RESOURCE_DESC*)&d3d12ResourceDescriptor);
	m_usedGPUMemoryBytes = (uint32_t)d3d12ResourceAllocationInfo.SizeInBytes;

	// Prepare the hardware mipmap layout
	{
		// Calculate twice the number of subresources for array textures, so we can calculate the slice pitch
		const uint32_t MaxSubresources = 2 * cr3d::MaxMipmaps;
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT subresourceFootprints[MaxSubresources];
		UINT numRows[MaxSubresources];
		d3d12RenderDevice->GetD3D12Device()->GetCopyableFootprints((D3D12_RESOURCE_DESC*)&d3d12ResourceDescriptor, 0, CrMin(m_d3d12SubresourceCount, MaxSubresources), 0, subresourceFootprints, numRows, nullptr, nullptr);

		for (uint32_t mip = 0; mip < m_mipmapCount; ++mip)
		{
			uint32_t subresourceIndex = crd3d::CalculateSubresource(mip, 0, 0, m_mipmapCount, m_arraySize);
			cr3d::MipmapLayout& mipmapLayout  = m_hardwareMipmapLayouts[mip];
			mipmapLayout.rowPitchBytes        = subresourceFootprints[subresourceIndex].Footprint.RowPitch;
			mipmapLayout.offsetBytes          = (uint32_t)subresourceFootprints[subresourceIndex].Offset;
			mipmapLayout.heightInPixelsBlocks = numRows[subresourceIndex];
		}

		if (m_arraySize > 1)
		{
			// The offset for the first mip of the second slice is the distance between slices
			m_slicePitchBytes = (uint32_t)subresourceFootprints[m_mipmapCount].Offset;
		}
		else
		{
			m_slicePitchBytes = 0;
		}
	}

	if (descriptor.initialData)
	{
		uint8_t* textureData = m_renderDevice->BeginTextureUpload(this);
		{
			CopyIntoTextureMemory(textureData, descriptor.initialData, 0, m_mipmapCount, 0, m_arraySize);
		}
		m_renderDevice->EndTextureUpload(this);
	}
}

CrTextureD3D12::~CrTextureD3D12()
{
	CrRenderDeviceD3D12* renderDeviceD3D12 = static_cast<CrRenderDeviceD3D12*>(m_renderDevice);

	if (IsRenderTarget())
	{
		for (size_t mip = 0; mip < m_mipmapCount; ++mip)
		{
			const auto& sliceArray = m_additionalViews->m_d3d12RTVSingleMipSlice[mip];

			for (const auto& descriptor : sliceArray)
			{
				renderDeviceD3D12->FreeRTVDescriptor(descriptor);
			}

			//const auto& singleMipDescriptor = m_additionalViews->m_d3d12RTVSingleMipAllSlices[mip];
			//renderDeviceD3D12->FreeRTVDescriptor(singleMipDescriptor);
		}
	}

	if (IsDepthStencil())
	{
		renderDeviceD3D12->FreeDSVDescriptor(m_additionalViews->m_d3d12DSVSingleMipSlice);

		if (cr3d::IsDepthStencilFormat(m_format))
		{
			renderDeviceD3D12->FreeDSVDescriptor(m_additionalViews->m_d3d12DSVSingleMipSliceReadOnlyDepth);
			renderDeviceD3D12->FreeDSVDescriptor(m_additionalViews->m_d3d12DSVSingleMipSliceReadOnlyStencil);
		}
	}

	// Don't release the swapchain texture in the destructor, as we'll destroy it manually in the swapchain class
	// This simplifies our model greatly as we don't need to go through the deletion queue
	if (!IsSwapchain())
	{
		m_d3d12Resource->Release();
	}
}