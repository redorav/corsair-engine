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

	IDXGISwapChain1* m_d3d12Swapchain;
};
