#pragma once

#include "Rendering/CrVisibility.h"
#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/CrRenderMesh.h"

#include "crstl/array.h"
#include "crstl/fixed_vector.h"
#include "crstl/intrusive_ptr.h"
#include "crstl/open_hashmap.h"
#include "crstl/pair.h"
#include "crstl/vector.h"

struct CrRenderModelDescriptor
{
	uint32_t AddMaterial(const CrMaterialHandle& material);

	void AddRenderMesh(const CrRenderMeshHandle& renderMesh, uint8_t materialIndex);

	uint32_t GetMaterialCount() const { return (uint32_t)m_materials.size(); }

	uint32_t GetRenderMeshCount() const { return (uint32_t)m_meshes.size(); }

	const CrMaterialHandle& GetMaterial(uint32_t materialIndex) const { return m_materials[materialIndex]; }

	const CrRenderMeshHandle& GetRenderMesh(uint32_t renderMeshIndex) const { return m_meshes[renderMeshIndex]; }

	uint32_t GetRenderMeshMaterial(uint32_t renderMeshIndex) const { return m_materialIndices[renderMeshIndex]; }

private:

	crstl::fixed_vector<CrRenderMeshHandle, 256> m_meshes;

	crstl::fixed_vector<uint8_t, 256> m_materialIndices;

	crstl::fixed_vector<CrMaterialHandle, 256> m_materials;
};

class CrRenderModel final : public crstl::intrusive_ptr_interface_delete
{
public:

	CrRenderModel() = default;

	CrRenderModel(const CrRenderModelDescriptor& descriptor);

	const CrBoundingBox& GetBoundingBox() const { return m_boundingBox; }

	const crstl::pair<const CrRenderMeshHandle&, const CrMaterialHandle&> GetRenderMeshMaterial(uint32_t meshIndex) const
	{
		const CrRenderMeshHandle& renderMesh = m_renderMeshes[meshIndex];
		uint32_t materialIndex = m_renderMeshMaterialIndex[meshIndex];
		const CrMaterialHandle& material = m_materials[materialIndex];

		return crstl::pair<const CrRenderMeshHandle&, const CrMaterialHandle&>(renderMesh, material);
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

	crstl::open_hashmap<CrRenderMesh*, uint8_t> m_pipelineMap;

	// A model has access to a collection of render meshes, each with a material attached.
	// To start with we'll make all of these arrays of equal size, but they don't have to
	// be. A merging of a material + a render mesh produces a pipeline state. The pipeline
	// state knows everything about where it's going to be rendered (it needs to)

	crstl::vector<CrRenderMeshHandle> m_renderMeshes;

	crstl::vector<uint8_t> m_renderMeshMaterialIndex;

	crstl::vector<CrMaterialHandle> m_materials;

	crstl::vector<crstl::array<CrGraphicsPipelineHandle, CrMaterialPipelineVariant::Count>> m_pipelines;
};
