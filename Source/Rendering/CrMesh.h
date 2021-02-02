#pragma once

#include "Core/SmartPointers/CrSharedPtr.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

class CrMesh
{
public:
	CrVertexBufferSharedHandle m_vertexBuffer;
	CrIndexBufferSharedHandle m_indexBuffer;
};

using CrRenderMeshSharedHandle = CrSharedPtr<CrMesh>;