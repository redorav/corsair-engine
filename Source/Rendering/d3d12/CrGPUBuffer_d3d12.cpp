#include "CrRendering_pch.h"

#include "CrGPUBuffer_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrD3D12.h"

#include "Core/Logging/ICrDebug.h"

CrHardwareGPUBufferD3D12::CrHardwareGPUBufferD3D12(CrRenderDeviceD3D12* renderDevice, const CrHardwareGPUBufferDescriptor& descriptor)
	: ICrHardwareGPUBuffer(descriptor)
{
	m_d3d12Device = renderDevice->GetD3D12Device();

	D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON;

	D3D12_HEAP_PROPERTIES heapProperties;
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

	switch (descriptor.access)
	{
		case cr3d::BufferAccess::CPURead:
			heapProperties.Type = D3D12_HEAP_TYPE_READBACK;
			break;
		case cr3d::BufferAccess::CPUWrite:
			heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
			initialResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
			break;
		default:
			break;
	}

	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	uint32_t bufferSize = descriptor.numElements * descriptor.stride;

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
	resourceDesc.Width = bufferSize;


	D3D12_CLEAR_VALUE optimizedClearValue = {};

	HRESULT hResult = S_OK;

	hResult = m_d3d12Device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE, 
		&resourceDesc,
		initialResourceState,
		nullptr,
		IID_PPV_ARGS(&m_d3dBufferResource));

	CrAssertMsg(hResult == S_OK, "Failed to create buffer");
}

void* CrHardwareGPUBufferD3D12::LockPS()
{
	void* data = nullptr;
	HRESULT hResult = m_d3dBufferResource->Map(0, nullptr, &data);
	CrAssertMsg(hResult == VK_SUCCESS, "Failed to map buffer");
	return data;
}

void CrHardwareGPUBufferD3D12::UnlockPS()
{
	m_d3dBufferResource->Unmap(0, nullptr);
}