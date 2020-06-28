#pragma once

#include "CrGPUBuffer.h"
#include <vulkan/vulkan.h>

#include "Core/Logging/ICrDebug.h"

class CrRenderDeviceVulkan;

class CrHardwareGPUBufferVulkan final : public ICrHardwareGPUBuffer
{
public:

	CrHardwareGPUBufferVulkan(CrRenderDeviceVulkan* renderDevice, const CrGPUBufferCreateParams& params);

	static VkMemoryPropertyFlags GetVkMemoryPropertyFlags(cr3d::BufferAccess::T access);

	static VkBufferUsageFlags GetVkBufferUsageFlagBits(cr3d::BufferUsage::T usage, cr3d::BufferAccess::T access);

	VkIndexType GetVkIndexType() const;

	VkBuffer GetVkBuffer() const;

	VkDeviceMemory GetVkMemory() const;
	
	virtual void* LockPS() final override;

	virtual void UnlockPS() final override;

private:

	VkDevice m_vkDevice;

	VkBuffer m_vkBuffer;

	VkDeviceMemory m_vkMemory;

	VkIndexType m_vkIndexType : 1; // Only used for index buffers
};

inline VkIndexType CrHardwareGPUBufferVulkan::GetVkIndexType() const
{
	return m_vkIndexType;
}

inline VkBuffer CrHardwareGPUBufferVulkan::GetVkBuffer() const
{
	return m_vkBuffer;
}

inline VkDeviceMemory CrHardwareGPUBufferVulkan::GetVkMemory() const
{
	return m_vkMemory;
}