#pragma once

#include "Core/SmartPointers/CrSharedPtr.h"

class CrVertexBufferCommon;
using CrVertexBufferSharedHandle = CrSharedPtr<CrVertexBufferCommon>;

class CrIndexBufferCommon;
using CrIndexBufferSharedHandle = CrSharedPtr<CrIndexBufferCommon>;

class CrMesh
{
public:
	CrVertexBufferSharedHandle m_vertexBuffer;
	CrIndexBufferSharedHandle m_indexBuffer;
};

using CrRenderMeshSharedHandle = CrSharedPtr<CrMesh>;