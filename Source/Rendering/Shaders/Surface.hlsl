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

Surface CreateDefaultSurface()
{
	Surface surface;

	surface.vertexNormalWorld    = float3(0, 1, 0);
	surface.vertexTangentWorld   = float3(1, 0, 0);
	surface.vertexBitangentWorld = float3(0, 0, 1);
	surface.pixelNormalTangent   = float3(0, 0, 1);
	surface.pixelNormalWorld     = float3(0, 1, 0);

	surface.albedoSRGB = surface.albedoLinear = 1.0;

	surface.roughness = 1.0;
	surface.F0 = 0.02;

	return surface;
}

#endif
