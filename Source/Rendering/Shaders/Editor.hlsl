#ifndef EDITOR_HLSL
#define EDITOR_HLSL

#include "Common.hlsl"
#include "GBuffer.hlsl"

Texture2D EditorSelectionTexture;

float4 EditorEdgeSelectionPS(VS_OUT_FULLSCREEN input) : SV_Target0
{
	// Sample in a radius
	float colorSamples = 0.0;
	float totalSamples = 0.0;

	for (int i = -1; i <= 1; ++i)
	{
		for (int j = -1; j <= 1; ++j)
		{
			float4 color = EditorSelectionTexture.Load(int3(input.hwPosition.xy + int2(i, j), 0));
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

Texture2D EditorInstanceIDTexture;
RWByteAddressBuffer EditorSelectedInstanceID;

struct MouseSelection
{
	int4 mouseCoordinates; // .xy mouse coordinates
};

cbuffer MouseSelection
{
	MouseSelection cb_MouseSelection;
};

[numthreads(1, 1, 1)]
void EditorResolveMouseSelectionCS(CS_IN input)
{
	int2 mouseCoordinates = cb_MouseSelection.mouseCoordinates.xy;

	float instanceID = (EditorInstanceIDTexture.Load(int3(mouseCoordinates, 0)).x + 0.5) * 65535.0;
	
	EditorSelectedInstanceID.Store(0, (uint) instanceID);
}

#endif