#pragma once

#include "Graphics/ICrGPUQueryPool.h"
#include <vulkan/vulkan.h>

namespace crgfx
{
	class IDevice;

	class CrGPUQueryPoolVulkan final : public IGPUQueryPool
	{
	public:

		CrGPUQueryPoolVulkan(crgfx::IDevice* renderDevice, const GPUQueryPoolDescriptor& descriptor);

		~CrGPUQueryPoolVulkan();

		VkQueryPool GetVkQueryPool() const { return m_vkQueryPool; }

		const ICrHardwareGPUBuffer* GetResultsBuffer() const { return m_queryBuffer.get(); }

	protected:

		virtual void GetTimingDataPS(GPUTimestamp* timingData, uint32_t timingCount) override;

		virtual void GetOcclusionDataPS(GPUOcclusion* occlusionData, uint32_t count) override;

		// Use the platform-independent code so we don't have to rewrite it
		CrHardwareGPUBufferHandle m_queryBuffer;

		VkQueryPool m_vkQueryPool;
	};
};