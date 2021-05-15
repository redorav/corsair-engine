#pragma once

#include <EASTL/fixed_map.h>

template<typename T, typename U, size_t N>
using CrFixedMap = eastl::fixed_map<T, U, N, false>;