#include "CrRendering_pch.h"

#include "Rendering/CrRenderModel.h"
#include "Rendering/CrRenderMesh.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/CrPipelineStateManager.h"
#include "Rendering/ICrPipeline.h"

CrRenderModel::CrRenderModel(const CrRenderModelDescriptor& descriptor)
{
	// Copy the materials
	m_materials.reserve(descriptor.materials.size());
	for (uint32_t i = 0; i < descriptor.materials.size(); ++i)
	{
		const CrMaterialHandle& material = descriptor.materials[i];
		m_materials.push_back(material);
	}

	m_renderMeshes.reserve(descriptor.meshes.size());
	m_pipelines.resize(descriptor.meshes.size());

	// For every mesh-material combination, create the necessary pipeline objects
	for (uint32_t meshIndex = 0; meshIndex < descriptor.meshes.size(); ++meshIndex)
	{
		const CrRenderMeshHandle& mesh = descriptor.meshes[meshIndex];
		const CrMaterialHandle& material = descriptor.materials[descriptor.materialIndices[meshIndex]];

		m_renderMeshes.push_back(mesh);
		m_renderMeshMaterialIndex.push_back((uint8_t)descriptor.materialIndices[meshIndex]);

		for (CrMaterialPipelineVariant::T pipelineVariant = CrMaterialPipelineVariant::First; pipelineVariant < CrMaterialPipelineVariant::Count; ++pipelineVariant)
		{
			const CrMaterialPassProperties& passProperties = CrMaterialPassProperties::GetMaterialPassProperties(pipelineVariant);

			CrGraphicsPipelineDescriptor pipelineDescriptor;
			pipelineDescriptor.renderTargets = passProperties.renderTargets;

			const CrGraphicsShaderHandle& graphicsShader = material->GetShader(passProperties.shaderVariant);

			CrGraphicsPipelineHandle pipeline = CrPipelineStateManager::Get().GetGraphicsPipeline(pipelineDescriptor, graphicsShader, mesh->GetVertexDescriptor());

			m_pipelines[meshIndex][pipelineVariant] = pipeline;
		}
	}
}

void CrRenderModel::ComputeBoundingBoxFromMeshes()
{
	float3 minVertex = float3( FLT_MAX);
	float3 maxVertex = float3(-FLT_MAX);

	for (const CrRenderMeshHandle& renderMesh : m_renderMeshes)
	{
		const CrBoundingBox& meshBox = renderMesh->GetBoundingBox();
		minVertex = min(minVertex, meshBox.center - meshBox.extents);
		maxVertex = max(maxVertex, meshBox.center + meshBox.extents);
	}

	m_boundingBox.center  = (maxVertex + minVertex) * 0.5f;
	m_boundingBox.extents = (maxVertex - minVertex) * 0.5f;
}