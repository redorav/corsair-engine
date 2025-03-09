#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/ICrPipeline.h"
#include "Rendering/CrVertexDescriptor.h"

#include "crstl/open_hashmap.h"

namespace CrBuiltinShaders { enum T : uint32_t; }

// A collection of all builtin pipelines compiled on boot. They are available to any program that wants them
class CrBuiltinPipelines
{
public:

	static void Initialize();

	static void Deinitialize();

	CrGraphicsPipelineHandle GetGraphicsPipeline
	(
		const CrGraphicsPipelineDescriptor& graphicsPipelineDescriptor,
		const CrVertexDescriptor& vertexDescriptor,
		CrBuiltinShaders::T vertexShader,
		CrBuiltinShaders::T pixelShader
	);

	CrComputePipelineHandle GetComputePipeline(CrBuiltinShaders::T computeShader);

	void RecompileComputePipelines();

	// Ubershader builtin pipelines

	CrGraphicsPipelineHandle BasicUbershaderForward;

	CrGraphicsPipelineHandle BasicUbershaderGBuffer;

	CrGraphicsPipelineHandle BasicUbershaderDebug;

private:

	CrBuiltinPipelines();

	crstl::open_hashmap<uint64_t, CrGraphicsPipelineHandle> m_builtinGraphicsPipelines;

	crstl::open_hashmap<uint64_t, CrComputePipelineHandle> m_builtinComputePipelines;
};

extern CrBuiltinPipelines* BuiltinPipelines;