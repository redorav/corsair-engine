#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrRenderMesh.h"

class CrShapeBuilder
{
public:

	// Quad
	struct CrQuadDescriptor
	{
		uint32_t subdivisionX = 0;
		uint32_t subdivisionY = 0;
	};

	static CrRenderMeshSharedHandle CreateQuad(const CrQuadDescriptor& descriptor);
};