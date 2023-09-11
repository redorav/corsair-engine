#ifndef GBUFFER_HLSL
#define GBUFFER_HLSL

#include "Common.hlsl"
#include "Depth.hlsl"
#include "Surface.hlsl"

Texture2D<float> GBufferDepthTexture;
Texture2D<float4> GBufferAlbedoAOTexture;
Texture2D<float4> GBufferNormalsTexture;
Texture2D<float4> GBufferMaterialTexture;

struct GBuffer
{
	float4 albedoAO;
	float4 worldNormalRoughness;
	float4 material;
	float rawDepth;
};

GBuffer ReadGBuffer(uint2 screenPixel)
{
	GBuffer gBuffer;
	gBuffer.albedoAO             = GBufferAlbedoAOTexture.Load(int3(screenPixel, 0));
	gBuffer.worldNormalRoughness = GBufferNormalsTexture.Load(int3(screenPixel, 0));
	gBuffer.material             = GBufferMaterialTexture.Load(int3(screenPixel, 0));
	gBuffer.rawDepth             = GBufferDepthTexture.Load(int3(screenPixel, 0));
	return gBuffer;
}

Surface DecodeGBufferSurface(uint2 screenPixel, float4 screenUVClip)
{
	GBuffer gBuffer = ReadGBuffer(screenPixel);

	Surface surface = CreateDefaultSurface();
	
	float linearDepth = LinearizeDepth(gBuffer.rawDepth, cb_Camera.linearization);

	surface.rawDepth            = gBuffer.rawDepth;
	surface.linearDepth         = linearDepth;

	surface.screenPixel         = screenPixel;
	surface.screenUV            = screenUVClip.xy;
	
	surface.positionView        = BackprojectView(screenUVClip.zw, linearDepth);
	surface.positionCameraWorld = mul(surface.positionView, (float3x3) cb_Camera.view2WorldRotation);
	surface.viewView            = normalize(surface.positionView);
	surface.viewWorld           = -normalize(surface.positionCameraWorld);
	
	//surface.viewWorld           = GetViewVectorWorld();
	surface.diffuseAlbedoSRGB   = gBuffer.albedoAO.rgb;
	surface.diffuseAlbedoLinear = surface.diffuseAlbedoSRGB * surface.diffuseAlbedoSRGB;
	surface.pixelNormalWorld    = normalize(gBuffer.worldNormalRoughness.xyz * 2.0 - 1.0);
	surface.roughness           = 0.2;
	surface.alpha               = surface.roughness * surface.roughness;

	return surface;
}

#endif