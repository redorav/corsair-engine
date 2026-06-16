#pragma once

#include "Graphics/ICrGPUSynchronization.h"
#include "d3d12.h"

namespace crgfx
{
	class IDevice;

	class GPUFenceD3D12 final : public IGPUFence
	{
	public:

		GPUFenceD3D12(crgfx::IDevice* renderDevice, bool signaled);

		~GPUFenceD3D12();

		ID3D12Fence* GetD3D12Fence() const { return m_d3d12Fence; }

		HANDLE GetFenceEvent() const { return m_fenceEvent; }

	private:

		HANDLE m_fenceEvent;

		ID3D12Fence* m_d3d12Fence;
	};

	class GPUSemaphoreD3D12 final : public IGPUSemaphore
	{
	public:

		GPUSemaphoreD3D12(crgfx::IDevice* renderDevice);

		~GPUSemaphoreD3D12();

	private:

		ID3D12Fence* m_d3d12Fence;
	};
};