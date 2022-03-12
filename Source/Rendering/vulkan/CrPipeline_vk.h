#pragma once

#include "Rendering/ICrPipeline.h"

class CrRenderDeviceVulkan;

class CrGraphicsPipelineVulkan final : public ICrGraphicsPipeline
{
public:

	CrGraphicsPipelineVulkan
	(
		const CrRenderDeviceVulkan* vulkanRenderDevice, const CrGraphicsPipelineDescriptor& pipelineDescriptor, 
		const ICrGraphicsShader* graphicsShader, const CrVertexDescriptor& vertexDescriptor
	);

	VkPipeline GetVkPipeline() const { return m_vkPipeline; }

	VkPipelineLayout GetVkPipelineLayout() const { return m_vkPipelineLayout; }

private:

	// Describes the resource binding layout for this pipeline
	// This is later used to bind the descriptor sets
	VkPipelineLayout m_vkPipelineLayout;

	VkPipeline m_vkPipeline;
};

class CrComputePipelineVulkan final : public ICrComputePipeline
{
public:

	CrComputePipelineVulkan(const CrRenderDeviceVulkan* vulkanRenderDevice, const ICrComputeShader* computeShader);

	VkPipeline GetVkPipeline() const { return m_vkPipeline; }

	VkPipelineLayout GetVkPipelineLayout() const { return m_vkPipelineLayout; }

private:

	VkPipelineLayout m_vkPipelineLayout;

	VkPipeline m_vkPipeline;
};