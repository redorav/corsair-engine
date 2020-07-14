#pragma once

#include "ICrCommandQueue.h"
#include <vulkan/vulkan.h>

class ICrGPUSemaphore;
class ICrGPUFence;
class ICrRenderDevice;

class CrCommandQueueVulkan final : public ICrCommandQueue
{
	friend class CrRenderDeviceVulkan;

public:

	CrCommandQueueVulkan(ICrRenderDevice* renderDevice, CrCommandQueueType::T type);

	virtual ICrCommandBuffer* CreateCommandBufferPS() final override;

	virtual void SubmitCommandBufferPS(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence) final override;

	virtual void WaitIdlePS() final override;

	VkDevice GetVkDevice() const;

	VkQueue GetVkQueue() const;

	VkCommandPool GetVkCommandBufferPool() const;

private:

	VkDevice m_vkDevice;

	VkQueue m_vkQueue;

	VkCommandPool m_vkCommandBufferPool;

	uint32_t m_queueNodeIndex;
};