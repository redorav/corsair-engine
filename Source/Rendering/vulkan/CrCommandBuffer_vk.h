#pragma once

#include "Rendering/ICrCommandBuffer.h"

#include "CrPipeline_vk.h"

#include "CrGPUQueryPool_vk.h"

#include "CrVulkan.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

#include "Rendering/CrCPUStackAllocator.h"

class CrTextureVulkan;
class ICrShaderBindingLayout;

class CrCommandBufferVulkan final : public ICrCommandBuffer
{
public:

	CrCommandBufferVulkan(CrRenderDeviceVulkan* renderDevice, const CrCommandBufferDescriptor& descriptor);

	virtual ~CrCommandBufferVulkan() override;

	const VkCommandBuffer& GetVkCommandBuffer() const;

	VkCommandBuffer& GetVkCommandBuffer();

private:

	virtual void BeginPS() override;

	virtual void EndPS() override;

	virtual void ClearRenderTargetPS(const ICrTexture* renderTarget, const float4& color, uint32_t level, uint32_t slice, uint32_t levelCount, uint32_t sliceCount) override;

	virtual void DrawPS(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;

	virtual void DrawIndexedPS(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) override;

	virtual void DispatchPS(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) override;

	virtual void DrawIndirectPS(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count) override;

	virtual void DrawIndexedIndirectPS(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count) override;

	virtual void DispatchIndirectPS(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset) override;

	virtual void BeginDebugEventPS(const char* eventName, const float4& color) override;

	virtual void EndDebugEventPS() override;

	virtual void InsertDebugMarkerPS(const char* markerName, const float4& color) override;

	virtual void BeginTimestampQueryPS(const ICrGPUQueryPool* queryPool, CrGPUQueryId query) override;

	virtual void EndTimestampQueryPS(const ICrGPUQueryPool* queryPool, CrGPUQueryId query) override;

	virtual void ResetGPUQueriesPS(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count) override;

	virtual void ResolveGPUQueriesPS(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count) override;

	virtual void FlushGraphicsRenderStatePS() override;

	virtual void FlushComputeRenderStatePS() override;

	virtual void BeginRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor) override;

	virtual void EndRenderPassPS() override;

	void GatherImageAndBufferBarriers(const CrRenderPassDescriptor::BufferTransitionVector& buffers, const CrRenderPassDescriptor::TextureTransitionVector& textures);

	void QueueVkImageBarrier(const ICrTexture* texture, uint32_t mipmapStart, uint32_t mipmapCount, uint32_t sliceStart, uint32_t sliceCount, 
		const cr3d::TextureState& sourceState, const cr3d::TextureState& destinationState);

	void FlushImageAndBufferBarriers();

	void UpdateResourceTableVulkan(const ICrShaderBindingLayout& bindingLayout, VkPipelineBindPoint vkPipelineBindPoint, VkDescriptorSetLayout vkDescriptorSetLayout, VkPipelineLayout vkPipelineLayout);

	CrCPUStackAllocator m_renderPassAllocator;

	VkAllocationCallbacks m_renderPassAllocationCallbacks = {};

	CrVector<VkRenderPass> m_usedRenderPasses;

	CrVector<VkFramebuffer> m_usedFramebuffers;

	VkCommandBuffer m_vkCommandBuffer;

	VkDescriptorPool m_vkDescriptorPool;

	// Barrier processing

	VkPipelineStageFlags m_srcStageMask = VK_PIPELINE_STAGE_NONE; // srcStageMask is an OR of all pipeline barrier stage masks

	VkPipelineStageFlags m_destStageMask = VK_PIPELINE_STAGE_NONE; // destStageMask is an OR of all pipeline barrier stage masks

	CrFixedVector<VkBufferMemoryBarrier, CrRenderPassDescriptor::MaxTransitionCount> m_bufferMemoryBarriers;

	CrFixedVector<VkImageMemoryBarrier, CrRenderPassDescriptor::MaxTransitionCount> m_imageMemoryBarriers;
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
	if (vkCmdBeginDebugUtilsLabel)
	{
		VkDebugUtilsLabelEXT debugUtilsLabel = {};
		debugUtilsLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		store(color, debugUtilsLabel.color);
		debugUtilsLabel.pLabelName = eventName;
		vkCmdBeginDebugUtilsLabel(m_vkCommandBuffer, &debugUtilsLabel);
	}
}

inline void CrCommandBufferVulkan::EndDebugEventPS()
{
	if (vkCmdEndDebugUtilsLabel)
	{
		vkCmdEndDebugUtilsLabel(m_vkCommandBuffer);
	}
}

inline void CrCommandBufferVulkan::InsertDebugMarkerPS(const char* markerName, const float4& color)
{
	if (vkCmdInsertDebugUtilsLabel)
	{
		VkDebugUtilsLabelEXT debugUtilsLabel = {};
		debugUtilsLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		store(color, debugUtilsLabel.color);
		debugUtilsLabel.pLabelName = markerName;
		vkCmdInsertDebugUtilsLabel(m_vkCommandBuffer, &debugUtilsLabel);
	}
}

inline void CrCommandBufferVulkan::BeginTimestampQueryPS(const ICrGPUQueryPool* queryPool, CrGPUQueryId query)
{
	const CrGPUQueryPoolVulkan* vulkanQueryPool = static_cast<const CrGPUQueryPoolVulkan*>(queryPool);
	vkCmdWriteTimestamp(m_vkCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, vulkanQueryPool->GetVkQueryPool(), query.id);
}

inline void CrCommandBufferVulkan::EndTimestampQueryPS(const ICrGPUQueryPool* queryPool, CrGPUQueryId query)
{
	const CrGPUQueryPoolVulkan* vulkanQueryPool = static_cast<const CrGPUQueryPoolVulkan*>(queryPool);
	vkCmdWriteTimestamp(m_vkCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, vulkanQueryPool->GetVkQueryPool(), query.id);
}