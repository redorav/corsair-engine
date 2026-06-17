#pragma once

#include "Graphics/ISwapchain.h"
#include <vulkan/vulkan.h>

namespace crgfx
{
	class SwapchainVulkan final : public ISwapchain
	{
	public:

		SwapchainVulkan(crgfx::IDevice* renderDevice, const crgfx::SwapchainDescriptor& swapchainDescriptor);

		~SwapchainVulkan();

		VkSwapchainKHR GetVkSwapchain() const;

		virtual crgfx::CrSwapchainResult AcquireNextImagePS(uint64_t timeoutNanoseconds = UINT64_MAX) override;

		virtual void PresentPS() override;

		virtual void ResizePS(uint32_t width, uint32_t height) override;

	private:

		void CreateSwapchainTextures();

		// Semaphores are signaled when present completes
		crstl::vector<GPUSemaphoreHandle> m_presentCompleteSemaphores;

		crstl::vector<GPUFenceHandle> m_imageReadyFences;

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

	inline VkSwapchainKHR SwapchainVulkan::GetVkSwapchain() const
	{
		return m_vkSwapchain;
	}
};