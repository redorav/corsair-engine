#pragma once

#include "Graphics/CrGPUBuffer.h"
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

	static const CrVkBufferStateInfo& GetVkBufferStateInfo(crgfx::BufferState::T bufferState);

	static VkBufferUsageFlags GetVkBufferUsageFlagBits(crgfx::BufferUsage::T usage, crgfx::MemoryAccess::T access);

	static VkPipelineStageFlags GetVkPipelineStageFlags(crgfx::BufferState::T bufferState, crgfx::ShaderStageFlags::T shaderStages);

	VkBuffer GetVkBuffer() const;

	VkBufferView GetVkBufferView() const;

	virtual void* LockPS() override;

	virtual void UnlockPS() override;

private:

	VkBuffer m_vkBuffer;

	VkBufferView m_vkBufferView = nullptr;

	VmaAllocation m_vmaAllocation;
};

inline VkBuffer CrHardwareGPUBufferVulkan::GetVkBuffer() const
{
	return m_vkBuffer;
}

inline VkBufferView CrHardwareGPUBufferVulkan::GetVkBufferView() const
{
	return m_vkBufferView;
}