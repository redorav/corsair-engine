#pragma once

#include "Rendering/ICrSwapchain.h"
#include <vulkan/vulkan.h>

class CrSwapchainVulkan final : public ICrSwapchain
{
public:

	CrSwapchainVulkan(ICrRenderDevice* renderDevice, const CrSwapchainDescriptor& swapchainDescriptor);

	~CrSwapchainVulkan();

	virtual void PresentPS() override;

	virtual CrSwapchainResult AcquireNextImagePS(const ICrGPUSemaphore* signalSemaphore, uint64_t timeoutNanoseconds = UINT64_MAX) override;

	VkSwapchainKHR GetVkSwapchain() const;

private:

	VkSurfaceKHR		m_vkSurface = nullptr;

	VkSwapchainKHR		m_vkSwapchain = nullptr;

	VkFormat			m_vkFormat = VK_FORMAT_UNDEFINED;

	VkColorSpaceKHR		m_vkColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
};

inline VkSwapchainKHR CrSwapchainVulkan::GetVkSwapchain() const
{
	return m_vkSwapchain;
}