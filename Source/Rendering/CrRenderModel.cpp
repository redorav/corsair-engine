#include "CrRendering_pch.h"

#include "Rendering/CrRenderModel.h"
#include "Rendering/CrMesh.h"

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
