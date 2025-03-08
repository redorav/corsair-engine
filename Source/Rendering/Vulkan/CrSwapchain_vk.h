#pragma once

#include "Rendering/ICrSwapchain.h"
#include <vulkan/vulkan.h>

class CrSwapchainVulkan final : public ICrSwapchain
{
public:

	CrSwapchainVulkan(ICrRenderDevice* renderDevice, const CrSwapchainDescriptor& swapchainDescriptor);

	~CrSwapchainVulkan();

	VkSwapchainKHR GetVkSwapchain() const;

	virtual CrSwapchainResult AcquireNextImagePS(uint64_t timeoutNanoseconds = UINT64_MAX) override;

	virtual void PresentPS() override;

	virtual void ResizePS(uint32_t width, uint32_t height) override;

private:

	void CreateSwapchainTextures();

	// Semaphores are signaled when present completes
	crstl::vector<CrGPUSemaphoreHandle> m_presentCompleteSemaphores;

	crstl::vector<CrGPUFenceHandle> m_imageReadyFences;

	// We need to have another index for the semaphore because we don't really know
	// the buffer index until we have acquired the image, but we need to signal the
	// semaphore during that call
	uint32_t m_currentSemaphoreIndex;

	VkSurfaceKHR		m_vkSurface = nullptr;

	VkSwapchainKHR		m_vkSwapchain = nullptr;

	VkFormat			m_vkFormat = VK_FORMAT_UNDEFINED;

	VkColorSpaceKHR		m_vkColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	VkFence				m_swapchainRecreationFence = nullptr;

	bool				m_acquired = false;

	// We use this mainly to resize the swapchain. There's a couple of things
	// we can modify but the rest stays the same
	VkSwapchainCreateInfoKHR m_vkSwapchainCreateInfo;
};

inline VkSwapchainKHR CrSwapchainVulkan::GetVkSwapchain() const
{
	return m_vkSwapchain;
}