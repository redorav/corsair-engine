#pragma once

#include "Graphics/IPipeline.h"

namespace crgfx
{
	class DeviceVulkan;

	class GraphicsPipelineVulkan final : public IGraphicsPipeline
	{
	public:

		GraphicsPipelineVulkan(crgfx::DeviceVulkan* vulkanRenderDevice, const GraphicsPipelineDescriptor& pipelineDescriptor, const crgfx::GraphicsShaderBytecode& graphicsShaderBytecode, const VertexDescriptor& vertexDescriptor);

		~GraphicsPipelineVulkan();

		VkPipeline GetVkPipeline() const { return m_vkPipeline; }

		VkPipelineLayout GetVkPipelineLayout() const { return m_vkPipelineLayout; }

		VkDescriptorSetLayout GetVkDescriptorSetLayout() const { return m_vkDescriptorSetLayout; }

#if !defined(CR_CONFIG_FINAL)

		virtual void RecompilePS(crgfx::IDevice* renderDevice, const crgfx::GraphicsShaderBytecode& graphicsShaderBytecode) override;

#endif

	private:

		void Initialize(crgfx::DeviceVulkan* vulkanRenderDevice, const GraphicsPipelineDescriptor& pipelineDescriptor, const crgfx::GraphicsShaderBytecode& graphicsShaderBytecode, const VertexDescriptor& vertexDescriptor);

		void Deinitialize();

		// Describes the resource binding layout for this pipeline This is later used to bind the descriptor sets
		VkPipelineLayout m_vkPipelineLayout;

		// The layout is needed when allocating descriptor sets from a pool.
		// TODO There is only one at the moment but we should allow for more
		// TODO The descriptor set layout will probably move to the resource descriptor table
		VkDescriptorSetLayout m_vkDescriptorSetLayout;

		VkPipeline m_vkPipeline;
	};

	class ComputePipelineVulkan final : public IComputePipeline
	{
	public:

		ComputePipelineVulkan(crgfx::DeviceVulkan* vulkanRenderDevice, const crgfx::ComputeShaderBytecode& computeShaderBytecode);

		~ComputePipelineVulkan();

		VkPipeline GetVkPipeline() const { return m_vkPipeline; }

		VkPipelineLayout GetVkPipelineLayout() const { return m_vkPipelineLayout; }

		VkDescriptorSetLayout GetVkDescriptorSetLayout() const { return m_vkDescriptorSetLayout; }

#if !defined(CR_CONFIG_FINAL)

		virtual void RecompilePS(crgfx::IDevice* renderDevice, const crgfx::ComputeShaderBytecode& computeShaderBytecode) override;

#endif

	private:

		void Initialize(crgfx::DeviceVulkan* vulkanRenderDevice, const crgfx::ComputeShaderBytecode& computeShaderBytecode);

		void Deinitialize();

		VkPipelineLayout m_vkPipelineLayout;

		VkDescriptorSetLayout m_vkDescriptorSetLayout;

		VkPipeline m_vkPipeline;
	};
};