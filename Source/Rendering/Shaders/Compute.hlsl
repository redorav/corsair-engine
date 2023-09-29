#ifndef COMPUTE_HLSL
#define COMPUTE_HLSL

#include "Common.hlsl"
#include "ComputeCommon.hlsl"
#include "IndirectRendering.hlsl"

Buffer<float4> ExampleDataBufferCompute;
RWBuffer<float4> ExampleRWDataBufferCompute;
RWTexture2D<float4> ExampleRWTextureCompute;
Texture2D ExampleTextureCompute;
Texture3D ExampleTexture3DCompute;
Texture2DArray ExampleTextureArrayCompute;

RWByteAddressBuffer ExampleIndirectBuffer;

RWStructuredBuffer<DispatchIndirectArguments> ExampleDispatchIndirect;

struct ComputeStruct
{
	float4 a;
	int4 b;
};

RWStructuredBuffer<ComputeStruct> ExampleRWStructuredBufferCompute;

StructuredBuffer<ComputeStruct> ExampleStructuredBufferCompute;

RWStructuredBuffer<int2> ExampleRWStructuredPerry;

[numthreads(8, 8, 1)]
void MainCS(CSInput csInput)
{
	float4 volumeTextureSample = ExampleTexture3DCompute.SampleLevel(AllLinearClampSampler, float3(0, 0, 0), 0);
	float4 textureArraySample  = ExampleTextureArrayCompute.SampleLevel(AllLinearClampSampler, float3(0, 0, 0), 0);

	//ExampleDataBufferCompute[0] = float4(0.0, 0.0, 1.0, 0.0);
	ExampleRWTextureCompute[csInput.groupThreadId.xy] = float4(csInput.groupThreadId.xy / 7.0, 0.0, 0.0) + volumeTextureSample + textureArraySample;

	ComputeStruct s;
	s.a = 3.0;
	s.b = 4;

	//ExampleRWStructuredBufferCompute[0] = ExampleStructuredBufferCompute[0];
}

[numthreads(1, 1, 1)]
void CreateIndirectArgumentsCS(CSInput csInput)
{
	//ExampleDispatchIndirect[0].dispatchX = 2;
	//ExampleDispatchIndirect[0].dispatchY = 2;
	//ExampleDispatchIndirect[0].dispatchZ = 2;
	WriteDispatchIndirect(ExampleIndirectBuffer, 0, 2, 2, 2);
}

#endif