#pragma once

struct CrBoundingBox
{
	float3 center;

	float3 extents; // Distance from the center to the corner
};

struct CrBoundingSphere
{
	float4 centerRadius;
};
