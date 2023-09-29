#ifndef TONEMAPPING_HLSL
#define TONEMAPPING_HLSL

#include "ComputeCommon.hlsl"
#include "Common.hlsl"

Texture2D<float3> HDRInput;
RWTexture2D<float3> RWPostProcessedOutput;

[numthreads(8, 8, 1)]
void PostProcessingCS(CSInput csInput)
{
	RWPostProcessedOutput[csInput.dispatchThreadId.xy] = LinearToSRGB(HDRInput[csInput.dispatchThreadId.xy]);
}

#endif