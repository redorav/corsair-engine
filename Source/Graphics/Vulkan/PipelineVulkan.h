#pragma once

#include "Graphics/IPipeline.h"

namespace crgfx
{
	class DeviceVulkan;

	class GraphicsPipelineVulkan final : public IGraphicsPipeline
	{
	public:

		GraphicsPipelineVulkan
		(
			crgfx::DeviceVulkan* vulkanRenderDevice, const GraphicsPipelineDescriptor& pipelineDescriptor,
			const crgfx::GraphicsShaderHandle& graphicsShader, const VertexDescriptor& vertexDescriptor
		);

		~GraphicsPipelineVulkan();

		VkPipeline GetVkPipeline() const { return m_vkPipeline; }

		VkPipelineLayout GetVkPipelineLayout() const { return m_vkPipelineLayout; }

#if !defined(CR_CONFIG_FINAL)

		virtual void RecompilePS(crgfx::IDevice* renderDevice, const crgfx::GraphicsShaderHandle& graphicsShader) override;

#endif

	private:

		void Initialize(crgfx::DeviceVulkan* vulkanRenderDevice, const GraphicsPipelineDescriptor& pipelineDescriptor, const crgfx::GraphicsShaderHandle& graphicsShader, const VertexDescriptor& vertexDescriptor);

		void Deinitialize();

		// Describes the resource binding layout for this pipeline
		// This is later used to bind the descriptor sets
		VkPipelineLayout m_vkPipelineLayout;

		VkPipeline m_vkPipeline;
	};

	class ComputePipelineVulkan final : public IComputePipeline
	{
	public:

		ComputePipelineVulkan(crgfx::DeviceVulkan* vulkanRenderDevice, const crgfx::ComputeShaderHandle& computeShader);

		~ComputePipelineVulkan();

		VkPipeline GetVkPipeline() const { return m_vkPipeline; }

		VkPipelineLayout GetVkPipelineLayout() const { return m_vkPipelineLayout; }

#if !defined(CR_CONFIG_FINAL)

		virtual void RecompilePS(crgfx::IDevice* renderDevice, const crgfx::ComputeShaderHandle& computeShader) override;

#endif

	private:

		void Initialize(crgfx::DeviceVulkan* vulkanRenderDevice, const crgfx::ComputeShaderHandle& computeShader);

		void Deinitialize();

		VkPipelineLayout m_vkPipelineLayout;

		VkPipeline m_vkPipeline;
	};
};