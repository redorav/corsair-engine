#pragma once

#include "Graphics/ICrGPUSynchronization.h"
#include <vulkan/vulkan.h>

namespace crgfx
{
	class IDevice;

	class GPUFenceVulkan final : public IGPUFence
	{
	public:

		GPUFenceVulkan(crgfx::IDevice* renderDevice, bool signaled);

		~GPUFenceVulkan();

		VkFence GetVkFence() const;

	private:

		VkFence m_vkFence;
	};

	inline VkFence GPUFenceVulkan::GetVkFence() const
	{
		return m_vkFence;
	}

	class GPUSemaphoreVulkan final : public IGPUSemaphore
	{
	public:

		GPUSemaphoreVulkan(crgfx::IDevice* renderDevice);

		~GPUSemaphoreVulkan();

		VkSemaphore GetVkSemaphore() const;

	private:

		VkSemaphore m_vkSemaphore;
	};

	inline VkSemaphore GPUSemaphoreVulkan::GetVkSemaphore() const
	{
		return m_vkSemaphore;
	}
};