#pragma once

#include "Rendering/ICrPipeline.h"

class CrRenderDeviceD3D12;

class CrGraphicsPipelineD3D12 final : public ICrGraphicsPipeline
{
public:

	CrGraphicsPipelineD3D12
	(
		const CrRenderDeviceD3D12* d3d12RenderDevice, const CrGraphicsPipelineDescriptor& pipelineDescriptor, 
		const ICrGraphicsShader* graphicsShader, const CrVertexDescriptor& vertexDescriptor
	);

	ID3D12PipelineState* m_d3d12PipelineState;
};

class CrComputePipelineD3D12 final : public ICrComputePipeline
{
public:

	CrComputePipelineD3D12
	(
		const CrRenderDeviceD3D12* d3d12RenderDevice, const ICrComputeShader* computeShader
	);

	ID3D12PipelineState* m_d3d12PipelineState;
};