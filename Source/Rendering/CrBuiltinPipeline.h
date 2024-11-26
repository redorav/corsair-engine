#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/ICrPipeline.h"
#include "Rendering/CrVertexDescriptor.h"

#include "Core/Containers/CrHashMap.h"

namespace CrBuiltinShaders { enum T : uint32_t; }

// A collection of all builtin pipelines compiled on boot. They are available to any program that wants them
class CrBuiltinPipelines
{
public:

	void Initialize();

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

	CrHashMap<uint64_t, CrGraphicsPipelineHandle> m_builtinGraphicsPipelines;

	CrHashMap<uint64_t, CrComputePipelineHandle> m_builtinComputePipelines;
};

extern CrBuiltinPipelines BuiltinPipelines;