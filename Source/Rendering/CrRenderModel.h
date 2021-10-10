#pragma once

#include "Rendering/CrVisibility.h"
#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrMaterial.h"

#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/Containers/CrVector.h"
#include "Core/Containers/CrFixedVector.h"
#include "Core/Containers/CrHashMap.h"
#include "Core/Containers/CrPair.h"
#include "Core/Containers/CrArray.h"

class CrRenderModel;
using CrRenderModelSharedHandle = CrSharedPtr<CrRenderModel>;

class CrMaterial;
using CrMaterialSharedHandle = CrSharedPtr<CrMaterial>;

class CrMesh;
using CrMeshSharedHandle = CrSharedPtr<CrMesh>;

struct CrRenderModelDescriptor
{
	CrFixedVector<CrMeshSharedHandle, 256> meshes;

	CrFixedVector<uint8_t, 256> materialIndices;

	CrFixedVector<CrMaterialSharedHandle, 256> materials;
};

class CrRenderModel
{
public:

	CrRenderModel() = default;

	CrRenderModel(const CrRenderModelDescriptor& descriptor);

	const CrBoundingBox& GetBoundingBox() const { return m_boundingBox; }

	const CrPair<const CrMeshSharedHandle&, const CrMaterialSharedHandle&> GetRenderMeshMaterial(uint32_t meshIndex) const
	{
		const CrMeshSharedHandle& renderMesh = m_renderMeshes[meshIndex];
		uint32_t materialIndex = (*m_materialMap.find(renderMesh.get())).second;
		const CrMaterialSharedHandle& material = m_materials[materialIndex];

		return CrPair<const CrMeshSharedHandle&, const CrMaterialSharedHandle&>(renderMesh, material);
	}

	const CrMaterialSharedHandle& GetMaterial(uint32_t meshIndex) const
	{
		return m_materials[meshIndex];
	}

	const CrGraphicsPipelineHandle& GetPipeline(uint32_t meshIndex, CrMaterialPipelineVariant::T pipelineVariant) const
	{
		return m_pipelines[meshIndex][pipelineVariant];
	}

	uint32_t GetRenderMeshCount() const
	{
		return (uint32_t)m_renderMeshes.size();
	}

	void ComputeBoundingBoxFromMeshes();

public:

	void Process();

	CrBoundingBox m_boundingBox;

private:

	CrHashMap<CrMesh*, uint8_t> m_materialMap;

	CrHashMap<CrMesh*, uint8_t> m_pipelineMap;

	// A model has access to a collection of render meshes, each with a material attached.
	// To start with we'll make all of these arrays of equal size, but they don't have to
	// be. A merging of a material + a render mesh produces a pipeline state. The pipeline
	// state knows everything about where it's going to be rendered (it needs to)

	CrVector<CrMeshSharedHandle> m_renderMeshes;


	CrVector<CrMaterialSharedHandle> m_materials;

	CrVector<CrArray<CrGraphicsPipelineHandle, CrMaterialPipelineVariant::Count>> m_pipelines;
};
