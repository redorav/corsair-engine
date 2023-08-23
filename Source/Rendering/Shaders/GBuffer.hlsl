#ifndef GBUFFER_HLSL
#define GBUFFER_HLSL

#include "Common.hlsl"
#include "Surface.hlsl"

Texture2D GBufferDepthTexture;
Texture2D GBufferAlbedoAOTexture;
Texture2D GBufferNormalsTexture;
Texture2D GBufferMaterialTexture;

struct GBuffer
{
	float4 albedoAO;
	float4 worldNormalRoughness;
	float4 material;
};

GBuffer ReadGBuffer(uint2 pixelCoords)
{
	GBuffer gBuffer;
	gBuffer.albedoAO             = GBufferAlbedoAOTexture.Load(int3(pixelCoords, 0));
	gBuffer.worldNormalRoughness = GBufferNormalsTexture.Load(int3(pixelCoords, 0));
	gBuffer.material             = GBufferMaterialTexture.Load(int3(pixelCoords, 0));
	return gBuffer;
}

Surface DecodeGBufferSurface(uint2 pixelCoords)
{
	GBuffer gBuffer = ReadGBuffer(pixelCoords);

	Surface surface = CreateDefaultSurface();
	
	surface.diffuseAlbedoSRGB   = gBuffer.albedoAO.rgb;
	surface.diffuseAlbedoLinear = surface.diffuseAlbedoSRGB * surface.diffuseAlbedoSRGB;
	surface.pixelNormalWorld    = gBuffer.worldNormalRoughness.xyz * 2.0 - 1.0;
	surface.roughness           = gBuffer.worldNormalRoughness.a;
	surface.alpha               = surface.roughness * surface.roughness;
	
	return surface;
}

#endif