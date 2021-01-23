#pragma once

#include "Rendering/ICrPipelineStateManager.h"

class CrRenderDeviceVulkan;

class CrGraphicsPipelineVulkan : public ICrGraphicsPipeline
{
public:

	// Describes the resource binding layout for this pipeline
	// This is later used to bind the descriptor sets
	VkPipelineLayout m_vkPipelineLayout;

	VkPipeline m_vkPipeline;
};

class CrPipelineStateManagerVulkan final : public ICrPipelineStateManager
{
public:

	CrPipelineStateManagerVulkan()
	{

	}

	~CrPipelineStateManagerVulkan()
	{

	}

private:

	virtual ICrGraphicsPipeline* CreateGraphicsPipelinePS
	(
		const CrGraphicsPipelineDescriptor& psoDescriptor, 
		const ICrGraphicsShader* graphicsShader, 
		const CrVertexDescriptor& vertexDescriptor,
		const CrRenderPassDescriptor& renderPassDescriptor
	) override;
};