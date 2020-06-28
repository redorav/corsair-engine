#pragma once

#include "ICrSwapchain.h"
#include <vulkan/vulkan.h>

class CrSwapchainVulkan final : public ICrSwapchain
{
public:

	CrSwapchainVulkan(ICrRenderDevice* renderDevice, uint32_t requestedWidth, uint32_t requestedHeight);

	~CrSwapchainVulkan();

	virtual void CreatePS(ICrRenderDevice* renderDevice, uint32_t requestedWidth, uint32_t requestedHeight) final override;

	virtual void PresentPS(ICrCommandQueue* queue, const ICrGPUSemaphore* waitSemaphore) final override;

	virtual CrSwapchainResult AcquireNextImagePS(const ICrGPUSemaphore* signalSemaphore, uint64_t timeoutNanoseconds = UINT64_MAX) final override;

	VkSwapchainKHR GetVkSwapchain();

private:

	VkDevice			m_vkDevice;

	VkSwapchainKHR		m_vkSwapChain;

	VkFormat			m_vkFormat;

	VkColorSpaceKHR		m_vkColorSpace;
};
