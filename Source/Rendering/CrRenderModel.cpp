#include "CrRendering_pch.h"

#include "Rendering/CrRenderModel.h"
#include "Rendering/CrMesh.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/CrPipelineStateManager.h"
#include "Rendering/ICrPipeline.h"

CrRenderModel::CrRenderModel(const CrRenderModelDescriptor& descriptor)
{
	// Copy the materials
	m_materials.reserve(descriptor.materials.size());
	for (uint32_t i = 0; i < descriptor.materials.size(); ++i)
	{
		const CrMaterialSharedHandle& material = descriptor.materials[i];
		m_materials.push_back(material);
	}

	m_renderMeshes.reserve(descriptor.meshes.size());
	m_pipelines.resize(descriptor.meshes.size());

	// For every combination of mesh and material, create the necessary pipeline objects
	for (uint32_t i = 0; i < descriptor.meshes.size(); ++i)
	{
		const CrMeshSharedHandle& mesh = descriptor.meshes[i];
		const CrMaterialSharedHandle& material = descriptor.materials[descriptor.materialIndices[i]];

		m_renderMeshes.push_back(mesh);
		m_materialMap.insert(CrPair<CrMesh*, uint8_t>(mesh.get(), (uint8_t)descriptor.materialIndices[i]));

		for (CrMaterialPipelineVariant::T pipelineVariant = CrMaterialPipelineVariant::First; pipelineVariant < CrMaterialPipelineVariant::Count; ++pipelineVariant)
		{
			const CrMaterialPassProperties& passProperties = CrMaterialPassProperties::GetProperties(pipelineVariant);

			CrGraphicsPipelineDescriptor pipelineDescriptor;
			pipelineDescriptor.renderTargets = passProperties.renderTargets;

			const CrGraphicsShaderHandle& graphicsShader = material->GetShader(passProperties.shaderVariant);

			CrGraphicsPipelineHandle pipeline = CrPipelineStateManager::Get()->GetGraphicsPipeline(pipelineDescriptor, graphicsShader, mesh->GetVertexDescriptor());

			m_pipelines[i][pipelineVariant] = pipeline;
		}
	}
}

void CrRenderModel::ComputeBoundingBoxFromMeshes()
{
	float3 minVertex = float3( FLT_MAX);
	float3 maxVertex = float3(-FLT_MAX);

	for (const CrRenderMeshSharedHandle& renderMesh : m_renderMeshes)
	{
		const CrBoundingBox& meshBox = renderMesh->GetBoundingBox();
		minVertex = min(minVertex, meshBox.center - meshBox.extents);
		maxVertex = max(minVertex, meshBox.center + meshBox.extents);
	}

	m_boundingBox.center  = (maxVertex + minVertex) * 0.5f;
	m_boundingBox.extents = (maxVertex - minVertex) * 0.5f;
}

void CrRenderModel::Process()
{

}
