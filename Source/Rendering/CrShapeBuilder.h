#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrRenderMesh.h"

#include "Math/CrHlslppVectorFloat.h"

class CrShapeBuilder
{
public:

	// Quad
	struct CrQuadDescriptor
	{
		uint32_t subdivisionX = 0;
		uint32_t subdivisionY = 0;
		float4 color = float4(1.0f);
	};

	static CrRenderMeshSharedHandle CreateQuad(const CrQuadDescriptor& descriptor);
};