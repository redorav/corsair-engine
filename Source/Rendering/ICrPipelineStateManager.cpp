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
	const CrGraphicsPipelineDescriptor& psoDescriptor, 
	const CrGraphicsShaderHandle& graphicsShader, 
	const CrVertexDescriptor& vertexDescriptor,
	const CrRenderPassDescriptor& renderPassDescriptor
)
{
	const CrHash& psoHash = psoDescriptor.GetHash();
	const CrHash& graphicsShaderHash = graphicsShader->GetHash();
	//CrHash vertexInputHash = 0; // TODO Create a Vertex Input Layout Hash

	const CrHash& combinedHash = psoHash << graphicsShaderHash;
	//combinedHash <<= vertexInputHash;

	eastl::hashtable_iterator<CrPair<const uint64_t, CrGraphicsPipelineHandle>, false, false> it = m_graphicsPipelines.find(combinedHash.m_hash);

	CrGraphicsPipelineHandle graphicsPipeline = nullptr;
	if (it != m_graphicsPipelines.end())
	{
		graphicsPipeline = it->second;
	}
	else
	{
		graphicsPipeline = m_renderDevice->CreateGraphicsPipeline(psoDescriptor, graphicsShader.get(), vertexDescriptor, renderPassDescriptor);
		graphicsPipeline->m_shader = graphicsShader;

		// Insert in the hashmap
		m_graphicsPipelines.insert({combinedHash.m_hash, graphicsPipeline});
	}

	return graphicsPipeline;
}

void ICrPipelineStateManager::Init(ICrRenderDevice* renderDevice)
{
	m_renderDevice = renderDevice;
}
