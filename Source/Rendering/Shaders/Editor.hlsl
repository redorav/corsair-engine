#ifndef EDITOR_HLSL
#define EDITOR_HLSL

#include "Common.hlsl"
#include "ComputeCommon.hlsl"
#include "GBuffer.hlsl"
#include "Colors.hlsl"

Texture2D EditorSelectionTexture;

float4 EditorEdgeSelectionPS(VSOutputFullscreen psInput) : SV_Target0
{
	// Sample in a radius
	float colorSamples = 0.0;
	float totalSamples = 0.0;

	for (int i = -1; i <= 1; ++i)
	{
		for (int j = -1; j <= 1; ++j)
		{
			float4 color = EditorSelectionTexture.Load(int3(psInput.hwPosition.xy + int2(i, j), 0));
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

struct EditorGridCB
{
	float4 gridParams; // .x scale .y subdivisions
};

cbuffer EditorGridCB
{
	EditorGridCB cb_EditorGrid;
};

struct VSOutputEditorGrid
{
	float4 hwPosition    : SV_Position;
	float4 worldPosition : WORLD_POSITION;
	float4 viewPosition  : VIEW_POSITION;
	float4 color         : COLOR0; // Color of the grid, plus alpha for fading
	float4 gridUV        : TEXCOORD0;
};

VSOutputEditorGrid EditorGridVS(uint vertexId : SV_VertexID)
{
	VSOutputEditorGrid vsOutput = (VSOutputEditorGrid)0;
	
	float3 positionLocal = 0.0;
	float3 color = 0.0;
	float2 gridUV = 0.0;
	
	switch (vertexId)
	{
		case 0:
			positionLocal = float3(-1, 0, 1);
			color = Colors::Red;
			gridUV = float2(0.0, 1.0);
			break;
		case 1:
			positionLocal = float3(1, 0, 1);
			color = Colors::Green;
			gridUV = float2(1.0, 1.0);
			break;
		case 2:
			positionLocal = float3(1, 0, -1);
			color = Colors::Blue;
			gridUV = float2(1.0, 0.0);
			break;
		case 3:
			positionLocal = float3(-1, 0, 1);
			color = Colors::Magenta;
			gridUV = float2(0.0, 1.0);
			break;
		case 4:
			positionLocal = float3(1, 0, -1);
			color = Colors::Cyan;
			gridUV = float2(1.0, 0.0);
			break;
		case 5:
			positionLocal = float3(-1, 0, -1);
			color = Colors::Yellow;
			gridUV = float2(0.0, 0.0);
			break;
	}
	
	float4 positionWorld = float4(positionLocal.xyz * cb_EditorGrid.gridParams.x, 1);

	float4 positionView = mul(positionWorld, CameraCB.world2View);
	
	float4 positionHomogeneous = mul(positionView, CameraCB.view2Projection);

	vsOutput.hwPosition = positionHomogeneous;
	vsOutput.worldPosition = positionWorld;
	vsOutput.viewPosition = positionView;
	vsOutput.color.rgb = color;
	vsOutput.gridUV.xy = gridUV;

	return vsOutput;
}

static const float3 WidgetBlue  = float3(100.0, 100.0, 255.0) / 255.0;
static const float3 WidgetRed   = float3(255.0, 100.0, 100.0) / 255.0;

// Shader used to render the editor grid. We procedurally create the grid via constants
// and the grid itself will be alpha blended with the rest of the scene
// A wonderful explanation on how to create a procedural grid: https://madebyevan.com/shaders/grid/
float4 EditorGridPS(VSOutputEditorGrid psInput) : SV_Target0
{
	float2 coord = psInput.worldPosition.xz / cb_EditorGrid.gridParams.y;
	
	float3 worldPosition = psInput.worldPosition.xyz;
	
	float horizontalDistanceToCamera = length(CameraCB.worldPosition.xz - worldPosition.xz);
	
	float2 grid = abs(frac(coord - 0.5) - 0.5) / fwidth(coord);
	
	float3 color = Colors::White;
	
	// Tint the X axis red
	if (abs(worldPosition.z) < 0.5)
	{
		if (grid.y <= grid.x)
		{
			color = WidgetRed;
			grid *= 0.5;
		}
	}
	
	// Tint the Z axis blue
	if (abs(worldPosition.x) < 0.5)
	{
		if (grid.x <= grid.y)
		{
			color = WidgetBlue;
			grid *= 0.5;
		}
	}
	
	float gridLine = min(grid.x, grid.y);

	float alpha = 1.0 - min(gridLine, 1.0);

	float alphaFade = 1.0 - saturate(horizontalDistanceToCamera / 400.0);
	
	alpha *= pow(alphaFade, 8.0);
	
	// Gamma correction
	alpha = pow(alpha, 1.0 / 2.2);
	
	return float4(color, alpha);
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
void EditorMouseSelectionResolveCS(CSInput csInput)
{
	int2 mouseCoordinates = cb_MouseSelection.mouseCoordinates.xy;

	float instanceID = EditorInstanceIDTexture.Load(int3(mouseCoordinates, 0)).x * 65535.0 + 0.5;
	
	EditorSelectedInstanceID.Store(0, (uint) instanceID);
}

#endif