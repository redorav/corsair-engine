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

float3 PackGBufferNormalOctahedral(float3 normal)
{
	const float2 normalOctahedral12bits = OctahedralEncode(normal) * 4095.0;
	const uint normalOctahedral24bits = uint(normalOctahedral12bits.x) | (uint(normalOctahedral12bits.y) << 12);
	return float3(uint3(normalOctahedral24bits, normalOctahedral24bits >> 8, normalOctahedral24bits >> 16) & 0xff) / 255.0;
}

float3 UnpackGBufferNormalOctahedral(float3 normalPacked)
{
	normalPacked *= 255.0;
	const uint normalOctahedral24Bits = uint(normalPacked.x) | (uint(normalPacked.y) << 8) | (uint(normalPacked.z) << 16);
	const float2 normalOctahedral12bits = float2(uint2(normalOctahedral24Bits, normalOctahedral24Bits >> 12) & 0xfff) / 4095.0;
	return OctahedralDecode(normalOctahedral12bits);
}

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
	
	float linearDepth = LinearizeDepth(gBuffer.rawDepth, CameraCB.linearization);

	surface.rawDepth            = gBuffer.rawDepth;
	surface.linearDepth         = linearDepth;

	surface.screenPixel         = screenPixel;
	surface.screenUV            = screenUVClip.xy;
	
	surface.positionView        = BackprojectView(screenUVClip.zw, linearDepth);
	surface.positionCameraWorld = mul(surface.positionView, (float3x3) CameraCB.view2WorldRotation);
	surface.viewView            = normalize(surface.positionView);
	surface.viewWorld           = -normalize(surface.positionCameraWorld);
	
	//surface.viewWorld           = GetViewVectorWorld();
	surface.diffuseAlbedoLinear = sRGBToLinear(gBuffer.albedoAO.rgb);
	surface.pixelNormalWorld    = UnpackGBufferNormalOctahedral(gBuffer.worldNormalRoughness.xyz);
	surface.roughness           = 0.4;
	surface.alpha               = surface.roughness * surface.roughness;

	return surface;
}

#endif