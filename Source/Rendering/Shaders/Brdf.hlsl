#ifndef BRDF_HLSL
#define BRDF_HLSL

RWTexture2D<float4> BRDFTexture;

StructuredBuffer<float4> ExampleStructuredBufferFloat;

Texture2D BRDF_GGX_Texture;

//struct Surface
//{
//	float3 albedo;

//	float3 F0;
//	float roughness;
//
//
//	// Geometric properties
//	float3 worldNormal;
//
//};

// There are several phenomena taking place at a surface, which we classify depending on the side of the surface they take place on
// Broadly speaking, when light interacts with a surface, it can either be reflected back, absorbed or transmitted to the other side.
//
// Within reflection, we typically have either diffuse or specular reflection, and within the diffuse reflection can have subsurface
// effects, where light is diffused and outscattered at a different point from where it entered.
//
// Within absorption, we typically have an rgb coefficient that expresses how much of the light was simply stored at the material
//
// Within transmission, we normally have several effects, either direct or diffuse transmission. Within that we can typically say that
// refraction is a subset of direct transmission (the other being the ray travelling in a straight line). Diffuse transmission is also
// typically called translucency.
//
// From here, we can start putting in the different parts:
//
//                            incidentLight = reflectedLight          + absorbedLight + transmittedLight
//                                            _______|_______________      |            ________|___________
//                            incidentLight = | diffuse + specular  | + absorbedLight + | direct + diffuse |
//
// In terms of how to model all of these effects, there are several examples here
//
// · Reflection
//
// Diffuse: Lambert BRDF
// Specular: GGX energy-conserving BRDF
// Subsurface Scattering: Screen Space Blur
//
// · Absorption
//
// Absorption: Tint Value
//
// · Transmission
//
// Direct Transmission: A transparency value that blends with framebuffer
// Refraction: Sample the framebuffer at an offset, based on normal
// Diffuse: Blur framebuffer, and sample that either directly or with an offset

struct BRDFReflection
{
	float3 diffuse;
	float3 specular;
};

struct BRDFTransmission
{
	float3 direct;
	float3 diffuse;
};

struct BRDFResult
{
	BRDFReflection   reflection;
	BRDFTransmission transmission;
	float3           absorption;
};

//BRDFResult EvaluateBRDF(Surface surface)
//{
//	BRDFResult brdf;
//
//	// Diffuse
//	{
//		//brdf.diffuse = 1.0 / pi;
//	}
//
//
//	return brdf;
//}

#endif
