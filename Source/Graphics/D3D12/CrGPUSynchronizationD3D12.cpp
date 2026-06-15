#include "Graphics/CrRendering_pch.h"

#include "CrGPUSynchronizationD3D12.h"
#include "DeviceD3D12.h"

#include "Core/CrMacros.h"

namespace crgfx
{
	GPUFenceD3D12::GPUFenceD3D12(crgfx::IDevice* renderDevice, bool signaled) : IGPUFence(renderDevice)
	{
		crgfx::DeviceD3D12* d3d12RenderDevice = static_cast<crgfx::DeviceD3D12*>(renderDevice);
		ID3D12Device* d3d12Device = d3d12RenderDevice->GetD3D12Device();
		d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_d3d12Fence));

		m_fenceEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

		if (signaled)
		{
			d3d12RenderDevice->GetD3D12GraphicsCommandQueue()->Signal(m_d3d12Fence, 1);
		}
	}

	GPUFenceD3D12::~GPUFenceD3D12()
	{
		m_d3d12Fence->Release();

		CloseHandle(m_fenceEvent);
	}

	GPUSemaphoreD3D12::GPUSemaphoreD3D12(crgfx::IDevice* renderDevice) : IGPUSemaphore(renderDevice)
	{
		ID3D12Device* d3d12Device = static_cast<crgfx::DeviceD3D12*>(renderDevice)->GetD3D12Device();
		d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_d3d12Fence));
	}

	GPUSemaphoreD3D12::~GPUSemaphoreD3D12()
	{
		m_d3d12Fence->Release();
	}
};