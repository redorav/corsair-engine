#ifndef COPYTEXTURE_HLSL
#define COPYTEXTURE_HLSL

#include "Common.hlsl"

Texture2D CopyTexture;

VSOutputFullscreen CopyTextureVS(VSInputFullscreen input)
{
    VSOutputFullscreen vsOutput = (VSOutputFullscreen) 0;
    
    if (input.vertexId == 0) // Top left
    {
        vsOutput.hwPosition = float4(-1.0, 1.0, 1.0, 1.0);
        vsOutput.uv = float2(0.0, 0.0);
    }
    else if (input.vertexId == 1) // Top right
    {
        vsOutput.hwPosition = float4(3.0, 1.0, 1.0, 1.0);
        vsOutput.uv = float2(2.0, 0.0);
    }
    else if (input.vertexId == 2) // Bottom left
    {
        vsOutput.hwPosition = float4(-1.0, -3.0, 1.0, 1.0);
        vsOutput.uv = float2(0.0, 2.0);
    }
    
    return vsOutput;
}

float4 CopyTexturePS(VSOutputFullscreen psInput) : SV_Target0
{
#if defined(DEPTH)
    return CopyTexture.Sample(AllPointClampSampler, psInput.uv);
#else
    return CopyTexture.Sample(AllLinearClampSampler, psInput.uv);
#endif
}

#endif