#pragma once

#include <cstdint>
#include <cfloat>

template<typename T>
struct CrNumericLimits
{
	static constexpr T max() { return T(0); }
	static constexpr T min() { return T(0); }
};

template<>
struct CrNumericLimits<uint8_t>
{
	static constexpr uint8_t max() { return UINT8_MAX; }
	static constexpr uint8_t min() { return 0; }
};

template<>
struct CrNumericLimits<int8_t>
{
	static constexpr int8_t max() { return INT8_MAX; }
	static constexpr int8_t min() { return INT8_MIN; }
};

template<>
struct CrNumericLimits<uint16_t>
{
	static constexpr uint16_t max() { return UINT16_MAX; }
	static constexpr uint16_t min() { return 0; }
};

template<>
struct CrNumericLimits<int16_t>
{
	static constexpr int16_t max() { return INT16_MAX; }
	static constexpr int16_t min() { return INT16_MIN; }
};

template<>
struct CrNumericLimits<uint32_t>
{
	static constexpr uint32_t max() { return UINT32_MAX; }
	static constexpr uint32_t min() { return 0; }
};

template<>
struct CrNumericLimits<int32_t>
{
	static constexpr int32_t max() { return INT32_MAX; }
	static constexpr int32_t min() { return INT32_MIN; }
};

template<>
struct CrNumericLimits<float>
{
	static constexpr float max() { return FLT_MAX; }
	static constexpr float min() { return FLT_MIN; }
};