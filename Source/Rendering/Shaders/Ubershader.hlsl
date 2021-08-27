#ifndef UBERSHADER_HLSL
#define UBERSHADER_HLSL

#include "Common.hlsl"
#include "Brdf.hlsl"
#include "Surface.hlsl"

struct UbershaderPixelOutput
{
#if defined(GBUFFER_VARIANT)
	float4 albedoTarget : SV_Target0;
	float4 normalsTarget : SV_Target1;
#else
    float4 finalTarget : SV_Target0;
#endif
};

VS_OUT UbershaderVS(VS_IN IN)
{
	VS_OUT output;
	
	#if defined(NO_TRANSFORM)
	output.hwPosition = float4(IN.pos.xyz, 1);
	#else

	float4 localPosition = float4(IN.pos.xyz, 1);

	float4 worldPosition = mul(localPosition, cb_Instance.local2World[0]);

	float4 viewPosition = mul(worldPosition, cb_Camera.world2View);

	output.hwPosition = mul(viewPosition, cb_Camera.view2Projection);
	//output.hwPosition = mul(cb_CameraVS.world2View, float4(IN.pos.xyz, 1));
	#endif

	output.color   = IN.normal.xyz; // TODO change
	output.uv      = IN.uv;
	output.normal  = IN.normal.xyz * 2.0 - 1.0;
	output.tangent = IN.tangent.xyz * 2.0 - 1.0;
	
	return output;
}

const float3 lightDirection = float3(0.0, -1.0, 0.0);

UbershaderPixelOutput UbershaderPS(VS_OUT IN)
{
	UbershaderPixelOutput pixelOutput = (UbershaderPixelOutput)0;

	Surface surface;
	
	// Interpolants
	surface.vertexNormalLocal = IN.normal.xyz;
	surface.vertexTangentLocal = IN.tangent.xyz;
	surface.vertexBitangentLocal = cross(surface.vertexNormalLocal, surface.vertexTangentLocal);
	
	surface.vertexNormalLocal = normalize(surface.vertexNormalLocal);
	surface.vertexTangentLocal = normalize(surface.vertexTangentLocal);
	surface.vertexBitangentLocal = normalize(surface.vertexBitangentLocal);
	
	float3x3 tbn = float3x3(surface.vertexTangentLocal, surface.vertexBitangentLocal, surface.vertexNormalLocal);
	
	// Texture reads
	float4 diffuse0 = DiffuseTexture0.Sample(AllLinearWrapSampler, IN.uv.xy);
	float4 normal0 = NormalTexture0.Sample(AllLinearWrapSampler, IN.uv.xy);
	float4 spec0 = SpecularTexture0.Sample(AllLinearWrapSampler, IN.uv.xy);
	
	surface.roughness = 1.0 - spec0.a;
	surface.F0 = spec0.rgb;
	
	surface.pixelNormalTangent = normal0.xyz * 2.0 - 1.0;
	
	// Space transformation
	surface.pixelNormalWorld = mul(surface.pixelNormalTangent, tbn);
	
	surface.albedoSRGB   = diffuse0.rgb;
	surface.albedoLinear = surface.albedoSRGB * surface.albedoSRGB;

    float NdotL = dot(normalize(surface.pixelNormalWorld.xyz), -lightDirection);
	
    float3 litSurface = surface.albedoSRGB.xyz; // * NdotL; // +surface.F0 * pow(;
	
#if defined(GBUFFER_VARIANT)
	pixelOutput.albedoTarget  = float4(surface.albedoSRGB, 1.0);
	pixelOutput.normalsTarget = float4(surface.pixelNormalWorld * 0.5 + 0.5, 1.0);
#else
    pixelOutput.finalTarget = float4(litSurface, 1.0 * cb_Color.tint2.a);
#endif

	return pixelOutput;
}

#endif