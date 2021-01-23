#pragma once

#include "Rendering/ICrPipeline.h"

class CrGraphicsPipelineVulkan : public ICrGraphicsPipeline
{
public:

	// Describes the resource binding layout for this pipeline
	// This is later used to bind the descriptor sets
	VkPipelineLayout m_vkPipelineLayout;

	VkPipeline m_vkPipeline;
};