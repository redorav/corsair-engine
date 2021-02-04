#ifndef TRIANGLE_HLSL
#define TRIANGLE_HLSL

#include "brdf.hlsl"
#include "surface.hlsl"

struct VS_IN
{
	float3 pos;
	float4 normal;
	float4 tangent;
	float2 uv;
};

struct VS_OUT
{
	float4 hwPosition 	: SV_Position;
	float3 color 		: TEXCOORD0;
	float3 normal		: TEXCOORD1;
	float3 tangent		: TEXCOORD2;
	float2 uv			: TEXCOORD3;
};

struct Camera
{
	float4x4 view2Projection;
	float4x4 world2View;
};

cbuffer Camera : register(b1)
{
	Camera cb_Camera;
};

struct Color
{
	float4 color;
	float4 tint2;
};

cbuffer Color : register(b2)
{
	Color cb_Color;
};

struct Engine
{
	float4 color;
	float4 tint2;
};

cbuffer Engine : register(b0)
{
	Engine cb_Engine;
};

SamplerState AllPointClampSampler;
SamplerState AllLinearClampSampler;
SamplerState AllAnisotropicClampSampler;

SamplerState AllPointWrapSampler;
SamplerState AllLinearWrapSampler;
SamplerState AllAnisotropicWrapSampler;

Texture2D DiffuseTexture0;
Texture2D NormalTexture0;
Texture2D SpecularTexture0;
Texture2D EmissiveTexture0;
Texture2D DisplacementTexture0; 
Texture2D DisplacementTexture3;

RWTexture1D<float4> RWTexture_1;
RWTexture2D<float4> RWTexture_2;
RWTexture3D<float4> RWTexture_3;

struct SBStruct
{
	float a;
	int b;
};

StructuredBuffer<SBStruct> ExampleStructuredBuffer;
RWStructuredBuffer<SBStruct> ExampleRWStructuredBuffer;

Buffer<int> ExampleBuffer;
RWBuffer<int> ExampleRWBuffer;

ByteAddressBuffer ExampleByteBuffer;
RWByteAddressBuffer ExampleRWByteBuffer;

struct DynamicLight
{
	float4 positionRadius;
	float4 color;
};

cbuffer DynamicLight
{
	DynamicLight cb_DynamicLight;
};

#define NO_TRANSFORM2

VS_OUT main_vs(VS_IN IN)
{
	VS_OUT output;
	
	#if defined(NO_TRANSFORM)
	output.hwPosition = float4(IN.pos.xyz, 1);
	#else

	float4 localPosition = float4(IN.pos.xyz, 1);

	// TODO world matrix
	float4 viewPosition = mul(cb_Camera.world2View, localPosition);

	output.hwPosition = mul(cb_Camera.view2Projection, viewPosition);
	//output.hwPosition = mul(cb_CameraVS.world2View, float4(IN.pos.xyz, 1));
	#endif

	output.color = IN.normal.xyz; // TODO change
    output.uv = IN.uv;
	output.normal = IN.normal.xyz * 2.0 - 1.0;
	output.tangent = IN.tangent.xyz * 2.0 - 1.0;
	
	return output;
}

const float3 lightDirection = float3(0.0, -1.0, 0.0);

float4 main_ps(VS_OUT IN) : SV_Target0
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