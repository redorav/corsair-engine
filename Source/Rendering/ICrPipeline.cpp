#include "Rendering/CrRendering_pch.h"

#include "Rendering/ICrPipeline.h"
#include "Rendering/ICrShader.h"
#include "Rendering/CrVertexDescriptor.h"

CrRenderTargetBlendDescriptor CrStandardPipelineStates::OpaqueBlend
{
	cr3d::BlendFactor::One,
	cr3d::BlendFactor::One,
	cr3d::BlendFactor::One,
	cr3d::BlendFactor::One,
	cr3d::ColorWriteComponent::All,
	cr3d::BlendOp::Add,
	cr3d::BlendOp::Add,
	false
};

CrRenderTargetBlendDescriptor CrStandardPipelineStates::AlphaBlend
{
	cr3d::BlendFactor::SrcAlpha,
	cr3d::BlendFactor::OneMinusSrcAlpha,
	cr3d::BlendFactor::One,
	cr3d::BlendFactor::Zero,
	cr3d::ColorWriteComponent::All,
	cr3d::BlendOp::Add,
	cr3d::BlendOp::Add,
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

ICrComputePipeline::ICrComputePipeline(ICrRenderDevice* renderDevice, const CrComputeShaderHandle& computeShader) 
	: CrGPUAutoDeletable(renderDevice), m_shader(computeShader)
{

}

ICrComputePipeline::~ICrComputePipeline() {}
