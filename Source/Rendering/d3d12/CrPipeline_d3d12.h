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

	ID3D12PipelineState* GetD3D12PipelineState() const { return m_d3d12PipelineState; }
	D3D12_PRIMITIVE_TOPOLOGY GetD3D12PrimitiveTopologyType() const { return m_d3d12PrimitiveTopology; }

private:

	D3D12_PRIMITIVE_TOPOLOGY m_d3d12PrimitiveTopology;

	ID3D12PipelineState* m_d3d12PipelineState;
};

class CrComputePipelineD3D12 final : public ICrComputePipeline
{
public:

	CrComputePipelineD3D12
	(
		const CrRenderDeviceD3D12* d3d12RenderDevice, const ICrComputeShader* computeShader
	);

	ID3D12PipelineState* GetD3D12PipelineState() const { return m_d3d12PipelineState; }
	ID3D12PipelineState* m_d3d12PipelineState;
};