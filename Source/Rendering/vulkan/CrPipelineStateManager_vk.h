#pragma once

#include "ICrPipelineStateManager.h"

class CrRenderDeviceVulkan;

class CrPipelineStateManagerVulkan final : public ICrPipelineStateManager
{
	friend class ICrPipelineStateManager;

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
		const CrGraphicsShader* graphicsShader, 
		const CrVertexDescriptor& vertexDescriptor,
		const CrRenderPassDescriptor& renderPassDescriptor
	) override;

	virtual void InitPS() override;

	VkDevice m_vkDevice;

	VkPipeline m_vkPipelineState;

	VkPipelineCache m_vkPipelineCache;
};