#ifndef TRIANGLE_HLSL
#define TRIANGLE_HLSL

#include "Common.hlsl"
#include "Brdf.hlsl"
#include "Surface.hlsl"

#define NO_TRANSFORM2

VS_OUT BasicVS(VS_IN IN)
{
	VS_OUT output;
	
	#if defined(NO_TRANSFORM)
	output.hwPosition = float4(IN.pos.xyz, 1);
	#else

	float4 localPosition = float4(IN.pos.xyz, 1);

	// TODO world matrix
	float4 viewPosition = mul(localPosition, cb_Camera.world2View);

	output.hwPosition = mul(viewPosition, cb_Camera.view2Projection);
	//output.hwPosition = mul(cb_CameraVS.world2View, float4(IN.pos.xyz, 1));
	#endif

	output.color = IN.normal.xyz; // TODO change
    output.uv = IN.uv;
	output.normal = IN.normal.xyz * 2.0 - 1.0;
	output.tangent = IN.tangent.xyz * 2.0 - 1.0;
	
	return output;
}

const float3 lightDirection = float3(0.0, -1.0, 0.0);

float4 BasicPS(VS_OUT IN) : SV_Target0
{
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
	
	// Lighting
	surface.albedoSRGB = diffuse0.rgb; // TODO Make sure format is specified as sRGB so decoding is automatic
	surface.albedoLinear = surface.albedoSRGB * surface.albedoSRGB;
	float NdotL = dot(normalize(surface.pixelNormalWorld.xyz), -lightDirection);
	//float NdotV
	
	float3 litSurface = surface.albedoLinear.xyz;// * NdotL; // +surface.F0 * pow(;
	
	return float4(sqrt(litSurface), 1.0 * cb_Color.tint2.a);
}

#endif