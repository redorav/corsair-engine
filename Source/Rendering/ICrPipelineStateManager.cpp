#include "CrRendering_pch.h"

#include "ICrPipelineStateManager.h"
#include "CrShaderGen.h"

#include "Core/CrMacros.h"
#include GRAPHICS_API_PATH(CrPipelineStateManager)
#include GRAPHICS_API_PATH(CrRenderDevice)

#include "Core/Containers/CrPair.h"

static CrPipelineStateManagerVulkan g_pipelineStateManager;

ICrPipelineStateManager* ICrPipelineStateManager::Get()
{
	return &g_pipelineStateManager;
}

CrGraphicsPipeline* ICrPipelineStateManager::GetGraphicsPipeline(const CrGraphicsPipelineDescriptor& psoDescriptor, const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor)
{
	const CrHash& psoHash = psoDescriptor.GetHash();
	const CrHash& graphicsShaderHash = graphicsShader->GetHash();
	//CrHash vertexInputHash = 0; // TODO Create a Vertex Input Layout Hash

	const CrHash& combinedHash = psoHash << graphicsShaderHash;
	//combinedHash <<= vertexInputHash;

	eastl::hashtable_iterator<CrPair<const uint64_t, CrGraphicsPipeline*>, false, false> it = m_graphicsPipelines.find(combinedHash.m_hash);

	CrGraphicsPipeline* graphicsPipeline = nullptr;
	if (it != m_graphicsPipelines.end())
	{
		graphicsPipeline = it->second;
	}
	else
	{
		graphicsPipeline = new CrGraphicsPipeline;
		graphicsPipeline->m_shader = graphicsShader;
		static_cast<CrPipelineStateManagerVulkan*>(this)->CreateGraphicsPipelinePS(graphicsPipeline, psoDescriptor, graphicsShader.get(), vertexDescriptor);

		// Insert in the hashmap
		m_graphicsPipelines.insert({combinedHash.m_hash, graphicsPipeline});
	}

	return graphicsPipeline;
}

void ICrPipelineStateManager::Init(ICrRenderDevice* renderDevice)
{
	static_cast<CrPipelineStateManagerVulkan*>(this)->InitPS(static_cast<CrRenderDeviceVulkan*>(renderDevice));
}
