#pragma once

namespace CrMath
{
	constexpr static float Pi = 3.14159265359f;
	constexpr static float Rad2Deg = 180.0f / CrMath::Pi;
	constexpr static float Deg2Rad = CrMath::Pi / 180.0f;
};

template<typename T>
inline const T& CrMin(const T& a, const T& b) { return a < b ? a : b; }

template<typename T>
inline const T& CrMax(const T& a, const T& b) { return a > b ? a : b; }

template<typename T>
inline const T& CrClamp(const T& x, const T& min, const T& max)
{
	return CrMin(CrMax(x, min), max);
}