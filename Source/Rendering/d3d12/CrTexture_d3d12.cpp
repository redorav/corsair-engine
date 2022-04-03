#include "CrRendering_pch.h"

#include "CrCommandBuffer_d3d12.h"
#include "CrTexture_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrD3D12.h"

#include "Core/CrAlignment.h"

#include "Core/Logging/ICrDebug.h"

CrTextureD3D12::CrTextureD3D12(ICrRenderDevice* renderDevice, const CrTextureDescriptor& descriptor)
	: ICrTexture(renderDevice, descriptor)
{
	CrRenderDeviceD3D12* d3d12RenderDevice = static_cast<CrRenderDeviceD3D12*>(renderDevice);
	ID3D12Device* d3d12Device = d3d12RenderDevice->GetD3D12Device();

	DXGI_FORMAT dxgiFormat = crd3d::GetDXGIFormat(descriptor.format);

	D3D12_RESOURCE_DESC resourceDescriptor = {};
	resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDescriptor.Width = m_width;
	resourceDescriptor.Height = m_height;
	resourceDescriptor.MipLevels = (UINT16)m_mipmapCount;
	resourceDescriptor.Format = dxgiFormat;
	resourceDescriptor.SampleDesc.Count = crd3d::GetD3D12SampleCount(descriptor.sampleCount);
	resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	if (IsDepthStencil())
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
		m_d3d12Resource = (ID3D12Resource*)descriptor.extraDataPtr;
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
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&m_d3d12Resource)
		);

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
	if (IsDepthStencil())
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

		m_additionalViews->m_d3d12DSVSingleMipSlice = d3d12RenderDevice->AllocateDSVDescriptor();
		d3d12Device->CreateDepthStencilView(m_d3d12Resource, &dsvDescriptor, m_additionalViews->m_d3d12DSVSingleMipSlice.cpuHandle);
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
	{
		m_d3d12ShaderResourceView = {};
		m_d3d12ShaderResourceView.Format = dxgiFormat;
		m_d3d12ShaderResourceView.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		if (IsCubemap())
		{
			m_d3d12ShaderResourceView.TextureCube.MostDetailedMip = 0;
			m_d3d12ShaderResourceView.TextureCube.MipLevels = m_mipmapCount;

			if (m_arraySize > 1)
			{
				m_d3d12ShaderResourceView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
				m_d3d12ShaderResourceView.TextureCubeArray.First2DArrayFace = 0;
				m_d3d12ShaderResourceView.TextureCubeArray.NumCubes = m_arraySize;
			}
			else
			{
				m_d3d12ShaderResourceView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			}
		}
		else if (IsVolumeTexture())
		{
			m_d3d12ShaderResourceView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			m_d3d12ShaderResourceView.Texture3D.MostDetailedMip = 0;
			m_d3d12ShaderResourceView.Texture3D.MipLevels = m_mipmapCount;
		}
		else if (Is1DTexture())
		{
			m_d3d12ShaderResourceView.Texture1D.MostDetailedMip = 0;
			m_d3d12ShaderResourceView.Texture1D.MipLevels = m_mipmapCount;

			if (m_arraySize > 1)
			{
				m_d3d12ShaderResourceView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
				m_d3d12ShaderResourceView.Texture1DArray.FirstArraySlice = 0;
				m_d3d12ShaderResourceView.Texture1DArray.ArraySize = m_arraySize;
			}
			else
			{
				m_d3d12ShaderResourceView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
			}
		}
		else
		{
			m_d3d12ShaderResourceView.Texture2D.MostDetailedMip = 0;
			m_d3d12ShaderResourceView.Texture2D.MipLevels = m_mipmapCount;

			if (m_arraySize > 1)
			{
				m_d3d12ShaderResourceView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				m_d3d12ShaderResourceView.Texture2DArray.FirstArraySlice = 0;
				m_d3d12ShaderResourceView.Texture2DArray.ArraySize = m_arraySize;
				m_d3d12ShaderResourceView.Texture2DArray.PlaneSlice = 0;
			}
			else
			{
				m_d3d12ShaderResourceView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				m_d3d12ShaderResourceView.Texture2D.PlaneSlice = 0;
			}
		}
	}

	d3d12RenderDevice->SetD3D12ObjectName(m_d3d12Resource, descriptor.name.c_str());

	uint32_t subResourceCount = resourceDescriptor.MipLevels;
	if (resourceDescriptor.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
	{
		subResourceCount *= resourceDescriptor.DepthOrArraySize;
	}

	// Query the layout. We only care about the first 32 subresources because we only need info for the mip levels for the first two faces / slices
	const uint32_t MaxSubresources = 32;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT subresourceFootprints[MaxSubresources];
	UINT numRows[MaxSubresources];
	UINT64 rowSizeInBytes[MaxSubresources];
	UINT64 totalBytes;
	d3d12RenderDevice->GetD3D12Device()->GetCopyableFootprints
	(
		&resourceDescriptor,
		0, // First subresource
		subResourceCount < MaxSubresources ? subResourceCount : MaxSubresources,
		0, // base offset
		subresourceFootprints, numRows, rowSizeInBytes, &totalBytes
	);

	if (descriptor.initialData)
	{
		CrHardwareGPUBufferDescriptor stagingBufferDescriptor(cr3d::BufferUsage::TransferSrc, cr3d::MemoryAccess::Staging, (uint32_t)totalBytes);
		CrSharedPtr<ICrHardwareGPUBuffer> stagingBuffer = d3d12RenderDevice->CreateHardwareGPUBuffer(stagingBufferDescriptor);
		CrHardwareGPUBufferD3D12* d3d12StagingBuffer = static_cast<CrHardwareGPUBufferD3D12*>(stagingBuffer.get());

		// TODO Rework how this all works. We shouldn't be stalling here or creating new command buffers.
		// However, changing this requires more framework to be in place
		const CrCommandBufferSharedHandle& comandBuffer = d3d12RenderDevice->GetAuxiliaryCommandBuffer();
		CrCommandBufferD3D12* d3d12CommandBuffer = static_cast<CrCommandBufferD3D12*>(comandBuffer.get());
		d3d12CommandBuffer->Begin();
		{
			uint8_t* data = (uint8_t*)stagingBuffer->Lock();

			for (uint32_t mip = 0; mip < m_mipmapCount; ++mip)
			{
				uint32_t subresourceIndex = crd3d::CalculateSubresource(mip, 0, 0, m_mipmapCount, m_arraySize);

				const D3D12_PLACED_SUBRESOURCE_FOOTPRINT& subresourceFootprint = subresourceFootprints[subresourceIndex];
				uint32_t rowCount = numRows[subresourceIndex];
				uint32_t rowPitch = subresourceFootprint.Footprint.RowPitch;
				uint32_t srcRowPitch = subresourceFootprints[0].Footprint.RowPitch >> mip; // TODO Fix this awful hack

				for (uint32_t row = 0; row < rowCount; ++row)
				{
					memcpy
					(
						data + subresourceFootprint.Offset + rowPitch * row,
						descriptor.initialData + GetMipSliceOffset(mip, 0) + srcRowPitch * row, // TODO This is DDS-specific
						rowSizeInBytes[subresourceIndex]
					);
				}

				D3D12_TEXTURE_COPY_LOCATION textureCopySource = {};
				textureCopySource.pResource = d3d12StagingBuffer->GetD3D12Buffer();
				textureCopySource.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				textureCopySource.PlacedFootprint = subresourceFootprints[subresourceIndex];

				D3D12_TEXTURE_COPY_LOCATION textureCopyDestination = {};
				textureCopyDestination.pResource = m_d3d12Resource;
				textureCopyDestination.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				textureCopyDestination.SubresourceIndex = subresourceIndex;

				d3d12CommandBuffer->GetD3D12CommandList()->CopyTextureRegion(&textureCopyDestination, 0, 0, 0, &textureCopySource, nullptr);
			}

			stagingBuffer->Unlock();

			// TODO Handle transition to default state
		}
		d3d12CommandBuffer->End();
		d3d12CommandBuffer->Submit();
		renderDevice->WaitIdle();
	}
}

CrTextureD3D12::~CrTextureD3D12()
{
	CrRenderDeviceD3D12* renderDeviceD3D12 = static_cast<CrRenderDeviceD3D12*>(m_renderDevice);

	if (IsRenderTarget())
	{
		for (const auto& sliceArray : m_additionalViews->m_d3d12RTVSingleMipSlice)
		{
			for (const auto& descriptor : sliceArray)
			{
				renderDeviceD3D12->FreeRTVDescriptor(descriptor);
			}
		}

		for (const auto& descriptor : m_additionalViews->m_d3d12RTVSingleMipAllSlices)
		{
			renderDeviceD3D12->FreeRTVDescriptor(descriptor);
		}
	}

	if (IsDepthStencil())
	{
		renderDeviceD3D12->FreeDSVDescriptor(m_additionalViews->m_d3d12DSVSingleMipSlice);
	}

	// Don't release resources we don't manage. The swapchain resource was handed to us by the OS
	if (!IsSwapchain())
	{
		m_d3d12Resource->Release();
	}
}