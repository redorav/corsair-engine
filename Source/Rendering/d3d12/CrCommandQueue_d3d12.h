#pragma once

#include "ICrCommandQueue.h"
#include <d3d12.h>

class ICrGPUSemaphore;
class ICrGPUFence;
class ICrRenderDevice;

class CrCommandQueueD3D12 final : public ICrCommandQueue
{
	friend class CrRenderDeviceD3D12;

public:

	CrCommandQueueD3D12(ICrRenderDevice* renderDevice, CrCommandQueueType::T type);

	virtual ICrCommandBuffer* CreateCommandBufferPS() override;

	virtual void SubmitCommandBufferPS(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence) override;

	virtual void WaitIdlePS() override;

private:

};