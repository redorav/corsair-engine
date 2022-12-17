#ifndef UBERSHADER_HLSL
#define UBERSHADER_HLSL

#include "Common.hlsl"
#include "Brdf.hlsl"
#include "Surface.hlsl"

struct UbershaderPixelOutput
{
#if (EMaterialShaderVariant == EMaterialShaderVariant_GBuffer)
	float4 albedoTarget   : SV_Target0;
	float4 normalsTarget  : SV_Target1;
	float4 materialTarget : SV_Target2;
#else
    float4 finalTarget : SV_Target0;
#endif
};

VS_OUT UbershaderVS(VS_IN IN)
{
	VS_OUT output;
	
	float4x4 local2WorldMatrix = cb_Instance.local2World[IN.instanceID];
	
	#if defined(NO_TRANSFORM)
	output.hwPosition = float4(IN.pos.xyz, 1);
	#else

	float4 positionLocal = float4(IN.pos.xyz, 1);

	float4 positionWorld = mul(positionLocal, local2WorldMatrix);

	float4 positionView = mul(positionWorld, cb_Camera.world2View);

	output.hwPosition = mul(positionView, cb_Camera.view2Projection);
	#endif
	
	// Careful with this code and non-uniform scaling
	float3 vertexNormalLocal = IN.normal.xyz * 2.0 - 1.0;
	float3 vertexNormalWorld = mul(float4(vertexNormalLocal, 0.0), local2WorldMatrix).xyz;
	
	float3 vertexTangentLocal = IN.tangent.xyz * 2.0 - 1.0;
	float3 vertexTangentWorld = mul(float4(vertexTangentLocal, 0.0), local2WorldMatrix).xyz;

	output.color   = IN.color;
	output.uv      = IN.uv;
	output.normal  = vertexNormalWorld.xyz;
	output.tangent = vertexTangentWorld.xyz;
	
	return output;
}

const float3 lightDirection = float3(0.0, -1.0, 0.0);

UbershaderPixelOutput UbershaderPS(VS_OUT IN)
{
	UbershaderPixelOutput pixelOutput = (UbershaderPixelOutput)0;

	Surface surface;
	
	// Interpolants
	surface.vertexNormalWorld = IN.normal.xyz;
	surface.vertexTangentWorld = IN.tangent.xyz;
	surface.vertexBitangentWorld = cross(surface.vertexNormalWorld, surface.vertexTangentWorld);
	
	surface.vertexNormalWorld = normalize(surface.vertexNormalWorld);
	surface.vertexTangentWorld = normalize(surface.vertexTangentWorld);
	surface.vertexBitangentWorld = normalize(surface.vertexBitangentWorld);
	
	float3x3 tbn = float3x3(surface.vertexTangentWorld, surface.vertexBitangentWorld, surface.vertexNormalWorld);
	
	// Texture reads
	float4 diffuse0 = DiffuseTexture0.Sample(AllLinearWrapSampler, IN.uv.xy) * IN.color;
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

#if (EMaterialShaderVariant == EMaterialShaderVariant_Debug)

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

#elif (EMaterialShaderVariant == EMaterialShaderVariant_GBuffer)
	pixelOutput.albedoTarget  = float4(surface.albedoSRGB, 1.0);
	pixelOutput.normalsTarget = float4(surface.pixelNormalWorld * 0.5 + 0.5, surface.roughness);
	pixelOutput.materialTarget = float4(surface.F0, 1.0);
#else
    pixelOutput.finalTarget = float4(litSurface, 1.0 * cb_Color.tint.a);
#endif
	
	return pixelOutput;
}

#endif