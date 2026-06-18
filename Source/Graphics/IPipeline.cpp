#include "Graphics/CrRendering_pch.h"

#include "Graphics/IPipeline.h"
#include "Graphics/IShader.h"
#include "Graphics/CrVertexDescriptor.h"

crgfx::RenderTargetBlendDescriptor CrStandardPipelineStates::OpaqueBlend
{
	crgfx::BlendFactor::One,
	crgfx::BlendFactor::One,
	crgfx::BlendFactor::One,
	crgfx::BlendFactor::One,
	crgfx::ColorWriteComponent::All,
	crgfx::BlendOp::Add,
	crgfx::BlendOp::Add,
	false
};

crgfx::RenderTargetBlendDescriptor CrStandardPipelineStates::AlphaBlend
{
	crgfx::BlendFactor::SrcAlpha,
	crgfx::BlendFactor::OneMinusSrcAlpha,
	crgfx::BlendFactor::One,
	crgfx::BlendFactor::Zero,
	crgfx::ColorWriteComponent::All,
	crgfx::BlendOp::Add,
	crgfx::BlendOp::Add,
	true
};

namespace crgfx
{
	IGraphicsPipeline::IGraphicsPipeline(crgfx::IDevice* renderDevice, const GraphicsPipelineDescriptor& pipelineDescriptor, const crgfx::GraphicsShaderHandle& graphicsShader, const VertexDescriptor& vertexDescriptor)
		: CrGPUAutoDeletable(renderDevice)
		, m_shader(graphicsShader)
#if !defined(CR_CONFIG_FINAL)
		, m_pipelineDescriptor(pipelineDescriptor)
		, m_vertexDescriptor(vertexDescriptor)
#endif
	{
		m_usedVertexStreamCount = vertexDescriptor.GetStreamCount();
	}

	IGraphicsPipeline::~IGraphicsPipeline() {}

#if !defined(CR_CONFIG_FINAL)

	void IGraphicsPipeline::Recompile(crgfx::IDevice* renderDevice, const crgfx::GraphicsShaderHandle& graphicsShader)
	{
		RecompilePS(renderDevice, graphicsShader);
		m_shader = graphicsShader;
	}

#endif

	IComputePipeline::IComputePipeline(crgfx::IDevice* renderDevice, const crgfx::ComputeShaderHandle& computeShader)
		: CrGPUAutoDeletable(renderDevice), m_shader(computeShader)
	{
		const CrShaderReflectionHeader& reflection = computeShader->GetBytecode()->GetReflection();
		m_threadGroupSizeX = reflection.threadGroupSizeX;
		m_threadGroupSizeY = reflection.threadGroupSizeY;
		m_threadGroupSizeZ = reflection.threadGroupSizeZ;
	}

	IComputePipeline::~IComputePipeline() {}

#if !defined(CR_CONFIG_FINAL)

	void IComputePipeline::Recompile(crgfx::IDevice* renderDevice, const crgfx::ComputeShaderHandle& computeShader)
	{
		RecompilePS(renderDevice, computeShader);
		m_shader = computeShader;
	}

#endif
};