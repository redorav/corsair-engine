#pragma once

#include <cstdint>
#include <initializer_list>

#include <hlsl++.h>
using namespace hlslpp;

#include <half.hpp>
using half_float::half;
using namespace half_float::literal;

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

// Templated vector classes. Use mainly as simple structs.
template<class T, uint32_t N>
struct VectorT
{
	VectorT<T, N>() {}
	VectorT<T, N>(std::initializer_list<T> l) {}
	static_assert(N < 4, "Do not use this with sizes other than 2, 3 and 4! Specializations are below.");
};

template<class T>
struct VectorT<T, 4>
{
	VectorT<T, 4>() {}
	VectorT<T, 4>(std::initializer_list<T> l) : x(*l.begin()), y(*(l.begin() + 1)), z(*(l.begin() + 2)), w(*(l.begin() + 3)) {}
	VectorT<T, 4>(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
	T x, y, z, w;
};

template<class T>
struct VectorT<T, 3>
{
	VectorT<T, 3>() {}
	VectorT<T, 3>(std::initializer_list<T> l) : x(*l.begin()), y(*(l.begin() + 1)), z(*(l.begin() + 2)) {}
	VectorT<T, 3>(T x, T y, T z) : x(x), y(y), z(z) {}
	T x, y, z;
};

template<class T>
struct VectorT<T, 2>
{
	VectorT<T, 2>() {}
	VectorT<T, 2>(std::initializer_list<T> l) : x(*l.begin()), y(*(l.begin() + 1)) {}
	VectorT<T, 2>(T x, T y, T z, T w) : x(x), y(y) {}
	T x, y;
};