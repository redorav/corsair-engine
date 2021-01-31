#ifndef COMPUTE_HLSL
#define COMPUTE_HLSL

RWBuffer<float4> ExampleDataBufferCompute;

[numthreads(8, 8, 1)]
void main_cs()
{
	ExampleDataBufferCompute[0] = float4(0.0, 0.0, 1.0, 0.0);
}

#endif