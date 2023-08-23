#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/ICrPipeline.h"
#include "Rendering/CrVertexDescriptor.h"

namespace CrBuiltinShaders { enum T : uint32_t; }

// A collection of all builtin pipelines compiled on boot. They are available to any program that wants them
class CrBuiltinPipelines
{
public:

	static void Initialize();

	static CrGraphicsPipelineHandle GetGraphicsPipeline
	(
		const CrGraphicsPipelineDescriptor& graphicsPipelineDescriptor,
		const CrVertexDescriptor& vertexDescriptor,
		CrBuiltinShaders::T vertexShader,
		CrBuiltinShaders::T pixelShader
	);

	static CrComputePipelineHandle GetComputePipeline(CrBuiltinShaders::T computeShader);

	static void RecompileComputePipelines();

	// Ubershader builtin pipelines

	static CrGraphicsPipelineHandle BasicUbershaderForward;

	static CrGraphicsPipelineHandle BasicUbershaderGBuffer;

	static CrGraphicsPipelineHandle BasicUbershaderDebug;

	static CrHashMap<uint64_t, CrGraphicsPipelineHandle> m_builtinGraphicsPipelines;

	static CrHashMap<uint64_t, CrComputePipelineHandle> m_builtinComputePipelines;
};