#pragma once

#include "ICrGPUStackAllocator.h"

typedef struct VkBuffer_T* VkBuffer;
typedef struct VkDeviceMemory_T* VkDeviceMemory;

class CrGPUStackAllocatorVulkan final : public ICrGPUStackAllocator
{
public:

	CrGPUStackAllocatorVulkan(ICrRenderDevice* renderDevice);

	~CrGPUStackAllocatorVulkan();

private:

	virtual void InitPS(size_t size) final override;

	virtual void* BeginPS() final override;

	virtual void EndPS() final override;

	VkDevice m_vkDevice = 0;

	VkBuffer m_vkBuffer = 0;

	VkDeviceMemory m_vkDeviceMemory = 0;
};
