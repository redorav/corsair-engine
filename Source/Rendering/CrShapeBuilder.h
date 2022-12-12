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

	static CrRenderMeshHandle CreateQuad(const CrQuadDescriptor& descriptor);

	struct CrCubeDescriptor
	{
		uint32_t subdivisionX = 0;
		uint32_t subdivisionY = 0;
		uint32_t subdivisionZ = 0;
		float4 color = float4(1.0f);
	};

	static CrRenderMeshHandle CreateCube(const CrCubeDescriptor& descriptor);

	struct CrSphereDescriptor
	{
		uint32_t subdivision = 0;
		float radius = 1.0f;
		float4 color = float4(1.0f);
	};

	static CrRenderMeshHandle CreateSphere(const CrSphereDescriptor& descriptor);
};