#ifndef EDITOR_HLSL
#define EDITOR_HLSL

#include "Common.hlsl"
#include "GBuffer.hlsl"

Texture2D SelectionTexture;

float4 EditorEdgeSelectionPS(VS_OUT_FULLSCREEN input) : SV_Target0
{
	// Sample in a radius
	float colorSamples = 0.0;
	float totalSamples = 0.0;

	for (int i = -1; i <= 1; ++i)
	{
		for (int j = -1; j <= 1; ++j)
		{
			float4 color = SelectionTexture.Load(int3(input.hwPosition.xy + int2(i, j), 0));
			bool isAnyColor = any(color);
			colorSamples += isAnyColor;
			totalSamples++;
		}
	}

	if (colorSamples > 0.0 && colorSamples != totalSamples)
	{
		float3 edgeColor = float3(30.0, 200.0, 255.0) / 255.0;
		return float4(edgeColor, 1.0);
	}
	else
	{
		return float4(0.0, 0.0, 0.0, 0.0);
	}
}

#endif