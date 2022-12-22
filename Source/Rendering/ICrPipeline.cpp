#include "Rendering/CrRendering_pch.h"

#include "Rendering/ICrPipeline.h"
#include "Rendering/CrVertexDescriptor.h"

ICrGraphicsPipeline::ICrGraphicsPipeline(const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor) : m_shader(graphicsShader)
{
	m_usedVertexStreamCount = vertexDescriptor.GetStreamCount();
}