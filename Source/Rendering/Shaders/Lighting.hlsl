#ifndef LIGHTING_HLSL
#define LIGHTING_HLSL

struct Light
{
	float3 directionPosition;
	float radius;
	float intensity;
	float3 color;
};

#endif