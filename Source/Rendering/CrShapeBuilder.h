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
		float3 scale = 1.0f;
		float4 color = 1.0f;
	};

	static CrRenderMeshHandle CreateQuad(const CrQuadDescriptor& descriptor);

	struct CrCubeDescriptor
	{
		uint32_t subdivisionX = 0;
		uint32_t subdivisionY = 0;
		uint32_t subdivisionZ = 0;
		float3 scale = 1.0f;
		float4 color = 1.0f;
	};

	static CrRenderMeshHandle CreateCube(const CrCubeDescriptor& descriptor);

	struct CrSphereDescriptor
	{
		uint32_t subdivision = 0;
		float3 scale = 1.0f;
		float4 color = 1.0f;
	};

	static CrRenderMeshHandle CreateSphere(const CrSphereDescriptor& descriptor);

	struct CrCylinderDescriptor
	{
		uint32_t subdivisionAxis = 0;
		uint32_t subdivisionLength = 0;
		float3 scale = 1.0f;
		float4 color = 1.0f;
	};

	static CrRenderMeshHandle CreateCylinder(const CrCylinderDescriptor& descriptor);
};