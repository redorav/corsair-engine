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
		const CrGraphicsPipelineDescriptor& psoDescriptor, 
		const CrGraphicsShaderHandle& graphicsShader, 
		const CrVertexDescriptor& vertexDescriptor,
		const CrRenderPassDescriptor& renderPassDescriptor
	);

	static ICrPipelineStateManager* Get();

protected:

	CrHashMap<uint64_t, CrGraphicsPipelineHandle> m_graphicsPipelines;

	ICrRenderDevice* m_renderDevice = nullptr;
};
