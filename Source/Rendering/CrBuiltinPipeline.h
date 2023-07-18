#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/ICrPipeline.h"
#include "Rendering/CrVertexDescriptor.h"

namespace CrBuiltinShaders { enum T : uint32_t; }

// This class handles builtin shader building. They get registered in a global database
// and are able to rebuild their contents live
class CrBuiltinGraphicsPipeline : CrIntrusivePtrInterface
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

	const ICrGraphicsPipeline* GetPipeline() { return m_graphicsPipeline.get(); }

private:

	CrGraphicsPipelineDescriptor m_graphicsPipelineDescriptor;

	CrVertexDescriptor m_vertexDescriptor;

	CrGraphicsPipelineHandle m_graphicsPipeline;
};

class CrBuiltinComputePipeline : public CrIntrusivePtrInterface
{
public:

	CrBuiltinComputePipeline() {}

	CrBuiltinComputePipeline(ICrRenderDevice* renderDevice, CrBuiltinShaders::T computeShader);

	const ICrComputePipeline* GetPipeline() { return m_computePipeline.get(); }

private:

	CrComputePipelineHandle m_computePipeline;
};

using CrBuiltinComputePipelineHandle = CrIntrusivePtr<CrBuiltinComputePipeline>;

// A collection of all builtin pipelines compiled on boot. They are available to any program that wants them
class CrBuiltinPipelines
{
public:

	static void Initialize(ICrRenderDevice* renderDevice);

	static CrBuiltinComputePipelineHandle GetComputePipeline(CrBuiltinShaders::T computeShader);

	// Ubershader builtin pipelines

	static CrBuiltinGraphicsPipeline BasicUbershaderForward;

	static CrBuiltinGraphicsPipeline BasicUbershaderGBuffer;

	static CrBuiltinGraphicsPipeline BasicUbershaderDebug;

	static CrArray<CrBuiltinComputePipelineHandle, 256> m_builtinComputePipelines;
};