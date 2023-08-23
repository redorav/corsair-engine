#include "Rendering/CrRendering_pch.h"

#include "Rendering/ICrPipeline.h"
#include "Rendering/ICrShader.h"
#include "Rendering/CrVertexDescriptor.h"

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
