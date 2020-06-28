#ifndef SURFACE_HLSL
#define SURFACE_HLSL

struct Surface
{
	// Geometric Properties
	float3 vertexNormalLocal;
	float3 vertexTangentLocal;
	float3 vertexBitangentLocal;

	float3 pixelNormalTangent;

	float3 pixelNormalWorld;

	// Material Properties
	float3 albedoSRGB;
	float3 albedoLinear;

	float roughness;
	float3 F0;
};

#endif
