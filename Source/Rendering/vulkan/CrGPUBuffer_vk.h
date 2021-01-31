#pragma once

#include "Rendering/CrGPUBuffer.h"
#include <vulkan/vulkan.h>

#include "Core/Logging/ICrDebug.h"

class CrRenderDeviceVulkan;

class CrHardwareGPUBufferVulkan final : public ICrHardwareGPUBuffer
{
public:

	CrHardwareGPUBufferVulkan(CrRenderDeviceVulkan* renderDevice, const CrHardwareGPUBufferDescriptor& descriptor);

	virtual ~CrHardwareGPUBufferVulkan() override;

	static VkMemoryPropertyFlags GetVkMemoryPropertyFlags(cr3d::BufferAccess::T access);

	static VkBufferUsageFlags GetVkBufferUsageFlagBits(cr3d::BufferUsage::T usage, cr3d::BufferAccess::T access);

	VkIndexType GetVkIndexType() const;

	VkBuffer GetVkBuffer() const;

	VkBufferView GetVkBufferView() const;

	VkDeviceMemory GetVkMemory() const;
	
	virtual void* LockPS() override;

	virtual void UnlockPS() override;

private:

	VkDevice m_vkDevice;

	VkBuffer m_vkBuffer;

	VkBufferView m_vkBufferView = nullptr;

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

inline VkBufferView CrHardwareGPUBufferVulkan::GetVkBufferView() const
{
	return m_vkBufferView;
}

inline VkDeviceMemory CrHardwareGPUBufferVulkan::GetVkMemory() const
{
	return m_vkMemory;
}