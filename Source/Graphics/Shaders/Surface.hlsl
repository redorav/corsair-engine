#ifndef SURFACE_HLSL
#define SURFACE_HLSL

struct Surface
{
	//---------------------
	// Geometric Properties
	//---------------------

	uint2 screenPixel;
	float2 screenUV;
	
	float3 positionView;

	// Position in world space, relative to the camera position
	float3 positionCameraWorld;
	
	// View vector in view space
	float3 viewView;

	// View vector in world space
	float3 viewWorld;
	
	float rawDepth;
	float linearDepth;
	
	float3 vertexNormalWorld;
	float3 vertexTangentWorld;
	float3 vertexBitangentWorld;

	float3 pixelNormalTangent;

	float3 pixelNormalWorld;

	// Material Properties
	float3 diffuseAlbedoLinear;

	float roughness;
	float alpha;
	float3 F0;
};

Surface CreateDefaultSurface()
{
	Surface surface;

	surface.screenPixel = 0;
	surface.screenUV = 0.0;
	
	surface.positionCameraWorld = 0.0;
	surface.viewWorld = float3(0.0, 0.0, 1.0);
	
	surface.vertexNormalWorld    = float3(0, 1, 0);
	surface.vertexTangentWorld   = float3(1, 0, 0);
	surface.vertexBitangentWorld = float3(0, 0, 1);
	surface.pixelNormalTangent   = float3(0, 0, 1);
	surface.pixelNormalWorld     = float3(0, 1, 0);

	surface.diffuseAlbedoLinear = 1.0;

	surface.roughness = 1.0;
	surface.F0 = 0.02;

	surface.rawDepth = 0.0;
	surface.linearDepth = 0.0;

	return surface;
}

#endif
