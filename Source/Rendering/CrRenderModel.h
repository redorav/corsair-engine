#pragma once

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



	CrHashMap<CrMesh*, uint32_t> m_materialMap;

	CrVector<CrRenderMeshSharedHandle> m_renderMeshes;

	CrVector<CrMaterialSharedHandle> m_materials;
};