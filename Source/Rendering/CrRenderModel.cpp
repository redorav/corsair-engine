#include "Rendering/CrRendering_pch.h"

#include "Rendering/CrRenderModel.h"
#include "Rendering/CrRenderMesh.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/CrPipelineStateManager.h"
#include "Rendering/ICrPipeline.h"

uint32_t CrRenderModelDescriptor::AddMaterial(const CrMaterialHandle& material)
{
	uint32_t currentIndex = (uint32_t)m_materials.size();
	m_materials.push_back(material);
	return currentIndex;
}

void CrRenderModelDescriptor::AddRenderMesh(const CrRenderMeshHandle& renderMesh, uint8_t materialIndex)
{
	m_meshes.push_back(renderMesh);
	m_materialIndices.push_back(materialIndex);
}

CrRenderModel::CrRenderModel(const CrRenderModelDescriptor& descriptor)
{
	// Copy the materials
	m_materials.reserve(descriptor.GetMaterialCount());
	for (uint32_t i = 0; i < descriptor.GetMaterialCount(); ++i)
	{
		const CrMaterialHandle& material = descriptor.GetMaterial(i);
		m_materials.push_back(material);
	}

	m_renderMeshes.reserve(descriptor.GetRenderMeshCount());
	m_pipelines.resize(descriptor.GetRenderMeshCount());

	// For every mesh-material combination, create the necessary pipeline objects
	for (uint32_t meshIndex = 0; meshIndex < descriptor.GetRenderMeshCount(); ++meshIndex)
	{
		const CrRenderMeshHandle& mesh = descriptor.GetRenderMesh(meshIndex);
		const CrMaterialHandle& material = descriptor.GetMaterial(descriptor.GetRenderMeshMaterial(meshIndex));

		m_renderMeshes.push_back(mesh);
		m_renderMeshMaterialIndex.push_back((uint8_t)descriptor.GetRenderMeshMaterial(meshIndex));

		for (CrMaterialPipelineVariant::T pipelineVariant = CrMaterialPipelineVariant::First; pipelineVariant < CrMaterialPipelineVariant::Count; ++pipelineVariant)
		{
			const CrMaterialPassProperties& passProperties = CrMaterialPassProperties::GetMaterialPassProperties(mesh, pipelineVariant);

			const CrGraphicsShaderHandle& graphicsShader = material->GetShader(passProperties.shaderVariant);

			if (graphicsShader)
			{
				CrGraphicsPipelineHandle pipeline = PipelineStateManager.GetGraphicsPipeline(passProperties.pipelineDescriptor, graphicsShader, mesh->GetVertexDescriptor());

				m_pipelines[meshIndex][pipelineVariant] = pipeline;
			}
		}
	}

	ComputeBoundingBoxFromMeshes();
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