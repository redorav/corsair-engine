#pragma once

#include "Graphics/IGPUQueryPool.h"
#include <vulkan/vulkan.h>

namespace crgfx
{
	class IDevice;

	class GPUQueryPoolVulkan final : public IGPUQueryPool
	{
	public:

		GPUQueryPoolVulkan(crgfx::IDevice* renderDevice, const GPUQueryPoolDescriptor& descriptor);

		~GPUQueryPoolVulkan();

		VkQueryPool GetVkQueryPool() const { return m_vkQueryPool; }

		const IHardwareGPUBuffer* GetResultsBuffer() const { return m_queryBuffer.get(); }

	protected:

		virtual void GetTimingDataPS(GPUTimestamp* timingData, uint32_t timingCount) override;

		virtual void GetOcclusionDataPS(GPUOcclusion* occlusionData, uint32_t count) override;

		// Use the platform-independent code so we don't have to rewrite it
		HardwareGPUBufferHandle m_queryBuffer;

		VkQueryPool m_vkQueryPool;
	};
};