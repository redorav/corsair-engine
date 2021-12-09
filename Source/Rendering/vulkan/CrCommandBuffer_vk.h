#pragma once

#include "Rendering/ICrCommandBuffer.h"

#include "CrGPUBuffer_vk.h"

#include "CrPipeline_vk.h"

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
