#ifndef COMMON_HLSL
#define COMMON_HLSL

// Graphics

struct VSInput
{
	float3 pos     : POSITION;
	float4 color   : COLOR;
	float4 normal  : NORMAL;
	float4 tangent : TANGENT;
	float2 uv      : TEXCOORD0;

	uint instanceID : SV_InstanceID;
};

struct VSOutput
{
	float4 hwPosition 	: SV_Position;
	float4 color 		: TEXCOORD0;
	float3 normal		: TEXCOORD1;
	float3 tangent		: TEXCOORD2;
	float2 uv			: TEXCOORD3;
};

struct VSInputFullscreen
{
    uint vertexId : SV_VertexID;
};

struct VSOutputFullscreen
{
    float4 hwPosition	: SV_Position;
    float2 uv			: TEXCOORD0;
};

struct Camera
{
	row_major float4x4 view2Projection;
	row_major float4x4 world2View;
	float4 projectionParams; // Use to linearize depth
	float4 screenResolution; // Resolution of final screen
	float4 worldPosition; // Position of the camera in world space
};

cbuffer Camera
{
	Camera cb_Camera;
};

struct Material
{
	float4 color;
	float4 tint;
};

cbuffer Material
{
	Material cb_Material;
};

struct Instance
{
	row_major float4x4 local2World[128];
};

cbuffer Instance
{
	Instance cb_Instance;
};

struct DebugShader
{
	float4 debugProperties; // .x Debug Shader Mode, .y Global Instance Id
	float4 debugColor;
};

cbuffer DebugShader
{
	DebugShader cb_DebugShader;
};

SamplerState AllPointClampSampler;
SamplerState AllLinearClampSampler;
SamplerState AllAnisotropicClampSampler;

SamplerState AllPointWrapSampler;
SamplerState AllLinearWrapSampler;
SamplerState AllAnisotropicWrapSampler;

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

struct DynamicLightCB
{
	float4 positionRadius;
	float4 colorIntensity;
};

cbuffer DynamicLightCB
{
	DynamicLightCB cb_DynamicLight;
};

static const float pi = 3.14159265359;

float min3(float a, float b, float c) { return min(min(a, b), c); }
float min4(float a, float b, float c, float d) { return min(min3(a, b, c), d); }

float max3(float a, float b, float c) { return max(a, max(b, c)); }
float max4(float a, float b, float c, float d) { return max(max3(a, b, c), d); }

float pow3(float x) { return x * x * x; }
float pow4(float x) { return x * x * x * x; }
float pow5(float x) { return x * x * x * x * x; }
float pow6(float x) { return x * x * x * x * x * x; }
float pow7(float x) { return x * x * x * x * x * x * x; }
float pow8(float x) { return x * x * x * x * x * x * x * x; }

#endif