#pragma once

#include "Rendering/ICrCommandQueue.h"
#include <vulkan/vulkan.h>

class ICrGPUSemaphore;
class ICrGPUFence;
class ICrRenderDevice;

class CrCommandQueueVulkan final : public ICrCommandQueue
{
public:

	CrCommandQueueVulkan(ICrRenderDevice* renderDevice, CrCommandQueueType::T type);

	virtual ICrCommandBuffer* CreateCommandBufferPS() override;

	virtual void SubmitCommandBufferPS(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence) override;

	virtual void WaitIdlePS() override;

	VkDevice GetVkDevice() const;

	VkQueue GetVkQueue() const;

	VkCommandPool GetVkCommandBufferPool() const;

private:

	VkDevice m_vkDevice;

	VkQueue m_vkQueue;

	VkCommandPool m_vkCommandBufferPool;

	uint32_t m_queueNodeIndex;
};