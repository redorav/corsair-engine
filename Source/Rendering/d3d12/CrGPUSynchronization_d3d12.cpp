#include "CrRendering_pch.h"

#include "CrGPUSynchronization_d3d12.h"
#include "CrRenderDevice_d3d12.h"

#include "Core/CrMacros.h"

CrGPUFenceD3D12::CrGPUFenceD3D12(ICrRenderDevice* renderDevice)
{
	ID3D12Device* d3d12Device = static_cast<CrRenderDeviceD3D12*>(renderDevice)->GetD3D12Device();
	d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_d3d12Fence));

	m_fenceEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
}

CrGPUFenceD3D12::~CrGPUFenceD3D12()
{
	m_d3d12Fence->Release();

	CloseHandle(m_fenceEvent);
}

CrGPUSemaphoreD3D12::CrGPUSemaphoreD3D12(ICrRenderDevice* renderDevice)
{
	ID3D12Device* d3d12Device = static_cast<CrRenderDeviceD3D12*>(renderDevice)->GetD3D12Device();
	d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_d3d12Fence));
}

CrGPUSemaphoreD3D12::~CrGPUSemaphoreD3D12()
{
	m_d3d12Fence->Release();
}
