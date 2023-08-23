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
float D_TrowbridgeReitzGGX(float3 N, float3 H, float alpha)
{
	float alpha2 = alpha * alpha;
	float NdotH = dot(N, H);
	float denominator = NdotH * NdotH * (alpha2 - 1.0) + 1.0;
	return alpha2 / (pi * denominator * denominator);
}

float G_Smith_Uncorrelated_GGX(float3 N, float3 L, float3 V, float alpha)
{
	float alpha2 = alpha * alpha;
	float NdotV = dot(N, V);
	float LdotV = dot(L, V);
	float numerator = 2.0 * NdotV * 2.0 * LdotV;
	float denominator = (NdotV + sqrt(alpha2 + (1.0 - alpha2) * NdotV * NdotV)) * (LdotV + sqrt(alpha2 + (1.0 - alpha2) * LdotV * LdotV));
	return numerator / denominator;
}

float3 F_Schlick(float3 F0, float3 V, float3 H)
{
	return F0 + (1.0 - F0) * pow5(1.0 - dot(V, H));
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
	return surface.diffuseAlbedoLinear * NdotL / pi;
}

float3 EvaluateSpecularBRDF(Surface surface, Light light)
{
	float3 N = surface.pixelNormalWorld;
	float3 V = surface.pixelNormalTangent;
	float3 L = light.directionPosition;
	
	float NdotL = saturate(dot(N, L));
	float NdotV = saturate(dot(N, V));
	
	float3 H = normalize(V + L);

	float D = D_TrowbridgeReitzGGX(surface.pixelNormalWorld, H, surface.alpha);
	
	float G = G_Smith_Uncorrelated_GGX(N, L, V, surface.alpha);
	
	float3 F = F_Schlick(surface.F0, V, H);
	
	float scalarTerm = D * G / (4.0 * NdotL * NdotV);
	
	return scalarTerm * F;
}

BSDF EvaluateBRDF(Surface surface, Light light)
{
	BSDF bsdf;

	bsdf.diffuse = EvaluateDiffuseBSDF(surface, light);

	bsdf.specular = EvaluateSpecularBRDF(surface, light);
	
	return bsdf;
}

#endif
