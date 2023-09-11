#ifndef TONEMAPPING_HLSL
#define TONEMAPPING_HLSL

#include "ComputeCommon.hlsl"

Texture2D<float3> HDRInput;
RWTexture2D<float3> RWPostProcessedOutput;

float3 LinearToSRGB(float3 linearInput)
{
	return pow(linearInput, 1.0 / 2.2);
}

[numthreads(8, 8, 1)]
void PostProcessingCS(CSInput csInput)
{
	RWPostProcessedOutput[csInput.dispatchThreadId.xy] = LinearToSRGB(HDRInput[csInput.dispatchThreadId.xy]);
}

#endif