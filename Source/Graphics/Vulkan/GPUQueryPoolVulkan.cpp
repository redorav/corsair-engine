#include "Graphics/CrRendering_pch.h"

#include "DeviceVulkan.h"
#include "GPUQueryPoolVulkan.h"
#include "GPUBufferVulkan.h"

#include "Core/Logging/ICrDebug.h"

VkQueryType GetVkQueryType(crgfx::QueryType queryType)
{
	switch (queryType)
	{
		case crgfx::QueryType::Timestamp: return VK_QUERY_TYPE_TIMESTAMP;
		case crgfx::QueryType::Occlusion: return VK_QUERY_TYPE_OCCLUSION;
		default: return VK_QUERY_TYPE_MAX_ENUM;
	}
}

namespace crgfx
{
	CrGPUQueryPoolVulkan::CrGPUQueryPoolVulkan(crgfx::IDevice* renderDevice, const GPUQueryPoolDescriptor& descriptor) : IGPUQueryPool(renderDevice, descriptor)
	{
		crgfx::DeviceVulkan* vulkanRenderDevice = static_cast<crgfx::DeviceVulkan*>(renderDevice);

		m_querySize = sizeof(uint64_t);

		VkQueryPoolCreateInfo poolCreateInfo = {};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		poolCreateInfo.flags = 0;
		poolCreateInfo.queryType = GetVkQueryType(descriptor.type);
		poolCreateInfo.queryCount = descriptor.count;

		VkResult vkResult = VK_SUCCESS;

		vkResult = vkCreateQueryPool(vulkanRenderDevice->GetVkDevice(), &poolCreateInfo, nullptr, &m_vkQueryPool);
		CrAssertMsg(vkResult == VK_SUCCESS, "Failed to create query pool");

		HardwareGPUBufferDescriptor queryBufferDescriptor
		(
			crgfx::BufferUsage::TransferDst,
			crgfx::MemoryAccess::GPUWriteCPURead,
			descriptor.count,
			m_querySize
		);

		m_queryBuffer = vulkanRenderDevice->CreateHardwareGPUBuffer(queryBufferDescriptor);

		m_timestampPeriod = vulkanRenderDevice->GetVkPhysicalDeviceProperties().limits.timestampPeriod;
	}

	CrGPUQueryPoolVulkan::~CrGPUQueryPoolVulkan()
	{
		crgfx::DeviceVulkan* vulkanRenderDevice = static_cast<crgfx::DeviceVulkan*>(m_renderDevice);
		vkDestroyQueryPool(vulkanRenderDevice->GetVkDevice(), m_vkQueryPool, nullptr);
	}

	void CrGPUQueryPoolVulkan::GetTimingDataPS(GPUTimestamp* timingData, uint32_t timingCount)
	{
		uint64_t* memory = (uint64_t*)m_queryBuffer->Lock();
		{
			memcpy(timingData, memory, timingCount * sizeof(timingData));
		}
		m_queryBuffer->Unlock();
	}

	void CrGPUQueryPoolVulkan::GetOcclusionDataPS(GPUOcclusion* occlusionData, uint32_t occlusionCount)
	{
		uint64_t* memory = (uint64_t*)m_queryBuffer->Lock();
		{
			for (uint32_t i = 0; i < occlusionCount; i++)
			{
				occlusionData[i].visibilitySamples = memory[i];
			}
		}
		m_queryBuffer->Unlock();
	}
};