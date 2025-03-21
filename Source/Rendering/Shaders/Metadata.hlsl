#include "BSDF.hlsl"
#include "Common.hlsl"
#include "Compute.hlsl"
#include "CopyTexture.hlsl"
#include "Depth.hlsl"
#include "DirectLighting.hlsl"
#include "Imgui.hlsl"
#include "GBuffer.hlsl"
#include "Editor.hlsl"
#include "PostProcessing.hlsl"

// Never include Ubershader.hlsl. It depends on defines that aren't present during metadata generation
// Instead we include UbershaderResources which has all the type information without the actual shader
// #include "Ubershader.hlsl"
#include "UbershaderResources.hlsl"

float4 metadata() : SV_Target0
{	
    return float4(0.0, 0.0, 0.0, 0.0);
}
