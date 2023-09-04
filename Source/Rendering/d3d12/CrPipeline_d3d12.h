#pragma once

#include "Rendering/ICrPipeline.h"

class CrRenderDeviceD3D12;

class CrGraphicsPipelineD3D12 final : public ICrGraphicsPipeline
{
public:

	CrGraphicsPipelineD3D12
	(
		CrRenderDeviceD3D12* d3d12RenderDevice, const CrGraphicsPipelineDescriptor& pipelineDescriptor, 
		const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor
	);

	~CrGraphicsPipelineD3D12();

	ID3D12PipelineState* GetD3D12PipelineState() const { return m_d3d12PipelineState; }

	ID3D12RootSignature* GetD3D12RootSignature() const { return m_d3d12RootSignature; }

	D3D12_PRIMITIVE_TOPOLOGY GetD3D12PrimitiveTopologyType() const { return m_d3d12PrimitiveTopology; }

#if !defined(CR_CONFIG_FINAL)

	virtual void RecompilePS(ICrRenderDevice* renderDevice, const CrGraphicsShaderHandle& graphicsShader) override;

#endif

private:

	void Initialize(CrRenderDeviceD3D12* d3d12RenderDevice, const CrGraphicsPipelineDescriptor& pipelineDescriptor, const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor);

	void Deinitialize();

	D3D12_PRIMITIVE_TOPOLOGY m_d3d12PrimitiveTopology;

	ID3D12RootSignature* m_d3d12RootSignature;

	ID3D12PipelineState* m_d3d12PipelineState;
};

class CrComputePipelineD3D12 final : public ICrComputePipeline
{
public:

	CrComputePipelineD3D12(CrRenderDeviceD3D12* d3d12RenderDevice, const CrComputeShaderHandle& computeShader);

	~CrComputePipelineD3D12();

	ID3D12PipelineState* GetD3D12PipelineState() const { return m_d3d12PipelineState; }

	ID3D12RootSignature* GetD3D12RootSignature() const { return m_d3d12RootSignature; }

#if !defined(CR_CONFIG_FINAL)

	virtual void RecompilePS(ICrRenderDevice* renderDevice, const CrComputeShaderHandle& computeShader) override;

#endif

private:

	void Initialize(CrRenderDeviceD3D12* d3d12RenderDevice, const CrComputeShaderHandle& computeShader);

	void Deinitialize();

	ID3D12RootSignature* m_d3d12RootSignature;

	ID3D12PipelineState* m_d3d12PipelineState;
};