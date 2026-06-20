#pragma once

#include "Graphics/CrGraphicsForwardDeclarations.h"
#include "Graphics/IPipeline.h"
#include "Graphics/VertexDescriptor.h"

#include "crstl/open_hashmap.h"

namespace CrBuiltinShaders { enum T : uint32_t; }
namespace CrBuiltinCompute { enum T : uint32_t; }

// A collection of all builtin pipelines compiled on boot. They are available to any program that wants them
class CrBuiltinPipelines
{
public:

	static void Initialize();

	static void Deinitialize();

	crgfx::GraphicsPipelineHandle GetGraphicsPipeline
	(
		const crgfx::GraphicsPipelineDescriptor& graphicsPipelineDescriptor,
		const crgfx::VertexDescriptor& vertexDescriptor,
		CrBuiltinShaders::T vertexShader,
		CrBuiltinShaders::T pixelShader
	);

	crgfx::ComputePipelineHandle GetComputePipeline(CrBuiltinCompute::T computeShader);

	void RecompileBuiltinPipelines();

	// Ubershader builtin pipelines

	crgfx::GraphicsPipelineHandle BasicUbershaderForward;

	crgfx::GraphicsPipelineHandle BasicUbershaderGBuffer;

	crgfx::GraphicsPipelineHandle BasicUbershaderDebug;

private:

	CrBuiltinPipelines();

	crstl::open_hashmap<uint64_t, crgfx::GraphicsPipelineHandle> m_builtinGraphicsPipelines;

	crstl::open_hashmap<uint64_t, crgfx::ComputePipelineHandle> m_builtinComputePipelines;
};

extern CrBuiltinPipelines* BuiltinPipelines;