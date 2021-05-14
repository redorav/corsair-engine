#pragma once

#include "Rendering/CrVisibility.h"

#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/Containers/CrVector.h"
#include "Core/Containers/CrHashMap.h"

class CrRenderModel;
using CrRenderModelSharedHandle = CrSharedPtr<CrRenderModel>;

class CrMaterial;
using CrMaterialSharedHandle = CrSharedPtr<CrMaterial>;

class CrMesh;
using CrRenderMeshSharedHandle = CrSharedPtr<CrMesh>;

class CrRenderModel
{
public:

	const CrBoundingBox& GetBoundingBox() const { return m_boundingBox; }

	void ComputeBoundingBoxFromMeshes();

public:

	CrBoundingBox m_boundingBox;

	CrHashMap<CrMesh*, uint32_t> m_materialMap;

	CrVector<CrRenderMeshSharedHandle> m_renderMeshes;

	CrVector<CrMaterialSharedHandle> m_materials;
};
