#pragma once

#include "Rendering/ICrCommandBuffer.h"

#include "CrGPUBuffer_vk.h"
#include "CrPipeline_vk.h"
#include "CrVulkan.h"

#include "Rendering/CrRendering.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

class CrTextureVulkan;
class CrShaderBindingTableVulkan;

class CrCommandBufferVulkan final : public ICrCommandBuffer
{
public:

	CrCommandBufferVulkan(ICrCommandQueue* commandQueue);

	virtual ~CrCommandBufferVulkan() override;

	static void GetVkImageLayoutAndAccessFlags(bool isDepth, cr3d::ResourceState::T resourceState, VkImageLayout& imageLayout, VkAccessFlags& accessFlags);

	const VkCommandBuffer& GetVkCommandBuffer() const;

	VkCommandBuffer& GetVkCommandBuffer();

private:

	virtual void BeginPS() override;

	virtual void EndPS() override;

	virtual void BindGraphicsPipelineStatePS(const ICrGraphicsPipeline* pipelineState) override;

	virtual void BindComputePipelineStatePS(const ICrComputePipeline* computePipeline) override;

	virtual void ClearRenderTargetPS(const ICrTexture* renderTarget, const float4& color, uint32_t level, uint32_t slice, uint32_t levelCount, uint32_t sliceCount) override;

	virtual void DrawPS(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;

	//void DrawIndirectPS(CrIndirectArgs* indirectArgs, uint32_t indirectArgsOffset) override;

	virtual void DrawIndexedPS(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) override;

	//void DrawIndexedIndirectPS(CrIndirectArgs* indirectArgs, uint32_t indirectArgsOffset) override;

	virtual void DispatchPS(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) override;

	//void DispatchIndirectPS(CrIndirectArgs* indirectArgs) override;

	virtual void BeginDebugEventPS(const char* eventName, const float4& color) override;

	virtual void EndDebugEventPS() override;

	virtual void TransitionTexturePS(const ICrTexture* texture, cr3d::ResourceState::T initialState, cr3d::ResourceState::T destinationState) override;

	virtual void FlushGraphicsRenderStatePS() override;

	virtual void FlushComputeRenderStatePS() override;

	virtual void BeginRenderPassPS(const ICrRenderPass* renderPass, const ICrFramebuffer* frameBuffer, const CrRenderPassBeginParams& renderPassParams) override;

	virtual void EndRenderPassPS() override;

	void UpdateResourceTableVulkan(const CrShaderBindingTableVulkan& bindingTable, VkPipelineBindPoint vkPipelineBindPoint, VkPipelineLayout vkPipelineLayout);

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

inline void CrCommandBufferVulkan::BindGraphicsPipelineStatePS(const ICrGraphicsPipeline* graphicsPipeline)
{
	// In Vulkan we specify the type of pipeline. In DX12 for instance they are separate objects
	vkCmdBindPipeline(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<const CrGraphicsPipelineVulkan*>(graphicsPipeline)->m_vkPipeline);
}

inline void CrCommandBufferVulkan::BindComputePipelineStatePS(const ICrComputePipeline* computePipeline)
{
	// In Vulkan we specify the type of pipeline. In DX12 for instance they are separate objects
	vkCmdBindPipeline(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, static_cast<const CrComputePipelineVulkan*>(computePipeline)->m_vkPipeline);
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
		markerInfo.color[0] = color.r;
		markerInfo.color[1] = color.g;
		markerInfo.color[2] = color.b;
		markerInfo.color[3] = color.a;
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
