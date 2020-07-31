#pragma once

#include "ICrPipelineStateManager.h"

class CrRenderDeviceD3D12;

class CrPipelineStateManagerD3D12 final : public ICrPipelineStateManager
{
	friend class ICrPipelineStateManager;

public:

	CrPipelineStateManagerD3D12()
	{

	}

	~CrPipelineStateManagerD3D12()
	{

	}

private:

	virtual void CreateGraphicsPipelinePS(ICrGraphicsPipeline* graphicsPipeline, const CrGraphicsPipelineDescriptor& psoDescriptor, 
		const CrGraphicsShader* graphicsShader, const CrVertexDescriptor& vertexDescriptor) override;

	virtual void InitPS(ICrRenderDevice* renderDevice) override;
};