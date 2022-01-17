#include "Common.hlsl"
#include "Ubershader.hlsl"
#include "Brdf.hlsl"
#include "Compute.hlsl"
#include "Imgui.hlsl"
#include "CopyTexture.hlsl"

float4 metadata() : SV_Target0
{	
    return float4(0.0, 0.0, 0.0, 0.0);
}
