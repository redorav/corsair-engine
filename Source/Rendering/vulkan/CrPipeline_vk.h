#pragma once

#include "Rendering/ICrPipeline.h"

class CrGraphicsPipelineVulkan final : public ICrGraphicsPipeline
{
public:

	// Describes the resource binding layout for this pipeline
	// This is later used to bind the descriptor sets
	VkPipelineLayout m_vkPipelineLayout;

	VkPipeline m_vkPipeline;
};

class CrComputePipelineVulkan final : public ICrComputePipeline
{
public:

	VkPipelineLayout m_vkPipelineLayout;

	VkPipeline m_vkPipeline;
};