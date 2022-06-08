#include "Basic.hlsl"
#include "Brdf.hlsl"
#include "Common.hlsl"
#include "Compute.hlsl"
#include "CopyTexture.hlsl"
#include "Imgui.hlsl"
#include "GBuffer.hlsl"
#include "Editor.hlsl"

// Never include Ubershader.hlsl. It depends on defines that aren't present during metadata generation
// #include "Ubershader.hlsl"

float4 metadata() : SV_Target0
{	
    return float4(0.0, 0.0, 0.0, 0.0);
}
