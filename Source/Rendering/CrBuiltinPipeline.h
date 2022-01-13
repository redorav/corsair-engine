#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/ICrPipeline.h"
#include "Rendering/CrVertexDescriptor.h"

namespace CrBuiltinShaders { enum T : uint32_t; }

// This class handles builtin shader building. They get registered in a global database
// and are able to rebuild their contents live
class CrBuiltinGraphicsPipeline
{
public:

	CrBuiltinGraphicsPipeline() {}

	CrBuiltinGraphicsPipeline
	(
		ICrRenderDevice* renderDevice,
		const CrGraphicsPipelineDescriptor& graphicsPipelineDescriptor,
		const CrVertexDescriptor& vertexDescriptor,
		CrBuiltinShaders::T vertexShader,
		CrBuiltinShaders::T pixelShader
	);

	const ICrGraphicsPipeline* get() { return m_graphicsPipeline.get(); }

	void RecompileGraphicsPipeline();

private:

	CrGraphicsPipelineDescriptor m_graphicsPipelineDescriptor;

	CrVertexDescriptor m_vertexDescriptor;

	CrGraphicsPipelineHandle m_graphicsPipeline;
};

class CrBuiltinComputePipeline
{
public:

	CrBuiltinComputePipeline() {}

	CrBuiltinComputePipeline(ICrRenderDevice* renderDevice, const CrComputePipelineDescriptor& computePipelineDescriptor, CrBuiltinShaders::T computeShader);

	const ICrComputePipeline* get() { return m_computePipeline.get(); }

private:

	CrComputePipelineDescriptor m_computePipelineDescriptor;

	CrComputePipelineHandle m_computePipeline;
};