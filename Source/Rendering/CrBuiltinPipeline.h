#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/ICrPipeline.h"
#include "Rendering/CrVertexDescriptor.h"

namespace CrBuiltinShaders { enum T : uint32_t; }

// This class handles builtin shader building. They get registered in a global database
// and are able to rebuild their contents live
class CrBuiltinGraphicsPipeline : public CrIntrusivePtrInterface
{
public:

	CrBuiltinGraphicsPipeline();

	CrBuiltinGraphicsPipeline
	(
		ICrRenderDevice* renderDevice,
		const CrGraphicsPipelineDescriptor& graphicsPipelineDescriptor,
		const CrVertexDescriptor& vertexDescriptor,
		CrBuiltinShaders::T vertexShaderIndex,
		CrBuiltinShaders::T pixelShaderIndex
	);

	const ICrGraphicsPipeline* GetPipeline() { return m_graphicsPipeline.get(); }

	void SetPipeline(const CrGraphicsPipelineHandle& graphicsPipeline) { m_graphicsPipeline = graphicsPipeline; }

	const CrGraphicsPipelineDescriptor& GetPipelineDescriptor() const { return m_graphicsPipelineDescriptor; }

	const CrVertexDescriptor& GetVertexDescriptor() const { return m_vertexDescriptor; }

	CrBuiltinShaders::T GetVertexShaderIndex() const { return m_vertexShaderIndex; }

	CrBuiltinShaders::T GetPixelShaderIndex() const { return m_pixelShaderIndex; }
	
private:

	CrGraphicsPipelineDescriptor m_graphicsPipelineDescriptor;

	CrVertexDescriptor m_vertexDescriptor;

	CrGraphicsPipelineHandle m_graphicsPipeline;

	CrBuiltinShaders::T m_vertexShaderIndex;

	CrBuiltinShaders::T m_pixelShaderIndex;
};

class CrBuiltinComputePipeline : public CrIntrusivePtrInterface
{
public:

	CrBuiltinComputePipeline() {}

	CrBuiltinComputePipeline(ICrRenderDevice* renderDevice, CrBuiltinShaders::T computeShaderIndex);

	const ICrComputePipeline* GetPipeline() { return m_computePipeline.get(); }

	void SetPipeline(const CrComputePipelineHandle& pipeline) { m_computePipeline = pipeline; }

	CrBuiltinShaders::T GetComputeShaderIndex() const { return m_computeShaderIndex; }

private:

	CrBuiltinShaders::T m_computeShaderIndex;

	CrComputePipelineHandle m_computePipeline;
};

using CrBuiltinGraphicsPipelineHandle = CrIntrusivePtr<CrBuiltinGraphicsPipeline>;
using CrBuiltinComputePipelineHandle = CrIntrusivePtr<CrBuiltinComputePipeline>;

// A collection of all builtin pipelines compiled on boot. They are available to any program that wants them
class CrBuiltinPipelines
{
public:

	static void Initialize();

	static CrBuiltinGraphicsPipelineHandle GetGraphicsPipeline
	(
		const CrGraphicsPipelineDescriptor& graphicsPipelineDescriptor,
		const CrVertexDescriptor& vertexDescriptor,
		CrBuiltinShaders::T vertexShader,
		CrBuiltinShaders::T pixelShader
	);

	static CrBuiltinComputePipelineHandle GetComputePipeline(CrBuiltinShaders::T computeShader);

	static void RecompileComputePipelines();

	// Ubershader builtin pipelines

	static CrBuiltinGraphicsPipelineHandle BasicUbershaderForward;

	static CrBuiltinGraphicsPipelineHandle BasicUbershaderGBuffer;

	static CrBuiltinGraphicsPipelineHandle BasicUbershaderDebug;

	static CrHashMap<uint64_t, CrBuiltinGraphicsPipelineHandle> m_builtinGraphicsPipelines;

	static CrHashMap<uint64_t, CrBuiltinComputePipelineHandle> m_builtinComputePipelines;
};