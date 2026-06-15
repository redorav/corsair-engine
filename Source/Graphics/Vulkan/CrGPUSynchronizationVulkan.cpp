#include "Graphics/CrRendering_pch.h"

#include "CrGPUSynchronizationVulkan.h"
#include "DeviceVulkan.h"

namespace crgfx
{
	GPUFenceVulkan::GPUFenceVulkan(crgfx::IDevice* renderDevice, bool signaled) : IGPUFence(renderDevice)
	{
		VkDevice vkDevice = static_cast<crgfx::DeviceVulkan*>(renderDevice)->GetVkDevice();

		VkFenceCreateInfo vkFenceCreateInfo {};
		vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		vkFenceCreateInfo.flags = (signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0);

		vkCreateFence(vkDevice, &vkFenceCreateInfo, nullptr, &m_vkFence);
	}

	GPUFenceVulkan::~GPUFenceVulkan()
	{
		vkDestroyFence(static_cast<crgfx::DeviceVulkan*>(m_renderDevice)->GetVkDevice(), m_vkFence, nullptr);
	}

	GPUSemaphoreVulkan::GPUSemaphoreVulkan(crgfx::IDevice* renderDevice) : IGPUSemaphore(renderDevice)
	{
		VkDevice vkDevice = static_cast<crgfx::DeviceVulkan*>(renderDevice)->GetVkDevice();

		VkSemaphoreCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		vkCreateSemaphore(vkDevice, &info, nullptr, &m_vkSemaphore);
	}

	GPUSemaphoreVulkan::~GPUSemaphoreVulkan()
	{
		vkDestroySemaphore(static_cast<crgfx::DeviceVulkan*>(m_renderDevice)->GetVkDevice(), m_vkSemaphore, nullptr);
	}
};