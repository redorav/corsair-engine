#include "Rendering/CrRendering_pch.h"

#include "CrRenderDevice_vk.h"
#include "CrGPUQueryPool_vk.h"
#include "CrGPUBuffer_vk.h"

#include "Core/Logging/ICrDebug.h"

VkQueryType GetVkQueryType(cr3d::QueryType queryType)
{
	switch (queryType)
	{
		case cr3d::QueryType::Timestamp: return VK_QUERY_TYPE_TIMESTAMP;
		case cr3d::QueryType::Occlusion: return VK_QUERY_TYPE_OCCLUSION;
		default: return VK_QUERY_TYPE_MAX_ENUM;
	}
}

CrGPUQueryPoolVulkan::CrGPUQueryPoolVulkan(ICrRenderDevice* renderDevice, const CrGPUQueryPoolDescriptor& descriptor) : ICrGPUQueryPool(renderDevice, descriptor)
{
	CrRenderDeviceVulkan* vulkanRenderDevice = static_cast<CrRenderDeviceVulkan*>(renderDevice);

	m_querySize = sizeof(uint64_t);

	VkQueryPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	poolCreateInfo.flags = 0;
	poolCreateInfo.queryType = GetVkQueryType(descriptor.type);
	poolCreateInfo.queryCount = descriptor.count;

	VkResult vkResult = VK_SUCCESS;

	vkResult = vkCreateQueryPool(vulkanRenderDevice->GetVkDevice(), &poolCreateInfo, nullptr, &m_vkQueryPool);
	CrAssertMsg(vkResult == VK_SUCCESS, "Failed to create query pool");

	CrHardwareGPUBufferDescriptor queryBufferDescriptor
	(
		cr3d::BufferUsage::TransferDst,
		cr3d::MemoryAccess::GPUWriteCPURead,
		descriptor.count,
		m_querySize
	);

	m_queryBuffer = vulkanRenderDevice->CreateHardwareGPUBuffer(queryBufferDescriptor);

	m_timestampPeriod = vulkanRenderDevice->GetVkPhysicalDeviceProperties().limits.timestampPeriod;
}

CrGPUQueryPoolVulkan::~CrGPUQueryPoolVulkan()
{
	CrRenderDeviceVulkan* vulkanRenderDevice = static_cast<CrRenderDeviceVulkan*>(m_renderDevice);
	vkDestroyQueryPool(vulkanRenderDevice->GetVkDevice(), m_vkQueryPool, nullptr);
}

void CrGPUQueryPoolVulkan::GetTimingDataPS(CrGPUTimestamp* timingData, uint32_t timingCount)
{
	uint64_t* memory = (uint64_t*)m_queryBuffer->Lock();
	{
		memcpy(timingData, memory, timingCount * sizeof(timingData));
	}
	m_queryBuffer->Unlock();
}

void CrGPUQueryPoolVulkan::GetOcclusionDataPS(CrGPUOcclusion* occlusionData, uint32_t occlusionCount)
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
