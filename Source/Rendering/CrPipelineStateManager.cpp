#include "CrRendering_pch.h"

#include "CrPipelineStateManager.h"
#include "ICrShader.h"
#include "ICrPipeline.h"
#include "ICrRenderDevice.h"

#include "Core/CrMacros.h"
#include "Core/Containers/CrPair.h"

static CrPipelineStateManager g_pipelineStateManager;

CrPipelineStateManager* CrPipelineStateManager::Get()
{
	return &g_pipelineStateManager;
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

	const CrHash combinedHash = pipelineHash << graphicsShaderHash << vertexDescriptorHash;

	const auto& it = m_graphicsPipelines.find(combinedHash.GetHash());

	CrGraphicsPipelineHandle graphicsPipeline;

	if (it != m_graphicsPipelines.end())
	{
		graphicsPipeline = it->second;
	}
	else
	{
		graphicsPipeline = m_renderDevice->CreateGraphicsPipeline(pipelineDescriptor, graphicsShader, vertexDescriptor);

		m_graphicsPipelines.insert({ combinedHash.GetHash(), graphicsPipeline }); // Insert in the hashmap
	}

	return graphicsPipeline;
}

CrComputePipelineHandle CrPipelineStateManager::GetComputePipeline(const CrComputePipelineDescriptor& pipelineDescriptor, const CrComputeShaderHandle& computeShader)
{
	CrAssertMsg(computeShader != nullptr, "Invalid compute shader passed to pipeline creation");

	const CrHash pipelineHash = pipelineDescriptor.ComputeHash();
	const CrHash computeShaderHash = computeShader->GetHash();

	const CrHash combinedHash = pipelineHash << computeShaderHash;

	const auto& it = m_computePipelines.find(combinedHash.GetHash());
	CrComputePipelineHandle computePipeline;

	if (it != m_computePipelines.end())
	{
		computePipeline = it->second;
	}
	else
	{
		computePipeline = m_renderDevice->CreateComputePipeline(pipelineDescriptor, computeShader);

		m_computePipelines.insert({ combinedHash.GetHash(), computePipeline }); // Insert in the hashmap
	}

	return computePipeline;
}

void CrPipelineStateManager::Initialize(ICrRenderDevice* renderDevice)
{
	m_renderDevice = renderDevice;
}
