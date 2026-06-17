#pragma once

#include "Graphics/CrGraphicsForwardDeclarations.h"
#include "Graphics/IPipeline.h"
#include "Graphics/CrVertexDescriptor.h"

#include "crstl/open_hashmap.h"

namespace CrBuiltinShaders { enum T : uint32_t; }
namespace CrBuiltinCompute { enum T : uint32_t; }

// A collection of all builtin pipelines compiled on boot. They are available to any program that wants them
class CrBuiltinPipelines
{
public:

	static void Initialize();

	static void Deinitialize();

	crgfx::CrGraphicsPipelineHandle GetGraphicsPipeline
	(
		const crgfx::CrGraphicsPipelineDescriptor& graphicsPipelineDescriptor,
		const CrVertexDescriptor& vertexDescriptor,
		CrBuiltinShaders::T vertexShader,
		CrBuiltinShaders::T pixelShader
	);

	crgfx::CrComputePipelineHandle GetComputePipeline(CrBuiltinCompute::T computeShader);

	void RecompileBuiltinPipelines();

	// Ubershader builtin pipelines

	crgfx::CrGraphicsPipelineHandle BasicUbershaderForward;

	crgfx::CrGraphicsPipelineHandle BasicUbershaderGBuffer;

	crgfx::CrGraphicsPipelineHandle BasicUbershaderDebug;

private:

	CrBuiltinPipelines();

	crstl::open_hashmap<uint64_t, crgfx::CrGraphicsPipelineHandle> m_builtinGraphicsPipelines;

	crstl::open_hashmap<uint64_t, crgfx::CrComputePipelineHandle> m_builtinComputePipelines;
};

extern CrBuiltinPipelines* BuiltinPipelines;