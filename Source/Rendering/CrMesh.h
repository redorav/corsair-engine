#pragma once

#include "Rendering/CrVisibility.h"

#include "Core/SmartPointers/CrSharedPtr.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

class CrMesh
{
public:

	const CrBoundingBox& GetBoundingBox() const { return m_boundingBox; }

public:

	CrBoundingBox m_boundingBox;

	CrVertexBufferSharedHandle m_vertexBuffer;

	CrIndexBufferSharedHandle m_indexBuffer;
};

using CrRenderMeshSharedHandle = CrSharedPtr<CrMesh>;