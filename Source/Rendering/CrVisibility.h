#pragma once

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

class CrVisibility
{
public:

	static bool ObbProjection(const CrBoundingBox& obb, const float4x4& transform, const float4x4& projectionMatrix);
};