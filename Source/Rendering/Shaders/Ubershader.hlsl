#ifndef UBERSHADER_HLSL
#define UBERSHADER_HLSL

#include "UbershaderResources.hlsl"
#include "Common.hlsl"
#include "Brdf.hlsl"
#include "Surface.hlsl"

struct UbershaderPixelOutput
{
#if defined(EMaterialShaderVariant_GBuffer)
	float4 albedoTarget   : SV_Target0;
	float4 normalsTarget  : SV_Target1;
	float4 materialTarget : SV_Target2;
#else
  float4 finalTarget : SV_Target0;
#endif
};

VSOutput UbershaderVS(VSInput vsInput)
{
	VSOutput vsOutput;
	
	float4x4 local2WorldMatrix = cb_Instance.local2World[vsInput.instanceID];
	
	#if defined(NO_TRANSFORM)
	output.hwPosition = float4(vsInput.pos.xyz, 1);
	#else

	float4 positionLocal = float4(vsInput.pos.xyz, 1);

	float4 positionWorld = mul(positionLocal, local2WorldMatrix);

	float4 positionView = mul(positionWorld, cb_Camera.world2View);

	vsOutput.hwPosition = mul(positionView, cb_Camera.view2Projection);
	#endif
	
	// Careful with this code and non-uniform scaling
	float3 vertexNormalLocal = vsInput.normal.xyz * 2.0 - 1.0;
	float3 vertexNormalWorld = mul(float4(vertexNormalLocal, 0.0), local2WorldMatrix).xyz;
	
	float3 vertexTangentLocal = vsInput.tangent.xyz * 2.0 - 1.0;
	float3 vertexTangentWorld = mul(float4(vertexTangentLocal, 0.0), local2WorldMatrix).xyz;

	vsOutput.color = vsInput.color;
	vsOutput.uv = vsInput.uv;
	vsOutput.normal = vertexNormalWorld.xyz;
	vsOutput.tangent = vertexTangentWorld.xyz;
	
	return vsOutput;
}

UbershaderPixelOutput UbershaderPS(VSOutput psInput)
{
	UbershaderPixelOutput pixelOutput = (UbershaderPixelOutput)0;

	Surface surface = CreateDefaultSurface();
	
	// Interpolants
	surface.vertexNormalWorld = psInput.normal.xyz;
	surface.vertexTangentWorld = psInput.tangent.xyz;
	surface.vertexBitangentWorld = cross(surface.vertexNormalWorld, surface.vertexTangentWorld);
	
	surface.vertexNormalWorld = normalize(surface.vertexNormalWorld);
	surface.vertexTangentWorld = normalize(surface.vertexTangentWorld);
	surface.vertexBitangentWorld = normalize(surface.vertexBitangentWorld);
	
	float3x3 tbn = float3x3(surface.vertexTangentWorld, surface.vertexBitangentWorld, surface.vertexNormalWorld);
	
#if defined(TEXTURED)

	float4 diffuse0 = DiffuseTexture0.Sample(AllLinearWrapSampler, psInput.uv.xy);
	float4 normal0 = NormalTexture0.Sample(AllLinearWrapSampler, psInput.uv.xy);
	float4 spec0 = SpecularTexture0.Sample(AllLinearWrapSampler, psInput.uv.xy);

	surface.roughness = 1.0 - spec0.a;
	surface.F0 = spec0.rgb;

	surface.pixelNormalTangent = normal0.xyz * 2.0 - 1.0;
	surface.pixelNormalWorld = mul(surface.pixelNormalTangent, tbn);

	surface.albedoSRGB = diffuse0.rgb;
	surface.albedoLinear = surface.albedoSRGB * surface.albedoSRGB;

#endif

    float3 litSurface = surface.albedoSRGB.xyz * psInput.color.rgb;

#if defined(EMaterialShaderVariant_Debug)

	float debugShaderMode = cb_DebugShader.debugProperties.x;

	const float DebugShaderModeInstanceID = 0;
	const float DebugShaderModeFlatColor = 1;

	if (debugShaderMode == DebugShaderModeInstanceID)
	{
		float normalizedInstanceId = cb_DebugShader.debugProperties.y / 65535.0;
		litSurface = float3(normalizedInstanceId, normalizedInstanceId, normalizedInstanceId);
	}
	else if (debugShaderMode == DebugShaderModeFlatColor)
	{
		litSurface = float3(1.0, 0.0, 0.0);
	}

	pixelOutput.finalTarget = float4(litSurface, cb_Color.tint.a);

#elif defined(EMaterialShaderVariant_GBuffer)
	pixelOutput.albedoTarget  = float4(surface.albedoSRGB, 1.0);
	pixelOutput.normalsTarget = float4(surface.pixelNormalWorld * 0.5 + 0.5, surface.roughness);
	pixelOutput.materialTarget = float4(surface.F0, 1.0);
#else
    pixelOutput.finalTarget = float4(litSurface, cb_Color.tint.a);
#endif
	
	return pixelOutput;
}

#endif