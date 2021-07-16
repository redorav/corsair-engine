#include "CrRendering_pch.h"

#include "ICrPipelineStateManager.h"
#include "ICrShader.h"
#include "ICrPipeline.h"
#include "ICrRenderDevice.h"

#include "Core/CrMacros.h"
#include "Core/Containers/CrPair.h"

static ICrPipelineStateManager g_pipelineStateManager;

ICrPipelineStateManager* ICrPipelineStateManager::Get()
{
	return &g_pipelineStateManager;
}

CrGraphicsPipelineHandle ICrPipelineStateManager::GetGraphicsPipeline
(
	const CrGraphicsPipelineDescriptor& pipelineDescriptor, 
	const CrGraphicsShaderHandle& graphicsShader, 
	const CrVertexDescriptor& vertexDescriptor
)
{
	const CrHash& pipelineHash = pipelineDescriptor.GetHash();
	const CrHash& graphicsShaderHash = graphicsShader->GetHash();
	const CrHash& vertexDescriptorHash = vertexDescriptor.ComputeHash();

	const CrHash& combinedHash = pipelineHash << graphicsShaderHash << vertexDescriptorHash;

	const auto& it = m_graphicsPipelines.find(combinedHash.m_hash);

	CrGraphicsPipelineHandle graphicsPipeline;

	if (it != m_graphicsPipelines.end())
	{
		graphicsPipeline = it->second;
	}
	else
	{
		graphicsPipeline = m_renderDevice->CreateGraphicsPipeline(pipelineDescriptor, graphicsShader, vertexDescriptor);

		m_graphicsPipelines.insert({ combinedHash.m_hash, graphicsPipeline }); // Insert in the hashmap
	}

	return graphicsPipeline;
}

CrComputePipelineHandle ICrPipelineStateManager::GetComputePipeline(const CrComputePipelineDescriptor& pipelineDescriptor, const CrComputeShaderHandle& computeShader)
{
	const CrHash& pipelineHash = pipelineDescriptor.GetHash();
	const CrHash& computeShaderHash = computeShader->GetHash();

	const CrHash& combinedHash = pipelineHash << computeShaderHash;

	const auto& it = m_computePipelines.find(combinedHash.m_hash);
	CrComputePipelineHandle computePipeline;

	if (it != m_computePipelines.end())
	{
		computePipeline = it->second;
	}
	else
	{
		computePipeline = m_renderDevice->CreateComputePipeline(pipelineDescriptor, computeShader);

		m_computePipelines.insert({ combinedHash.m_hash, computePipeline }); // Insert in the hashmap
	}

	return computePipeline;
}

void ICrPipelineStateManager::Init(ICrRenderDevice* renderDevice)
{
	m_renderDevice = renderDevice;
}
