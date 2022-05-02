#ifndef SURFACE_HLSL
#define SURFACE_HLSL

struct Surface
{
	// Geometric Properties
	float3 vertexNormalWorld;
	float3 vertexTangentWorld;
	float3 vertexBitangentWorld;

	float3 pixelNormalTangent;

	float3 pixelNormalWorld;

	// Material Properties
	float3 albedoSRGB;
	float3 albedoLinear;

	float roughness;
	float3 F0;
};

#endif
