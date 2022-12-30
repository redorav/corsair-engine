#include "Rendering/CrRendering_pch.h"

#include "CrGPUSynchronization_vk.h"
#include "CrRenderDevice_vk.h"

CrGPUFenceVulkan::CrGPUFenceVulkan(ICrRenderDevice* renderDevice) : ICrGPUFence(renderDevice)
{
	VkDevice vkDevice = static_cast<CrRenderDeviceVulkan*>(renderDevice)->GetVkDevice();

	VkFenceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	info.flags = 0;

	vkCreateFence(vkDevice, &info, nullptr, &m_vkFence);
}

CrGPUFenceVulkan::~CrGPUFenceVulkan()
{
	vkDestroyFence(static_cast<CrRenderDeviceVulkan*>(m_renderDevice)->GetVkDevice(), m_vkFence, nullptr);
}

CrGPUSemaphoreVulkan::CrGPUSemaphoreVulkan(ICrRenderDevice* renderDevice) : ICrGPUSemaphore(renderDevice)
{
	VkDevice vkDevice = static_cast<CrRenderDeviceVulkan*>(renderDevice)->GetVkDevice();

	VkSemaphoreCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	vkCreateSemaphore(vkDevice, &info, nullptr, &m_vkSemaphore);
}

CrGPUSemaphoreVulkan::~CrGPUSemaphoreVulkan()
{
	vkDestroySemaphore(static_cast<CrRenderDeviceVulkan*>(m_renderDevice)->GetVkDevice(), m_vkSemaphore, nullptr);
}
