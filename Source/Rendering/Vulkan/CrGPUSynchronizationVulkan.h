#pragma once

#include "Rendering/ICrGPUSynchronization.h"
#include <vulkan/vulkan.h>

class ICrRenderDevice;

class CrGPUFenceVulkan final : public ICrGPUFence
{
public:

	CrGPUFenceVulkan(ICrRenderDevice* renderDevice, bool signaled);

	~CrGPUFenceVulkan();

	VkFence GetVkFence() const;

private:

	VkFence m_vkFence;
};

inline VkFence CrGPUFenceVulkan::GetVkFence() const
{
	return m_vkFence;
}

class CrGPUSemaphoreVulkan final : public ICrGPUSemaphore
{
public:

	CrGPUSemaphoreVulkan(ICrRenderDevice* renderDevice);

	~CrGPUSemaphoreVulkan();

	VkSemaphore GetVkSemaphore() const;

private:

	VkSemaphore m_vkSemaphore;
};

inline VkSemaphore CrGPUSemaphoreVulkan::GetVkSemaphore() const
{
	return m_vkSemaphore;
}