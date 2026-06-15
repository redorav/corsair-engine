#pragma once

#include "Graphics/ICrGPUSynchronization.h"
#include "d3d12.h"

namespace crgfx
{
	class IDevice;

	class CrGPUFenceD3D12 final : public ICrGPUFence
	{
	public:

		CrGPUFenceD3D12(crgfx::IDevice* renderDevice, bool signaled);

		~CrGPUFenceD3D12();

		ID3D12Fence* GetD3D12Fence() const { return m_d3d12Fence; }

		HANDLE GetFenceEvent() const { return m_fenceEvent; }

	private:

		HANDLE m_fenceEvent;

		ID3D12Fence* m_d3d12Fence;
	};

	class CrGPUSemaphoreD3D12 final : public ICrGPUSemaphore
	{
	public:

		CrGPUSemaphoreD3D12(crgfx::IDevice* renderDevice);

		~CrGPUSemaphoreD3D12();

	private:

		ID3D12Fence* m_d3d12Fence;
	};
};