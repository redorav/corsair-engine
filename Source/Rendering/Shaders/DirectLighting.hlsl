#ifndef DIRECT_LIGHTING_HLSL
#define DIRECT_LIGHTING_HLSL

#include "GBuffer.hlsl"
#include "Common.hlsl"
#include "Lighting.hlsl"
#include "BSDF.hlsl"
#include "DirectLightingShared.hlsl"

struct DynamicLight
{
	float4 positionRadius;
	float4 colorIntensity;
};

cbuffer DynamicLightCB
{
	DynamicLight DynamicLightCB;
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
	
	Light light = ReadLight(DynamicLightCB);

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

struct GBufferDebug
{
	uint4 decodeOptions; // .x decode target
};

cbuffer GBufferDebug
{
	GBufferDebug cb_GBufferDebug;
};

float4 GBufferDebugPS(VSOutputFullscreen psInput) : SV_Target0
{
	int2 screenPixel = psInput.hwPosition.xy;

	Surface surface = DecodeGBufferSurface(screenPixel, psInput.screenUVClip);

	uint gbufferDebugMode = cb_GBufferDebug.decodeOptions.x;

	float3 debugColor = 0.0;

	if(gbufferDebugMode == GBufferDebugMode::Albedo)
	{
		debugColor = LinearToSRGB(surface.diffuseAlbedoLinear);
	}
		else if (gbufferDebugMode == GBufferDebugMode::WorldNormals)
	{
		debugColor = surface.pixelNormalWorld.xyz * 0.5 + 0.5;
	}
	else if (gbufferDebugMode == GBufferDebugMode::Roughness)
	{
		debugColor = surface.roughness.xxx;
	}
	else if(gbufferDebugMode == GBufferDebugMode::F0)
	{
		debugColor = surface.F0;
	}

	return float4(debugColor, 1.0);
}

#endif