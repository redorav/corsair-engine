#pragma once

#include "Core/Containers/CrHashMap.h"

#include "Core/SmartPointers/CrSharedPtr.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

class CrPipelineStateManager
{
public:

	CrPipelineStateManager() {}

	void Initialize(ICrRenderDevice* renderDevice);

	CrGraphicsPipelineHandle GetGraphicsPipeline
	(
		const CrGraphicsPipelineDescriptor& pipelineDescriptor, 
		const CrGraphicsShaderHandle& graphicsShader, 
		const CrVertexDescriptor& vertexDescriptor
	);

	CrComputePipelineHandle GetComputePipeline(const CrComputePipelineDescriptor& pipelineDescriptor, const CrComputeShaderHandle& computeShader);

	static CrPipelineStateManager* Get();

protected:

	CrHashMap<uint64_t, CrGraphicsPipelineHandle> m_graphicsPipelines;

	CrHashMap<uint64_t, CrComputePipelineHandle> m_computePipelines;

	ICrRenderDevice* m_renderDevice = nullptr;
};
