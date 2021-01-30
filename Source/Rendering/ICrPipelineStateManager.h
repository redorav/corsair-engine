#pragma once

#include "Core/Containers/CrHashMap.h"

#include "Core/SmartPointers/CrSharedPtr.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

class ICrPipelineStateManager
{
public:

	ICrPipelineStateManager() {}

	void Init(ICrRenderDevice* renderDevice);

	CrGraphicsPipelineHandle GetGraphicsPipeline
	(
		const CrGraphicsPipelineDescriptor& pipelineDescriptor, 
		const CrGraphicsShaderHandle& graphicsShader, 
		const CrVertexDescriptor& vertexDescriptor,
		const CrRenderPassDescriptor& renderPassDescriptor
	);

	CrComputePipelineHandle GetComputePipeline(const CrComputePipelineDescriptor& pipelineDescriptor, const CrComputeShaderHandle& computeShader);

	static ICrPipelineStateManager* Get();

protected:

	CrHashMap<uint64_t, CrGraphicsPipelineHandle> m_graphicsPipelines;

	CrHashMap<uint64_t, CrComputePipelineHandle> m_computePipelines;

	ICrRenderDevice* m_renderDevice = nullptr;
};
