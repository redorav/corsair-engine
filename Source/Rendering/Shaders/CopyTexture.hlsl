#ifndef COPYTEXTURE_HLSL
#define COPYTEXTURE_HLSL

#include "Common.hlsl"

Texture2D CopyTexture;

VS_OUT_FULLSCREEN CopyTextureVS(VS_IN_FULLSCREEN input)
{
    VS_OUT_FULLSCREEN vsOut = (VS_OUT_FULLSCREEN) 0;
    
    if (input.vertexId == 0) // Top left
    {
        vsOut.hwPosition = float4(-1.0, 1.0, 1.0, 1.0);
        vsOut.uv = float2(0.0, 0.0);
    }
    else if (input.vertexId == 1) // Top right
    {
        vsOut.hwPosition = float4(3.0, 1.0, 1.0, 1.0);
        vsOut.uv = float2(2.0, 0.0);
    }
    else if (input.vertexId == 2) // Bottom left
    {
        vsOut.hwPosition = float4(-1.0, -3.0, 1.0, 1.0);
        vsOut.uv = float2(0.0, 2.0);
    }
    
    return vsOut;
}

float4 CopyTexturePS(VS_OUT_FULLSCREEN input) : SV_Target0
{
    return CopyTexture.Sample(AllLinearClampSampler, input.uv);
}

#endif