#ifndef BRDF_HLSL
#define BRDF_HLSL

#include "Common.hlsl"
#include "Surface.hlsl"
#include "Lighting.hlsl"

RWTexture2D<float4> BRDFTexture;
Texture2D BRDF_GGX_Texture;

struct BSDF
{
	float3 diffuse;
	float3 specular;
};

// References
// http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html

// N is the normal
// H is the half vector
// alpha is the remapped roughnes (i.e. roughness^2)
float D_TrowbridgeReitzGGX(float NdotH, float alpha)
{
	float alpha2 = alpha * alpha;
	float denominator = NdotH * NdotH * (alpha2 - 1.0) + 1.0;
	return alpha2 / (pi * denominator * denominator);
}

float G_Smith_Uncorrelated_GGX(float NdotV, float NdotL, float alpha)
{
	float alpha2 = alpha * alpha;
	float numerator = 2.0 * NdotV * 2.0 * NdotL;
	float denominator = (NdotV + sqrt(alpha2 + (1.0 - alpha2) * NdotV * NdotV)) * (NdotL + sqrt(alpha2 + (1.0 - alpha2) * NdotL * NdotL));
	return numerator / denominator;
}

float3 F_Schlick(float3 F0, float VdotH)
{
	return F0 + (1.0 - F0) * pow5(1.0 - VdotH);
}

// Optimized version of the above
float3 DFG_Trowbridge_SmithU_Schlick(float NdotV, float NdotL, float NdotH, float VdotH, float alpha, float3 F0)
{
	float alpha2 = alpha * alpha;
	float Ddenominator = NdotH * NdotH * (alpha2 - 1.0) + 1.0;
	
	float dfgScalarNumerator = NdotL * alpha2; // D * F
	float dfgDenominator = 1.0;
	dfgDenominator *= (pi * Ddenominator * Ddenominator); // D
	dfgDenominator *= (NdotV + sqrt(alpha2 + (1.0 - alpha2) * NdotV * NdotV)) * (NdotL + sqrt(alpha2 + (1.0 - alpha2) * NdotL * NdotL)); // G

	float dfgScalar = dfgScalarNumerator / dfgDenominator;

	return (F0 + (1.0 - F0) * pow5(1.0 - VdotH)) * dfgScalar;
}

float F0FromIOR(float ior)
{
	float sqrf0 = (ior - 1.0) / (ior + 1.0);
	return sqrf0 * sqrf0;
}

float3 EvaluateDiffuseBSDF(Surface surface, Light light)
{
	float3 N = surface.pixelNormalWorld;
	float3 L = light.directionPosition;
	float NdotL = saturate(dot(N, L));
	return surface.diffuseAlbedoLinear * (NdotL / pi);
}

float3 EvaluateSpecularBRDF(Surface surface, Light light)
{
	float3 N = surface.pixelNormalWorld;
	float3 V = surface.viewWorld;
	float3 L = light.directionPosition;
	float3 H = normalize(V + L);

	float NdotL = saturate(dot(N, L));
	float NdotV = saturate(dot(N, V));
	float NdotH = saturate(dot(N, H));
	float VdotH = saturate(dot(V, H));

	return DFG_Trowbridge_SmithU_Schlick(NdotV, NdotL, NdotH, VdotH, surface.alpha, surface.F0);
}

BSDF EvaluateBRDF(Surface surface, Light light)
{
	BSDF bsdf;

	bsdf.diffuse = EvaluateDiffuseBSDF(surface, light);

	bsdf.specular = EvaluateSpecularBRDF(surface, light);
	
	return bsdf;
}

#endif
