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
	//CrHash vertexInputHash = 0; // TODO Create a Vertex Input Layout Hash

	const CrHash& combinedHash = pipelineHash << graphicsShaderHash;
	//combinedHash <<= vertexInputHash;

	const auto& it = m_graphicsPipelines.find(combinedHash.m_hash);

	CrGraphicsPipelineHandle graphicsPipeline;

	if (it != m_graphicsPipelines.end())
	{
		graphicsPipeline = it->second;
	}
	else
	{
		graphicsPipeline = m_renderDevice->CreateGraphicsPipeline(pipelineDescriptor, graphicsShader.get(), vertexDescriptor);
		graphicsPipeline->m_shader = graphicsShader; // TODO Move inside CreateGraphicsPipeline

		// Insert in the hashmap
		m_graphicsPipelines.insert({ combinedHash.m_hash, graphicsPipeline });
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
		computePipeline = m_renderDevice->CreateComputePipeline(pipelineDescriptor, computeShader.get());
		computePipeline->m_shader = computeShader;

		// Insert in the hashmap
		m_computePipelines.insert({ combinedHash.m_hash, computePipeline });
	}

	return computePipeline;
}

void ICrPipelineStateManager::Init(ICrRenderDevice* renderDevice)
{
	m_renderDevice = renderDevice;
}
