#ifndef IMGUI_HLSL
#define IMGUI_HLSL

struct VSInputUI
{
	float2 position : POSITION;
	float2 uv       : TEXCOORD0;
	float4 color    : COLOR;
};

struct VSOutputUI
{
	float4 hwPosition : SV_Position;
	float4 color      : COLOR;
	float2 uv         : TEXCOORD0;
};

struct UIData
{
	float4x4 projection;
};

cbuffer UIData
{
	UIData cbData;
}

Texture2D UITexture;
SamplerState UISampleState;

VSOutputUI ImguiVS(VSInputUI vsInput)
{
	VSOutputUI output = (VSOutputUI)0;
	output.uv = vsInput.uv;
	output.color = vsInput.color;
	output.hwPosition = mul(cbData.projection, float4(vsInput.position, 0.0, 1.0));
	return output;
}

float4 ImguiPS(VSOutputUI psInput) : SV_Target0
{
	return UITexture.Sample(UISampleState, psInput.uv) * psInput.color;
}

#endif