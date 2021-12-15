#pragma once

#include "Rendering/ICrCommandBuffer.h"

#include "CrGPUBuffer_vk.h"

#include "CrPipeline_vk.h"

#include "CrGPUQueryPool_vk.h"

#include "CrVulkan.h"

#include "Rendering/CrRendering.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

#include "Rendering/CrCPUStackAllocator.h"

class CrTextureVulkan;
class CrShaderBindingLayoutVulkan;

class CrCommandBufferVulkan final : public ICrCommandBuffer
{
public:

	CrCommandBufferVulkan(ICrCommandQueue* commandQueue);

	virtual ~CrCommandBufferVulkan() override;

	const VkCommandBuffer& GetVkCommandBuffer() const;

	VkCommandBuffer& GetVkCommandBuffer();

private:

	virtual void BeginPS() override;

	virtual void EndPS() override;

	virtual void ClearRenderTargetPS(const ICrTexture* renderTarget, const float4& color, uint32_t level, uint32_t slice, uint32_t levelCount, uint32_t sliceCount) override;

	virtual void DrawPS(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;

	//void DrawIndirectPS(CrIndirectArgs* indirectArgs, uint32_t indirectArgsOffset) override;

	virtual void DrawIndexedPS(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) override;

	//void DrawIndexedIndirectPS(CrIndirectArgs* indirectArgs, uint32_t indirectArgsOffset) override;

	virtual void DispatchPS(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) override;

	//void DispatchIndirectPS(CrIndirectArgs* indirectArgs) override;

	virtual void BeginDebugEventPS(const char* eventName, const float4& color) override;

	virtual void EndDebugEventPS() override;

	virtual void InsertDebugMarkerPS(const char* markerName, const float4& color) override;

	virtual void BeginTimingQueryPS(const ICrGPUQueryPool* queryPool, CrGPUQueryId query) override;

	virtual void EndTimingQueryPS(const ICrGPUQueryPool* queryPool, CrGPUQueryId query) override;

	virtual void ResetGPUQueriesPS(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count) override;

	virtual void ResolveGPUQueriesPS(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count) override;

	virtual void FlushGraphicsRenderStatePS() override;

	virtual void FlushComputeRenderStatePS() override;

	virtual void BeginRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor) override;

	virtual void EndRenderPassPS() override;

	template<typename T, typename S>
	void FlushImageAndBufferBarriers(const T& buffers, const S& textures);

	void UpdateResourceTableVulkan(const CrShaderBindingLayoutVulkan& bindingTable, VkPipelineBindPoint vkPipelineBindPoint, VkPipelineLayout vkPipelineLayout);

	CrCPUStackAllocator m_renderPassAllocator;

	VkAllocationCallbacks m_renderPassAllocationCallbacks = {};

	CrVector<VkRenderPass> m_usedRenderPasses;

	CrVector<VkFramebuffer> m_usedFramebuffers;

	VkDevice m_vkDevice;

	VkCommandBuffer m_vkCommandBuffer;

	VkDescriptorPool m_vkDescriptorPool;
};

inline const VkCommandBuffer& CrCommandBufferVulkan::GetVkCommandBuffer() const
{
	return m_vkCommandBuffer;
}

inline VkCommandBuffer& CrCommandBufferVulkan::GetVkCommandBuffer()
{
	return m_vkCommandBuffer;
}

inline void CrCommandBufferVulkan::DrawPS(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	vkCmdDraw(m_vkCommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

inline void CrCommandBufferVulkan::DrawIndexedPS(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	vkCmdDrawIndexed(m_vkCommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

inline void CrCommandBufferVulkan::DispatchPS(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
{
	vkCmdDispatch(m_vkCommandBuffer, threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

inline void CrCommandBufferVulkan::BeginDebugEventPS(const char* eventName, const float4& color)
{
	if (vkCmdDebugMarkerBegin)
	{
		VkDebugMarkerMarkerInfoEXT markerInfo = {};
		markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
		store(color, markerInfo.color);
		markerInfo.pMarkerName = eventName;
		vkCmdDebugMarkerBegin(m_vkCommandBuffer, &markerInfo);
	}
}

inline void CrCommandBufferVulkan::EndDebugEventPS()
{
	if (vkCmdDebugMarkerEnd)
	{
		vkCmdDebugMarkerEnd(m_vkCommandBuffer);
	}
}

inline void CrCommandBufferVulkan::InsertDebugMarkerPS(const char* markerName, const float4& color)
{
	if (vkCmdDebugMarkerInsert)
	{
		VkDebugMarkerMarkerInfoEXT markerInfo = {};
		markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
		store(color, markerInfo.color);
		markerInfo.pMarkerName = markerName;
		vkCmdDebugMarkerInsert(m_vkCommandBuffer, &markerInfo);
	}
}

// Vulkan exposes timestamp queries, but a timestamp by itself isn't very useful unless compared
// to some other timestamp. Therefore we create this fictional begin/end pair for timestamps.
// It's useful to do it anyway because we need to specify the stage, and that is implicit in the
// Begin and End. Note that we assume query ids are consecutive, as that's what a pool will do.
inline void CrCommandBufferVulkan::BeginTimingQueryPS(const ICrGPUQueryPool* queryPool, CrGPUQueryId query)
{
	const CrGPUQueryPoolVulkan* vulkanQueryPool = static_cast<const CrGPUQueryPoolVulkan*>(queryPool);
	vkCmdWriteTimestamp(m_vkCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, vulkanQueryPool->GetVkQueryPool(), 2 * query.id);
}

inline void CrCommandBufferVulkan::EndTimingQueryPS(const ICrGPUQueryPool* queryPool, CrGPUQueryId query)
{
	const CrGPUQueryPoolVulkan* vulkanQueryPool = static_cast<const CrGPUQueryPoolVulkan*>(queryPool);
	vkCmdWriteTimestamp(m_vkCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, vulkanQueryPool->GetVkQueryPool(), 2 * query.id + 1);
}

inline void CrCommandBufferVulkan::ResetGPUQueriesPS(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count)
{
	const CrGPUQueryPoolVulkan* vulkanQueryPool = static_cast<const CrGPUQueryPoolVulkan*>(queryPool);

	if (queryPool->GetType() == cr3d::QueryType::Timing)
	{
		start = 2 * start;
		count = 2 * count;
	}

	vkCmdResetQueryPool(m_vkCommandBuffer, vulkanQueryPool->GetVkQueryPool(), start, count);
}

inline void CrCommandBufferVulkan::ResolveGPUQueriesPS(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count)
{
	const CrGPUQueryPoolVulkan* vulkanQueryPool = static_cast<const CrGPUQueryPoolVulkan*>(queryPool);
	const CrHardwareGPUBufferVulkan* vulkanGPUBuffer = static_cast<const CrHardwareGPUBufferVulkan*>(vulkanQueryPool->GetResultsBuffer());

	if (queryPool->GetType() == cr3d::QueryType::Timing)
	{
		start = 2 * start;
		count = 2 * count;
	}

	// The wait flag here doesn't wait on the CPU, rather the GPU will ensure query results are all ready before resolving. In practice this means
	// it won't try to reorder the copy on the GPU to a point before the query was actually finished
	vkCmdCopyQueryPoolResults(m_vkCommandBuffer, vulkanQueryPool->GetVkQueryPool(), start, count, vulkanGPUBuffer->GetVkBuffer(), 0, vulkanQueryPool->GetQuerySize(), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
}