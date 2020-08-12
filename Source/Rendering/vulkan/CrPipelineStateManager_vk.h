#pragma once

#include "ICrPipelineStateManager.h"

class CrRenderDeviceVulkan;

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

	virtual void CreateGraphicsPipelinePS
	(
		ICrGraphicsPipeline* graphicsPipeline, 
		const CrGraphicsPipelineDescriptor& psoDescriptor, 
		const ICrGraphicsShader* graphicsShader, 
		const CrVertexDescriptor& vertexDescriptor,
		const CrRenderPassDescriptor& renderPassDescriptor
	) override;

	virtual void InitPS() override;

	VkDevice m_vkDevice;

	VkPipeline m_vkPipelineState;

	VkPipelineCache m_vkPipelineCache;
};