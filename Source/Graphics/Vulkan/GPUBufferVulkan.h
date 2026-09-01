#pragma once

#include "Graphics/GPUBuffer.h"
#include <vulkan/vulkan.h>

#include "Core/Logging/ICrDebug.h"

struct CrVkBufferStateInfo
{
	VkAccessFlags accessMask = VK_ACCESS_FLAG_BITS_MAX_ENUM;
};

namespace crgfx
{
	class DeviceVulkan;

	class HardwareGPUBufferVulkan final : public IHardwareGPUBuffer
	{
	public:

		HardwareGPUBufferVulkan(crgfx::DeviceVulkan* renderDevice, const HardwareGPUBufferDescriptor& descriptor);

		virtual ~HardwareGPUBufferVulkan() override;

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

	inline VkBuffer HardwareGPUBufferVulkan::GetVkBuffer() const
	{
		return m_vkBuffer;
	}

	inline VkBufferView HardwareGPUBufferVulkan::GetVkBufferView() const
	{
		return m_vkBufferView;
	}
};