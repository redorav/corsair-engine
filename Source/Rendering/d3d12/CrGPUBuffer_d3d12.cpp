#include "Rendering/CrRendering_pch.h"

#include "CrGPUBuffer_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrD3D12.h"

#include "Core/Logging/ICrDebug.h"

CrHardwareGPUBufferD3D12::CrHardwareGPUBufferD3D12(CrRenderDeviceD3D12* d3d12RenderDevice, const CrHardwareGPUBufferDescriptor& descriptor)
	: ICrHardwareGPUBuffer(d3d12RenderDevice, descriptor)
{
	m_d3d12Device = d3d12RenderDevice->GetD3D12Device();

	D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON;

	D3D12_HEAP_PROPERTIES heapProperties;
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

	switch (descriptor.access)
	{
		case cr3d::MemoryAccess::GPUOnlyWrite:
		case cr3d::MemoryAccess::GPUOnlyRead:
			heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
			break;
		case cr3d::MemoryAccess::GPUWriteCPURead:
			heapProperties.Type = D3D12_HEAP_TYPE_READBACK;
			initialResourceState = D3D12_RESOURCE_STATE_COPY_DEST;
			break;
		case cr3d::MemoryAccess::StagingUpload:
			heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
			initialResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
			break;
		case cr3d::MemoryAccess::StagingDownload:
			heapProperties.Type = D3D12_HEAP_TYPE_READBACK;
			initialResourceState = D3D12_RESOURCE_STATE_COPY_DEST;
			break;
		case cr3d::MemoryAccess::CPUStreamToGPU:
			heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
			initialResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
			break;
		default:
			break;
	}

	m_d3d12InitialState = initialResourceState;

	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDesc;
	resourceDesc.Alignment = 0;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.Height = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc = { 1, 0 };
	resourceDesc.Width = m_sizeBytes;

	if (descriptor.access == cr3d::MemoryAccess::GPUOnlyWrite)
	{
		resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	D3D12_CLEAR_VALUE optimizedClearValue = {};

	HRESULT hResult = S_OK;

	hResult = m_d3d12Device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, initialResourceState, nullptr, IID_PPV_ARGS(&m_d3d12Resource));

	CrAssertMsg(hResult == S_OK, "Failed to create buffer");

	if (descriptor.initialData)
	{
		CrAssertMsg(descriptor.initialDataSize <= m_sizeBytes, "Not enough memory in buffer");

		if (descriptor.access == cr3d::MemoryAccess::GPUOnlyWrite)
		{
			uint8_t* bufferData = m_renderDevice->BeginBufferUpload(this);
			{
				memcpy(bufferData, descriptor.initialData, descriptor.initialDataSize);
			}
			m_renderDevice->EndBufferUpload(this);
		}
		else
		{
			void* data;
			hResult = m_d3d12Resource->Map(0, nullptr, &data);
			CrAssertMsg(hResult == S_OK, "Failed to map buffer");
			memcpy(data, descriptor.initialData, descriptor.initialDataSize);
			m_d3d12Resource->Unmap(0, nullptr);
		}
	}

	d3d12RenderDevice->SetD3D12ObjectName(m_d3d12Resource, descriptor.name);
}

void* CrHardwareGPUBufferD3D12::LockPS()
{
	void* data = nullptr;
	HRESULT hResult = m_d3d12Resource->Map(0, nullptr, &data);
	CrAssertMsg(hResult == S_OK, "Failed to map buffer");
	return data;
}

void CrHardwareGPUBufferD3D12::UnlockPS()
{
	m_d3d12Resource->Unmap(0, nullptr);
}