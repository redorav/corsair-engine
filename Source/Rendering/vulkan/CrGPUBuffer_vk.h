#pragma once

#include "Rendering/CrGPUBuffer.h"
#include <vulkan/vulkan.h>

#include "Core/Logging/ICrDebug.h"

struct CrVkBufferStateInfo
{
	VkAccessFlags accessMask = VK_ACCESS_FLAG_BITS_MAX_ENUM;
};

class CrRenderDeviceVulkan;

class CrHardwareGPUBufferVulkan final : public ICrHardwareGPUBuffer
{
public:

	CrHardwareGPUBufferVulkan(CrRenderDeviceVulkan* renderDevice, const CrHardwareGPUBufferDescriptor& descriptor);

	virtual ~CrHardwareGPUBufferVulkan() override;

	static const CrVkBufferStateInfo& GetVkBufferStateInfo(cr3d::BufferState::T bufferState);

	static VkBufferUsageFlags GetVkBufferUsageFlagBits(cr3d::BufferUsage::T usage, cr3d::MemoryAccess::T access);

	VkIndexType GetVkIndexType() const;

	VkBuffer GetVkBuffer() const;

	VkBufferView GetVkBufferView() const;

	virtual void* LockPS() override;

	virtual void UnlockPS() override;

private:

	VkBuffer m_vkBuffer;

	VkBufferView m_vkBufferView = nullptr;

	VmaAllocation m_vmaAllocation;

	VkIndexType m_vkIndexType; // Only used for index buffers
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