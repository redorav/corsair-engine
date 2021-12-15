#ifndef COMPUTE_HLSL
#define COMPUTE_HLSL

#include "Common.hlsl"

Buffer<float4> ExampleDataBufferCompute;
RWBuffer<float4> ExampleRWDataBufferCompute;
RWTexture2D<float4> ExampleRWTextureCompute;
Texture2D ExampleTextureCompute;

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
	//ExampleDataBufferCompute[0] = float4(0.0, 0.0, 1.0, 0.0);
	ExampleRWTextureCompute[input.groupThreadId.xy] = float4(input.groupThreadId.xy / 7.0, 0.0, 0.0);

	ComputeStruct s;
	s.a = 3.0;
	s.b = 4;

	//ExampleRWStructuredBufferCompute[0] = ExampleStructuredBufferCompute[0];
}

#endif