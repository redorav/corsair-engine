#pragma once

#include <initializer_list>

// Templated vector classes. Use mainly as simple structs.
template<class T, uint32_t N>
struct VectorT
{
	VectorT<T, N>() {}
	VectorT<T, N>(std::initializer_list<T>) {}
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
	VectorT<T, 2>(T x, T y) : x(x), y(y) {}
	T x, y;
};