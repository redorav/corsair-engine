#ifndef UTILITIES_HLSL
#define UTILITIES_HLSL

#define DispatchIndirectSize 3
#define DrawIndirectSize 4
#define DrawIndexedIndirectSize 5

void WriteDrawIndirectInstances(RWByteAddressBuffer indirectArgs, const int indirectDrawIndex, int instanceCount, int firstInstance)
{
	const int offset = indirectDrawIndex * DrawIndirectSize;
	indirectArgs.Store((offset + 1) * 4, instanceCount);
	indirectArgs.Store((offset + 3) * 4, firstInstance);
}

void WriteIndexedDrawIndirectInstances(RWByteAddressBuffer indirectArgs, int indirectDrawIndex, int instanceCount, int firstInstance)
{
	const int offset = indirectDrawIndex * DrawIndexedIndirectSize;
	indirectArgs.Store((offset + 1) * 4, instanceCount);
	indirectArgs.Store((offset + 4) * 4, firstInstance);
}

void WriteDrawIndirect(RWByteAddressBuffer indirectArgs, const int indirectDrawIndex, int vertexCount, int instanceCount, int firstVertex, int firstInstance)
{
	const int offset = indirectDrawIndex * DrawIndirectSize;
	indirectArgs.Store4(offset, uint4(vertexCount, instanceCount, firstVertex, firstInstance));
}

void WriteIndexedDrawIndirect(RWByteAddressBuffer indirectArgs, const int indirectDrawIndex, int indexCount, int instanceCount, int firstIndex, int vertexOffset, int firstInstance)
{
	const int offset = indirectDrawIndex * DrawIndexedIndirectSize;
	indirectArgs.Store4((offset + 0) * 4, indexCount);
	indirectArgs.Store((offset + 4) * 4, firstInstance);
}

void WriteDispatchIndirect(RWByteAddressBuffer indirectArgs, int indirectDrawIndex, int threadGroupCountX, int threadGroupCountY, int threadGroupCountZ)
{
	const int offset = indirectDrawIndex * DispatchIndirectSize;
	indirectArgs.Store((offset + 0) * 4, max(1, threadGroupCountX));
	indirectArgs.Store((offset + 1) * 4, max(1, threadGroupCountY));
	indirectArgs.Store((offset + 2) * 4, max(1, threadGroupCountZ));
}

#endif