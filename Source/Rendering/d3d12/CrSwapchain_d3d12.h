#pragma once

#include "ICrSwapchain.h"
#include <d3d12.h>

class CrSwapchainD3D12 final : public ICrSwapchain
{
public:

	CrSwapchainD3D12(ICrRenderDevice* renderDevice, const CrSwapchainDescriptor& swapchainDescriptor);

	~CrSwapchainD3D12();

	virtual void CreatePS(ICrRenderDevice* renderDevice, const CrSwapchainDescriptor& swapchainDescriptor) final override;

	virtual void PresentPS(ICrCommandQueue* queue, const ICrGPUSemaphore* waitSemaphore) final override;

	virtual CrSwapchainResult AcquireNextImagePS(const ICrGPUSemaphore* signalSemaphore, uint64_t timeoutNanoseconds = UINT64_MAX) final override;

private:


};
