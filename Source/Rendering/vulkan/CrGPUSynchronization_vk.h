#pragma once

#include "ICrGPUSynchronization.h"
#include <vulkan/vulkan.h>

class ICrRenderDevice;

// Fences do CPU-GPU synchronization
class CrGPUFenceVulkan final : public ICrGPUFence
{
public:

	CrGPUFenceVulkan(ICrRenderDevice* renderDevice);

	~CrGPUFenceVulkan();

	const VkFence& GetVkFence() const;

private:

	VkDevice m_vkDevice;

	VkFence m_vkFence;
};

inline const VkFence& CrGPUFenceVulkan::GetVkFence() const
{
	return m_vkFence;
}

// Semaphores do inter-queue synchronization
class CrGPUSemaphoreVulkan final : public ICrGPUSemaphore
{
public:

	CrGPUSemaphoreVulkan(ICrRenderDevice* renderDevice);

	~CrGPUSemaphoreVulkan();

	const VkSemaphore& GetVkSemaphore() const;

private:

	VkDevice m_vkDevice;

	VkSemaphore m_vkSemaphore;
};

inline const VkSemaphore& CrGPUSemaphoreVulkan::GetVkSemaphore() const
{
	return m_vkSemaphore;
}

// TODO Create events for fine-grained command buffer sync