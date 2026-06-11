#include "Graphics/CrRendering_pch.h"

#include "Graphics/ICrPipeline.h"
#include "Graphics/ICrShader.h"
#include "Graphics/CrVertexDescriptor.h"

CrRenderTargetBlendDescriptor CrStandardPipelineStates::OpaqueBlend
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

CrRenderTargetBlendDescriptor CrStandardPipelineStates::AlphaBlend
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

ICrGraphicsPipeline::ICrGraphicsPipeline(ICrRenderDevice* renderDevice, const CrGraphicsPipelineDescriptor& pipelineDescriptor, const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor)
	: CrGPUAutoDeletable(renderDevice)
	, m_shader(graphicsShader)
#if !defined(CR_CONFIG_FINAL)
	, m_pipelineDescriptor(pipelineDescriptor)
	, m_vertexDescriptor(vertexDescriptor)
#endif
{
	m_usedVertexStreamCount = vertexDescriptor.GetStreamCount();
}

ICrGraphicsPipeline::~ICrGraphicsPipeline() {}

#if !defined(CR_CONFIG_FINAL)

void ICrGraphicsPipeline::Recompile(ICrRenderDevice* renderDevice, const CrGraphicsShaderHandle& graphicsShader)
{
	RecompilePS(renderDevice, graphicsShader);
	m_shader = graphicsShader;
}

#endif

ICrComputePipeline::ICrComputePipeline(ICrRenderDevice* renderDevice, const CrComputeShaderHandle& computeShader) 
	: CrGPUAutoDeletable(renderDevice), m_shader(computeShader)
{
	const CrShaderReflectionHeader& reflection = computeShader->GetBytecode()->GetReflection();
	m_threadGroupSizeX = reflection.threadGroupSizeX;
	m_threadGroupSizeY = reflection.threadGroupSizeY;
	m_threadGroupSizeZ = reflection.threadGroupSizeZ;
}

ICrComputePipeline::~ICrComputePipeline() {}

#if !defined(CR_CONFIG_FINAL)

void ICrComputePipeline::Recompile(ICrRenderDevice* renderDevice, const CrComputeShaderHandle& computeShader)
{
	RecompilePS(renderDevice, computeShader);
	m_shader = computeShader;
}

#endif