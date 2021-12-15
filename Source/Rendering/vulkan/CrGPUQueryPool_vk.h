#pragma once

#include "Rendering/ICrGPUQueryPool.h"
#include <vulkan/vulkan.h>

class ICrRenderDevice;

class CrGPUQueryPoolVulkan final : public ICrGPUQueryPool
{
public:

	CrGPUQueryPoolVulkan(ICrRenderDevice* renderDevice, const CrGPUQueryPoolDescriptor& descriptor);

	~CrGPUQueryPoolVulkan();

	VkQueryPool GetVkQueryPool() const { return m_vkQueryPool; }

	const ICrHardwareGPUBuffer* GetResultsBuffer() const { return m_queryBuffer.get(); }

protected:

	virtual void GetTimingDataPS(CrGPUTiming* timingData, uint32_t timingCount) override;

	virtual void GetOcclusionDataPS(CrGPUOcclusion* occlusionData, uint32_t count) override;

	double m_timestampPeriod;

	// Use the platform-independent code so we don't have to rewrite it
	CrUniquePtr<ICrHardwareGPUBuffer> m_queryBuffer;

	VkQueryPool m_vkQueryPool;
};