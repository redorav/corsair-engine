#ifndef BASIC_HLSL
#define BASIC_HLSL

// Basic shaders used for core operations like rendering lines, flat colors, fullscreen quads, etc

#include "Common.hlsl"

VS_OUT BasicVS(VS_IN input)
{
    VS_OUT output;
    
    #if defined(NO_TRANSFORM)
	output.hwPosition = float4(input.pos.xyz, 1);
	#else
	float4 localPosition = float4(input.pos.xyz, 1);
	float4 worldPosition = mul(localPosition, cb_Instance.local2World[input.instanceID]);
    float4 viewPosition = mul(worldPosition, cb_Camera.world2View);
    output.hwPosition = mul(viewPosition, cb_Camera.view2Projection);
    #endif
    
	output.color = input.color;
    
    return output;
}

float4 BasicPS(VS_OUT input) : SV_Target0
{
	return float4(input.color.rgb, 1.0);

}

#endif