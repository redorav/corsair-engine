#ifndef LIGHTING_HLSL
#define LIGHTING_HLSL

#include "GBuffer.hlsl"
#include "Common.hlsl"

struct Light
{
	float3 directionPosition;
	float radius;
	float3 radiance; // Includes intensity
};

Light ReadLight(DynamicLightCB dynamicLightCB)
{
	Light light = (Light) 0;
	
	light.directionPosition = dynamicLightCB.positionRadius.xyz;
	light.radius = dynamicLightCB.positionRadius.w;
	light.radiance = dynamicLightCB.radiance.rgb;
	
	return light;
}

float3 DirectionalLighting(Surface surface, Light light)
{
	float NdotL = dot(surface.pixelNormalWorld, light.directionPosition);
	
	return surface.albedoLinear * light.radiance * NdotL;
}

float4 DeferredLightingPS(VSOutputFullscreen psInput) : SV_Target0
{
	uint2 pixelCoords = (uint2)psInput.hwPosition.xy;
	
	Surface surface = DecodeGBufferSurface(pixelCoords);
	
	Light light = ReadLight(cb_DynamicLight);
	
	float3 lighting = DirectionalLighting(surface, light);
	
	return float4(lighting, 1.0);
}

#endif