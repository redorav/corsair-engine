#ifndef COMMON_HLSL
#define COMMON_HLSL

struct VS_IN
{
	float3 pos : POSITION;
	float4 normal : NORMAL;
	float4 tangent : TANGENT;
	float2 uv : TEXCOORD;
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
	row_major float4x4 view2Projection;
	row_major float4x4 world2View;
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

#endif