#pragma once

#include "Graphics/IPipeline.h"

namespace crgfx
{
	class DeviceD3D12;

	class GraphicsPipelineD3D12 final : public IGraphicsPipeline
	{
	public:

		GraphicsPipelineD3D12
		(
			crgfx::DeviceD3D12* d3d12RenderDevice, const GraphicsPipelineDescriptor& pipelineDescriptor,
			const crgfx::GraphicsShaderHandle& graphicsShader, const VertexDescriptor& vertexDescriptor
		);

		~GraphicsPipelineD3D12();

		ID3D12PipelineState* GetD3D12PipelineState() const { return m_d3d12PipelineState; }

		ID3D12RootSignature* GetD3D12RootSignature() const { return m_d3d12RootSignature; }

		D3D12_PRIMITIVE_TOPOLOGY GetD3D12PrimitiveTopologyType() const { return m_d3d12PrimitiveTopology; }

#if !defined(CR_CONFIG_FINAL)

		virtual void RecompilePS(crgfx::IDevice* renderDevice, const crgfx::GraphicsShaderHandle& graphicsShader) override;

#endif

	private:

		void Initialize(crgfx::DeviceD3D12* d3d12RenderDevice, const GraphicsPipelineDescriptor& pipelineDescriptor, const crgfx::GraphicsShaderHandle& graphicsShader, const VertexDescriptor& vertexDescriptor);

		void Deinitialize();

		D3D12_PRIMITIVE_TOPOLOGY m_d3d12PrimitiveTopology;

		ID3D12RootSignature* m_d3d12RootSignature;

		ID3D12PipelineState* m_d3d12PipelineState;
	};

	class ComputePipelineD3D12 final : public IComputePipeline
	{
	public:

		ComputePipelineD3D12(crgfx::DeviceD3D12* d3d12RenderDevice, const crgfx::ComputeShaderHandle& computeShader);

		~ComputePipelineD3D12();

		ID3D12PipelineState* GetD3D12PipelineState() const { return m_d3d12PipelineState; }

		ID3D12RootSignature* GetD3D12RootSignature() const { return m_d3d12RootSignature; }

#if !defined(CR_CONFIG_FINAL)

		virtual void RecompilePS(crgfx::IDevice* renderDevice, const crgfx::ComputeShaderHandle& computeShader) override;

#endif

	private:

		void Initialize(crgfx::DeviceD3D12* d3d12RenderDevice, const crgfx::ComputeShaderHandle& computeShader);

		void Deinitialize();

		ID3D12RootSignature* m_d3d12RootSignature;

		ID3D12PipelineState* m_d3d12PipelineState;
	};
};