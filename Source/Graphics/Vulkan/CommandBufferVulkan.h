#pragma once

#include "Graphics/ICommandBuffer.h"

#include "PipelineVulkan.h"

#include "GPUQueryPoolVulkan.h"

#include "CrVulkan.h"

#include "Graphics/CrGraphicsForwardDeclarations.h"

#include "Graphics/CrCPUStackAllocator.h"

namespace crgfx
{
	class ShaderBindingLayout;

	class CommandBufferVulkan final : public crgfx::ICommandBuffer
	{
	public:

		CommandBufferVulkan(crgfx::DeviceVulkan* renderDevice, const crgfx::CommandBufferDescriptor& descriptor);

		virtual ~CommandBufferVulkan() override;

		const VkCommandBuffer& GetVkCommandBuffer() const;

		VkCommandBuffer& GetVkCommandBuffer();

	private:

		virtual void BeginPS() override;

		virtual void EndPS() override;

		virtual void ClearRenderTargetPS(const crgfx::ITexture* renderTarget, const float4& color, uint32_t mip, uint32_t slice, uint32_t mipCount, uint32_t sliceCount) override;

		virtual void DrawPS(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;

		virtual void DrawIndexedPS(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) override;

		virtual void DispatchPS(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) override;

		virtual void DrawIndirectPS(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count) override;

		virtual void DrawIndexedIndirectPS(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count) override;

		virtual void DispatchIndirectPS(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset) override;

		virtual void BeginDebugEventPS(const char* eventName, const float4& color) override;

		virtual void EndDebugEventPS() override;

		virtual void InsertDebugMarkerPS(const char* markerName, const float4& color) override;

		virtual void BeginTimestampQueryPS(const IGPUQueryPool* queryPool, CrGPUQueryId query) override;

		virtual void EndTimestampQueryPS(const IGPUQueryPool* queryPool, CrGPUQueryId query) override;

		virtual void ResetGPUQueriesPS(const IGPUQueryPool* queryPool, uint32_t start, uint32_t count) override;

		virtual void ResolveGPUQueriesPS(const IGPUQueryPool* queryPool, uint32_t start, uint32_t count) override;

		virtual void FlushGraphicsRenderStatePS() override;

		virtual void FlushComputeRenderStatePS() override;

		virtual void BeginRenderPassPS(const crgfx::RenderPassDescriptor& renderPassDescriptor) override;

		virtual void EndRenderPassPS() override;

		void GatherImageAndBufferBarriers(const crgfx::RenderPassDescriptor::BufferTransitionVector& buffers, const crgfx::RenderPassDescriptor::TextureTransitionVector& textures);

		void QueueVkImageBarrier(const crgfx::ITexture* texture, uint32_t mipmapStart, uint32_t mipmapCount, uint32_t sliceStart, uint32_t sliceCount,
			const crgfx::TextureState& sourceState, const crgfx::TextureState& destinationState);

		void FlushImageAndBufferBarriers();

		void UpdateResourceTableVulkan(const crgfx::ShaderBindingLayout& bindingLayout, VkPipelineBindPoint vkPipelineBindPoint, VkDescriptorSetLayout vkDescriptorSetLayout, VkPipelineLayout vkPipelineLayout);

		VkCommandBuffer m_vkCommandBuffer;

		VkDescriptorPool m_vkDescriptorPool;

		// Barrier processing

		VkPipelineStageFlags m_srcStageMask = VK_PIPELINE_STAGE_NONE; // srcStageMask is an OR of all pipeline barrier stage masks

		VkPipelineStageFlags m_destStageMask = VK_PIPELINE_STAGE_NONE; // destStageMask is an OR of all pipeline barrier stage masks

		crstl::fixed_vector<VkBufferMemoryBarrier, RenderPassDescriptor::MaxTransitionCount> m_bufferMemoryBarriers;

		crstl::fixed_vector<VkImageMemoryBarrier, RenderPassDescriptor::MaxTransitionCount> m_imageMemoryBarriers;
	};

	inline const VkCommandBuffer& CommandBufferVulkan::GetVkCommandBuffer() const
	{
		return m_vkCommandBuffer;
	}

	inline VkCommandBuffer& CommandBufferVulkan::GetVkCommandBuffer()
	{
		return m_vkCommandBuffer;
	}

	inline void CommandBufferVulkan::DrawPS(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
	{
		vkCmdDraw(m_vkCommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
	}

	inline void CommandBufferVulkan::DrawIndexedPS(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
	{
		vkCmdDrawIndexed(m_vkCommandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	inline void CommandBufferVulkan::DispatchPS(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
	{
		vkCmdDispatch(m_vkCommandBuffer, threadGroupCountX, threadGroupCountY, threadGroupCountZ);
	}

	inline void CommandBufferVulkan::BeginDebugEventPS(const char* eventName, const float4& color)
	{
		if (vkCmdBeginDebugUtilsLabel)
		{
			VkDebugUtilsLabelEXT debugUtilsLabel = {};
			debugUtilsLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
			store(debugUtilsLabel.color, color);
			debugUtilsLabel.pLabelName = eventName;
			vkCmdBeginDebugUtilsLabel(m_vkCommandBuffer, &debugUtilsLabel);
		}
	}

	inline void CommandBufferVulkan::EndDebugEventPS()
	{
		if (vkCmdEndDebugUtilsLabel)
		{
			vkCmdEndDebugUtilsLabel(m_vkCommandBuffer);
		}
	}

	inline void CommandBufferVulkan::InsertDebugMarkerPS(const char* markerName, const float4& color)
	{
		if (vkCmdInsertDebugUtilsLabel)
		{
			VkDebugUtilsLabelEXT debugUtilsLabel = {};
			debugUtilsLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
			store(debugUtilsLabel.color, color);
			debugUtilsLabel.pLabelName = markerName;
			vkCmdInsertDebugUtilsLabel(m_vkCommandBuffer, &debugUtilsLabel);
		}
	}

	inline void CommandBufferVulkan::BeginTimestampQueryPS(const IGPUQueryPool* queryPool, CrGPUQueryId query)
	{
		const CrGPUQueryPoolVulkan* vulkanQueryPool = static_cast<const CrGPUQueryPoolVulkan*>(queryPool);
		vkCmdWriteTimestamp(m_vkCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, vulkanQueryPool->GetVkQueryPool(), query.id);
	}

	inline void CommandBufferVulkan::EndTimestampQueryPS(const IGPUQueryPool* queryPool, CrGPUQueryId query)
	{
		const CrGPUQueryPoolVulkan* vulkanQueryPool = static_cast<const CrGPUQueryPoolVulkan*>(queryPool);
		vkCmdWriteTimestamp(m_vkCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, vulkanQueryPool->GetVkQueryPool(), query.id);
	}
};