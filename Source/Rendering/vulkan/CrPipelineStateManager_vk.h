#pragma once

#include "Rendering/ICrPipelineStateManager.h"

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
};