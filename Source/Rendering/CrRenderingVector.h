#pragma once

#include "hlsl++.h"

// These vectors are simple structs that allow us to interface with shaders. hlsl++ is no good when it comes to this task
// for two reasons:
//
// 1) The hardware vectors are greater in size than the data they're supposed to represent. This is just the nature of SIMD
// 2) Operations on vectors such as multiply or add would cause issues with write-combine memory which is what we normally 
//    write to when using GPU-visible memory. For that reason it's better to just provide convenience structs that one does
//    not read from, even accidentally.
//
// We can still provide conversion operators such that it feels natural to store them using hlsl++

namespace cr3d
{
	struct float2
	{
		float2() = default;
		float2(hlslpp::float2 f) { hlslpp::store(f, &x); }
		float x, y;
	};

	struct float3
	{
		float3() = default;
		float3(hlslpp::float3 f) { hlslpp::store(f, &x); }
		float x, y, z;
	};

	struct float4
	{
		float4() = default;
		float4(hlslpp::float4 f) { hlslpp::store(f, &x); }
		float x, y, z, w;
	};

	struct float4x4
	{
		float4x4() = default;
		float4x4(hlslpp::float4x4 m) { hlslpp::store(m, &m00); }
		float m00, m01, m02, m03,
		      m10, m11, m12, m13,
		      m20, m21, m22, m23,
		      m30, m31, m32, m33;
	};

	struct int2
	{
		int32_t x, y;
	};

	struct int3
	{
		int32_t x, y, z;
	};

	struct int4
	{
		int32_t x, y, z, w;
	};

	struct uint2
	{
		uint32_t x, y;
	};

	struct uint3
	{
		uint32_t x, y, z;
	};

	struct uint4
	{
		uint32_t x, y, z, w;
	};
}