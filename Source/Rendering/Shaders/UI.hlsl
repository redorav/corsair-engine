#ifndef UI_HLSL
#define UI_HLSL

struct VS_IN_UI
{
	float2 position;
	float2 uv;
	float4 color;
};

struct VS_OUT_UI
{
	float4 hwPosition : SV_Position;
	float4 color : COLOR;
	float2 uv : UV;
};

struct UIData
{
	float4x4 projection;
};

cbuffer UIData : register(b0)
{
	UIData cbData;
}

Texture2D UITexture;
SamplerState UISampleState;

VS_OUT_UI main_vs(VS_IN_UI IN)
{
	VS_OUT_UI output = (VS_OUT_UI)0;
	output.uv = IN.uv;
	output.color = IN.color;
	output.hwPosition = mul(cbData.projection, float4(IN.position, 0.0, 1.0));
	return output;
}

float4 main_ps(VS_OUT_UI IN) : SV_Target0
{
	return UITexture.Sample(UISampleState, IN.uv) * IN.color;
}

#endif