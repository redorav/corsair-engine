#include "Rendering/CrRendering_pch.h"

#include "Rendering/ICrPipeline.h"
#include "Rendering/ICrShader.h"
#include "Rendering/CrVertexDescriptor.h"

ICrGraphicsPipeline::ICrGraphicsPipeline(ICrRenderDevice* renderDevice, const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor)
	: CrGPUDeletable(renderDevice), m_shader(graphicsShader)
{
	m_usedVertexStreamCount = vertexDescriptor.GetStreamCount();
}

ICrGraphicsPipeline::~ICrGraphicsPipeline() {}

ICrComputePipeline::ICrComputePipeline(ICrRenderDevice* renderDevice, const CrComputeShaderHandle& computeShader) 
	: CrGPUDeletable(renderDevice), m_shader(computeShader)
{

}

ICrComputePipeline::~ICrComputePipeline() {}
