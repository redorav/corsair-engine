#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"

#include "Math/CrHlslppVectorFloatType.h"

struct CrBoundingBox
{
	CrBoundingBox() {}

	CrBoundingBox(float3 center, float3 extents)
		: center(center)
		, extents(extents)
	{}

	float3 center;

	float3 extents; // Distance from the center to the corner
};

struct CrBoundingSphere
{
	float4 centerRadius;
};

typedef crstl::array<float4, 8> CrBoxVertices;

class CrVisibility
{
public:

	// Project a bounding box in world space using the view2Projection matrix, and return 8 corners
	static void ComputeObbProjection(const CrBoundingBox& obb, const float4x4& worldTransform, const float4x4& view2ProjectionMatrix, CrBoxVertices& projectedCorners);

	// Determine whether the following projected points (without perspective divide) are inside the [-w, w] bounds and [0, w] bounds for z
	static bool AreProjectedPointsOnScreen(const CrBoxVertices& projectedCorners);

	// Check whether obb intersects frustum
	static bool IsObbInFrustum(const CrBoundingBox& obb, const float4x4& transform, const float4x4& projectionMatrix);
};