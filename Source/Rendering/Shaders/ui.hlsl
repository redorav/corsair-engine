struct VS_IN
{
	float2 position;
	float2 uv;
	uint color;
};

struct VS_OUT
{
	float4 hwPosition : SV_Position;
	float4 color;
};

VS_OUT main_vs(VS_IN IN)
{
	VS_OUT output = (VS_OUT)0;
	return output;
}

float4 main_ps(VS_OUT IN) : SV_Target0
{
	return float4(1.0, 0.0, 1.0, 1.0);
}