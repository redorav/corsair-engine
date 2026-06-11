#pragma once

#include "Rendering/ICrSwapchain.h"

#include "d3d12.h"

class CrSwapchainD3D12 final : public ICrSwapchain
{
public:

	CrSwapchainD3D12(ICrRenderDevice* renderDevice, const CrSwapchainDescriptor& swapchainDescriptor);

	~CrSwapchainD3D12();

	virtual CrSwapchainResult AcquireNextImagePS(uint64_t timeoutNanoseconds = UINT64_MAX) override;

	virtual void PresentPS() override;

	virtual void ResizePS(uint32_t width, uint32_t height) override;

private:

	void CreateSwapchainTextures();

	ID3D12Fence* m_d3d12Fence;

	HANDLE m_fenceEvent;

	crstl::vector<UINT64> m_fenceValues;

	DXGI_SWAP_CHAIN_FLAG m_d3d12SwapchainFlags;

	IDXGISwapChain3* m_d3d12Swapchain;
};
