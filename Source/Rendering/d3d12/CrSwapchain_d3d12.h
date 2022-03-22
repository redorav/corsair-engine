#pragma once

#include "Rendering/ICrSwapchain.h"
#include <d3d12.h>

class CrSwapchainD3D12 final : public ICrSwapchain
{
public:

	CrSwapchainD3D12(ICrRenderDevice* renderDevice, const CrSwapchainDescriptor& swapchainDescriptor);

	~CrSwapchainD3D12();

	virtual void PresentPS() override;

	virtual CrSwapchainResult AcquireNextImagePS(uint64_t timeoutNanoseconds = UINT64_MAX) override;

private:

	ID3D12Fence* m_d3d12Fence;

	HANDLE m_fenceEvent;

	CrVector<UINT64> m_fenceValues;

	IDXGISwapChain3* m_d3d12Swapchain;
};
