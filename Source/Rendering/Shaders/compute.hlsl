#ifndef COMPUTE_HLSL
#define COMPUTE_HLSL

RWBuffer<float4> ExampleDataBufferCompute;
RWTexture2D<float4> ExampleRWTextureCompute;

[numthreads(8, 8, 1)]
void main_cs()
{
	ExampleDataBufferCompute[0] = float4(0.0, 0.0, 1.0, 0.0);
	ExampleRWTextureCompute[uint2(0, 0)] = float4(1.0, 0.0, 0.0, 0.0);
}

#endif