#include "Rendering/CrRendering_pch.h"

#include "Rendering/CrPipelineStateManager.h"
#include "Rendering/ICrShader.h"
#include "Rendering/ICrPipeline.h"
#include "Rendering/ICrRenderDevice.h"

#include "Core/CrMacros.h"
#include "Core/Containers/CrPair.h"

CrPipelineStateManager PipelineStateManager;

void CrPipelineStateManager::Initialize(ICrRenderDevice* renderDevice)
{
	m_renderDevice = renderDevice;
}

CrGraphicsPipelineHandle CrPipelineStateManager::GetGraphicsPipeline
(
	const CrGraphicsPipelineDescriptor& pipelineDescriptor, 
	const CrGraphicsShaderHandle& graphicsShader, 
	const CrVertexDescriptor& vertexDescriptor
)
{
	CrAssertMsg(graphicsShader != nullptr, "Invalid graphics shader passed to pipeline creation");

	const CrHash pipelineHash = pipelineDescriptor.ComputeHash();
	const CrHash graphicsShaderHash = graphicsShader->GetHash();
	const CrHash vertexDescriptorHash = vertexDescriptor.ComputeHash();

	const CrHash combinedHash = pipelineHash + graphicsShaderHash + vertexDescriptorHash;

	const auto& it = m_graphicsPipelines.find(combinedHash.GetHash());

	CrGraphicsPipelineHandle graphicsPipeline;

	if (it != m_graphicsPipelines.end())
	{
		graphicsPipeline = it->second;
	}
	else
	{
		graphicsPipeline = m_renderDevice->CreateGraphicsPipeline(pipelineDescriptor, graphicsShader, vertexDescriptor);

		m_graphicsPipelines.insert(combinedHash.GetHash(), graphicsPipeline); // Insert in the hashmap
	}

	return graphicsPipeline;
}

CrComputePipelineHandle CrPipelineStateManager::GetComputePipeline(const CrComputeShaderHandle& computeShader)
{
	CrAssertMsg(computeShader != nullptr, "Invalid compute shader passed to pipeline creation");

	const CrHash computeShaderHash = computeShader->GetHash();

	const auto& it = m_computePipelines.find(computeShaderHash.GetHash());
	CrComputePipelineHandle computePipeline;

	if (it != m_computePipelines.end())
	{
		computePipeline = it->second;
	}
	else
	{
		computePipeline = m_renderDevice->CreateComputePipeline(computeShader);

		m_computePipelines.insert(computeShaderHash.GetHash(), computePipeline); // Insert in the hashmap
	}

	return computePipeline;
}
