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
	float4 hwPosition   : SV_Position;
	float4 color        : COLOR;
	float3 normal       : NORMAL;
	float3 tangent      : TANGENT;
	float2 uv           : TEXCOORD0;
};

struct VSInputFullscreen
{
    uint vertexId : SV_VertexID;
};

struct VSOutputFullscreen
{
    float4 hwPosition   : SV_Position;
	float4 screenUVClip : TEXCOORD0; // Screen position in UV space, position in [-1, 1]
};

struct CameraData
{
	row_major float4x4 view2Projection;
	row_major float4x4 world2View;
	row_major float3x4 view2WorldRotation;
	float4 linearization; // Used to linearize depth
	float4 backprojection;
	float4 screenResolution; // Resolution of final screen [.xy resolution, .zw 1.0 / resolution]
	float4 worldPosition; // .xyz Position of the camera in world space, .w Near Plane
};

cbuffer CameraCB
{
	CameraData CameraCB;
};

struct Material
{
	float4 color;
	float4 emissive;
};

cbuffer MaterialCB
{
	Material MaterialCB;
};

struct Instance
{
	row_major float4x4 local2World[128];
};

cbuffer InstanceCB
{
	Instance InstanceCB;
};

struct DebugShader
{
	float4 debugProperties; // .x Debug Shader Mode, .y Global Instance Id
	float4 debugColor;
};

cbuffer DebugShaderCB
{
	DebugShader DebugShaderCB;
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

// Returns a view vector in view space
float3 ComputeViewRay(float2 positionNdc, float nearPlane, float4x4 projection2ViewMatrix)
{
	float4 viewSpacePosition = mul(float4(positionNdc, nearPlane, 1.0), projection2ViewMatrix);
	return normalize(viewSpacePosition.xyz);
}

// Computes fragment position in world space
float3 BackprojectView(float2 screenClip, float linearDepth)
{
	float2 viewSpaceXY = screenClip * linearDepth;

	return float3(viewSpaceXY, linearDepth);
}

float3 LinearToSRGB(float3 xLinear)
{
	return pow(xLinear, 1.0f / 2.2f);
}

float3 sRGBToLinear(float3 xSRGB)
{
	return pow(xSRGB, 2.2f);
}

// Octahedral Encoding and Decoding

float2 OctahedralEncode(float3 n)
{
	n /= (abs(n.x) + abs(n.y) + abs(n.z));
	n.xy = select(n.z >= 0.0, n.xy, ((1.0 - abs(n.yx)) * select(n.xy >= 0.0, 1.0, -1.0)));
	return n.xy * 0.5 + 0.5;
}

// https://twitter.com/Stubbesaurus/status/937994790553227264
float3 OctahedralDecode(float2 f)
{
	f = f * 2.0 - 1.0;
	float3 n = float3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
	float t = saturate(-n.z);
	n.xy += select(n.xy >= 0.0, -t, t);
	return normalize(n);
}

#endif