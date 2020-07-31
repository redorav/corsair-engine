#pragma once

#include "ICrCommandBuffer.h"
#include "CrGPUBuffer_vk.h"
#include "CrVulkan.h"
#include "CrRendering.h"

class CrVertexBufferCommon;
class CrIndexBufferCommon;
class ICrGraphicsPipeline;
class CrTextureVulkan;

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

	virtual void SetViewportPS(const CrViewport& viewport) override;

	virtual void SetScissorPS(uint32_t topLeftX, uint32_t topLeftY, uint32_t width, uint32_t height) override;

	virtual void BindIndexBufferPS(const ICrHardwareGPUBuffer* indexBuffer) override;

	virtual void BindVertexBuffersPS(const ICrHardwareGPUBuffer* vertexBuffer, uint32_t bindPoint) override;

	virtual void BindGraphicsPipelineStatePS(const ICrGraphicsPipeline* pipelineState) override;

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

	virtual void UpdateResourceTablesPS() override;

	virtual void BeginRenderPassPS(const ICrRenderPass* renderPass, const ICrFramebuffer* frameBuffer, const CrRenderPassBeginParams& renderPassParams) override;

	virtual void EndRenderPassPS(const ICrRenderPass* renderPass) override;

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

inline void CrCommandBufferVulkan::SetViewportPS(const CrViewport& viewport)
{
	// TODO Be able to set multiple viewports
	VkViewport vkViewport =
	{
		viewport.x, 
		viewport.y + viewport.height,
		viewport.width,
		-viewport.height, // Requires VK_KHR_maintenance1
		viewport.minDepth,
		viewport.maxDepth
	};

	vkCmdSetViewport(m_vkCommandBuffer, 0, 1, &vkViewport);
}

inline void CrCommandBufferVulkan::SetScissorPS(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	VkRect2D scissor = { { (int32_t) x, (int32_t) y }, { width, height } };
	vkCmdSetScissor(m_vkCommandBuffer, 0, 1, &scissor);
}

inline void CrCommandBufferVulkan::BindIndexBufferPS(const ICrHardwareGPUBuffer* indexBuffer)
{
	const CrHardwareGPUBufferVulkan* vulkanGPUBuffer = static_cast<const CrHardwareGPUBufferVulkan*>(indexBuffer);
	vkCmdBindIndexBuffer(m_vkCommandBuffer, vulkanGPUBuffer->GetVkBuffer(), 0, vulkanGPUBuffer->GetVkIndexType());
}

inline void CrCommandBufferVulkan::BindVertexBuffersPS(const ICrHardwareGPUBuffer* vertexBuffer, uint32_t bindPoint)
{
	const CrHardwareGPUBufferVulkan* vulkanGPUBuffer = static_cast<const CrHardwareGPUBufferVulkan*>(vertexBuffer);

	VkDeviceSize offsets[1] = { 0 };
	// TODO Shader bind location! Retrieve this from the PSO which should have the current shader
	// TODO Number of vertex shaders to be able to have several vertex streams ??
	// TODO Make sure function accepts multiple vertex buffers
	const VkBuffer vkBuffers[1] = { vulkanGPUBuffer->GetVkBuffer() };
	vkCmdBindVertexBuffers(m_vkCommandBuffer, bindPoint, 1, vkBuffers, offsets);
}

inline void CrCommandBufferVulkan::BindGraphicsPipelineStatePS(const ICrGraphicsPipeline* pipelineState)
{
	vkCmdBindPipeline(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineState->m_pipeline); // In Vulkan we specify the type of pipeline. In DX12 for instance they are separate objects
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
