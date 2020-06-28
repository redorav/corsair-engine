#pragma once

#include "CrMath.h"

class CrTransform
{
public:

	float3 position;
	float3 scale;
	quaternion rotation;

	float4x4 AsMatrix()
	{
		// float3x3(rotation); // TODO Transform from quaternion to float3x3
		// Multiply matrix by scale
		// Put position in appropriate row/column
		float4x4 dummy;
		return dummy;
	}
};