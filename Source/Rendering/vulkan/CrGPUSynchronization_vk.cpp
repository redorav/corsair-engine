#include "Rendering/CrRendering_pch.h"

#include "CrGPUSynchronization_vk.h"
#include "CrRenderDevice_vk.h"

CrGPUFenceVulkan::CrGPUFenceVulkan(ICrRenderDevice* renderDevice)
{
	m_vkDevice = static_cast<CrRenderDeviceVulkan*>(renderDevice)->GetVkDevice();

	VkFenceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	info.flags = 0;

	vkCreateFence(m_vkDevice, &info, nullptr, &m_vkFence);
}

CrGPUFenceVulkan::~CrGPUFenceVulkan()
{
	vkDestroyFence(m_vkDevice, m_vkFence, nullptr);
}

CrGPUSemaphoreVulkan::CrGPUSemaphoreVulkan(ICrRenderDevice* renderDevice)
{
	m_vkDevice = static_cast<CrRenderDeviceVulkan*>(renderDevice)->GetVkDevice();

	VkSemaphoreCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	vkCreateSemaphore(m_vkDevice, &info, nullptr, &m_vkSemaphore);
}

CrGPUSemaphoreVulkan::~CrGPUSemaphoreVulkan()
{
	vkDestroySemaphore(m_vkDevice, m_vkSemaphore, nullptr);
}
