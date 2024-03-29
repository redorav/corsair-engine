#ifndef DEFERRED_LIGHTING_HLSL
#define DEFERRED_LIGHTING_HLSL

#include "GBuffer.hlsl"
#include "Common.hlsl"
#include "Lighting.hlsl"
#include "BSDF.hlsl"

struct DynamicLight
{
	float4 positionRadius;
	float4 colorIntensity;
};

cbuffer DynamicLight
{
	DynamicLight cb_DynamicLight;
};

struct LightComponents
{
	float intensity;
	float3 color;
};

Light ReadLight(DynamicLight dynamicLight)
{
	Light light = (Light) 0;
	
	light.directionPosition = dynamicLight.positionRadius.xyz;
	light.radius = dynamicLight.positionRadius.w;
	light.color = dynamicLight.colorIntensity.rgb;
	light.intensity = dynamicLight.colorIntensity.a;
	
	return light;
}

float3 DirectionalLighting(Surface surface, Light light)
{
	float3 diffuseLighting = EvaluateDiffuseBSDF(surface, light);
	
	float3 specularLighting = EvaluateSpecularBRDF(surface, light);
	
	return (diffuseLighting + specularLighting) * light.color;
}

float4 DeferredLightingPS(VSOutputFullscreen psInput) : SV_Target0
{
	uint2 screenPixel = (uint2) psInput.hwPosition.xy;
	
	Surface surface = DecodeGBufferSurface(screenPixel, psInput.screenUVClip);
	
	Light light = ReadLight(cb_DynamicLight);
	
	light.directionPosition.xyz = normalize(float3(1, 1, 1));

	float3 lighting = DirectionalLighting(surface, light);
	
	if (surface.rawDepth == 0.0)
	{
		// Temporarily clear to color when depth is far plane
		return float4(40.0, 87.0, 220.0, 255.0) / 255.0;
	}
	else
	{
		return float4(lighting, 1.0);
	}
}
#endif