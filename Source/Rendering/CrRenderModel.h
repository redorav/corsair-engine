#pragma once

#include "Rendering/CrVisibility.h"
#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/CrRenderMesh.h"

#include "Core/SmartPointers/CrIntrusivePtr.h"
#include "Core/Containers/CrVector.h"
#include "Core/Containers/CrFixedVector.h"
#include "Core/Containers/CrHashMap.h"
#include "Core/Containers/CrPair.h"
#include "Core/Containers/CrArray.h"

struct CrRenderModelDescriptor
{
	uint32_t AddMaterial(const CrMaterialHandle& material);

	void AddRenderMesh(const CrRenderMeshHandle& renderMesh, uint8_t materialIndex);

	uint32_t GetMaterialCount() const { return (uint32_t)m_materials.size(); }

	uint32_t GetRenderMeshCount() const { return (uint32_t)m_meshes.size(); }

	const CrMaterialHandle& GetMaterial(uint32_t materialIndex) const { return m_materials[materialIndex]; }

	const CrRenderMeshHandle& GetRenderMesh(uint32_t renderMeshIndex) const { return m_meshes[renderMeshIndex]; }

	const uint32_t GetRenderMeshMaterial(uint32_t renderMeshIndex) const { return m_materialIndices[renderMeshIndex]; }

private:

	CrFixedVector<CrRenderMeshHandle, 256> m_meshes;

	CrFixedVector<uint8_t, 256> m_materialIndices;

	CrFixedVector<CrMaterialHandle, 256> m_materials;
};

class CrRenderModel final : public CrIntrusivePtrInterface
{
public:

	CrRenderModel() = default;

	CrRenderModel(const CrRenderModelDescriptor& descriptor);

	const CrBoundingBox& GetBoundingBox() const { return m_boundingBox; }

	const CrPair<const CrRenderMeshHandle&, const CrMaterialHandle&> GetRenderMeshMaterial(uint32_t meshIndex) const
	{
		const CrRenderMeshHandle& renderMesh = m_renderMeshes[meshIndex];
		uint32_t materialIndex = m_renderMeshMaterialIndex[meshIndex];
		const CrMaterialHandle& material = m_materials[materialIndex];

		return CrPair<const CrRenderMeshHandle&, const CrMaterialHandle&>(renderMesh, material);
	}

	const CrMaterialHandle& GetMaterial(uint32_t meshIndex) const
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

private:

	void ComputeBoundingBoxFromMeshes();

	CrBoundingBox m_boundingBox;

	CrHashMap<CrRenderMesh*, uint8_t> m_pipelineMap;

	// A model has access to a collection of render meshes, each with a material attached.
	// To start with we'll make all of these arrays of equal size, but they don't have to
	// be. A merging of a material + a render mesh produces a pipeline state. The pipeline
	// state knows everything about where it's going to be rendered (it needs to)

	CrVector<CrRenderMeshHandle> m_renderMeshes;

	CrVector<uint8_t> m_renderMeshMaterialIndex;

	CrVector<CrMaterialHandle> m_materials;

	CrVector<CrArray<CrGraphicsPipelineHandle, CrMaterialPipelineVariant::Count>> m_pipelines;
};
