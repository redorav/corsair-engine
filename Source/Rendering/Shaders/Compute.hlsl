#ifndef COMPUTE_HLSL
#define COMPUTE_HLSL

#include "Common.hlsl"
#include "Utilities.hlsl"

Buffer<float4> ExampleDataBufferCompute;
RWBuffer<float4> ExampleRWDataBufferCompute;
RWTexture2D<float4> ExampleRWTextureCompute;
Texture2D ExampleTextureCompute;
Texture3D ExampleTexture3DCompute;
Texture2DArray ExampleTextureArrayCompute;

RWByteAddressBuffer ExampleIndirectBuffer;

struct DispatchIndirectArguments
{
	uint dispatchX;
	uint dispatchY;
	uint dispatchZ;
};

RWStructuredBuffer<DispatchIndirectArguments> ExampleDispatchIndirect;

struct ComputeStruct
{
	float4 a;
	int4 b;
};

RWStructuredBuffer<ComputeStruct> ExampleRWStructuredBufferCompute;

StructuredBuffer<ComputeStruct> ExampleStructuredBufferCompute;

RWStructuredBuffer<int2> ExampleRWStructuredPerry;

struct CS_IN
{
	uint3 groupId			: SV_GroupID;			// Index of the thread group a compute shader thread is executing in
	uint3 groupThreadId		: SV_GroupThreadID;		// Index for an individual thread within a thread group (local to the group)
	uint  groupIndex		: SV_GroupIndex;		// Flattened version of SV_GroupThreadID
	uint3 dispatchThreadId	: SV_DispatchThreadID;	// SV_GroupID * numthreads + SV_GroupThreadID - The global thread index
};

[numthreads(8, 8, 1)]
void MainCS(CS_IN input)
{
	float4 volumeTextureSample = ExampleTexture3DCompute.SampleLevel(AllLinearClampSampler, float3(0, 0, 0), 0);
	float4 textureArraySample  = ExampleTextureArrayCompute.SampleLevel(AllLinearClampSampler, float3(0, 0, 0), 0);

	//ExampleDataBufferCompute[0] = float4(0.0, 0.0, 1.0, 0.0);
	ExampleRWTextureCompute[input.groupThreadId.xy] = float4(input.groupThreadId.xy / 7.0, 0.0, 0.0) + volumeTextureSample + textureArraySample;

	ComputeStruct s;
	s.a = 3.0;
	s.b = 4;

	//ExampleRWStructuredBufferCompute[0] = ExampleStructuredBufferCompute[0];
}

[numthreads(1, 1, 1)]
void CreateIndirectArgumentsCS(CS_IN input)
{
	//ExampleDispatchIndirect[0].dispatchX = 2;
	//ExampleDispatchIndirect[0].dispatchY = 2;
	//ExampleDispatchIndirect[0].dispatchZ = 2;
	WriteDispatchIndirect(ExampleIndirectBuffer, 0, 2, 2, 2);
}

#endif