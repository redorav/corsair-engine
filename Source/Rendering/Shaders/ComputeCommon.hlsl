#ifndef COMPUTE_COMMON_HLSL
#define COMPUTE_COMMON_HLSL

// Compute

struct CS_IN
{
	uint3 groupId			: SV_GroupID;			// Index of the thread group a compute shader thread is executing in
	uint3 groupThreadId		: SV_GroupThreadID;		// Index for an individual thread within a thread group (local to the group)
	uint  groupIndex		: SV_GroupIndex;		// Flattened version of SV_GroupThreadID
	uint3 dispatchThreadId	: SV_DispatchThreadID;	// SV_GroupID * numthreads + SV_GroupThreadID - The global thread index
};

struct DispatchIndirectArguments
{
	uint dispatchX;
	uint dispatchY;
	uint dispatchZ;
};

struct DrawIndirectArguments
{
	uint vertexCount;
	uint instanceCount;
	uint firstVertex;
	uint firstInstance;
};

struct DrawInstancedIndirectArguments
{
	uint indexCount;
	uint instanceCount;
	uint firstIndex;
	uint vertexOffset;
	uint firstInstance;
};

#endif