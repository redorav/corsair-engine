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

	void CreateGraphicsPipelinePS(CrGraphicsPipeline* graphicsPipeline, const CrGraphicsPipelineDescriptor& psoDescriptor, const CrGraphicsShader* graphicsShader, const CrVertexDescriptor& vertexDescriptor);

	void InitPS(CrRenderDeviceVulkan* renderDevice);

	VkDevice m_vkDevice;

	VkPipeline m_vkPipelineState;

	VkPipelineCache m_vkPipelineCache;
};