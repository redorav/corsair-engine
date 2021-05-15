#pragma once

#include <EASTL/fixed_set.h>

template<typename T, size_t N>
using CrFixedSet = eastl::fixed_set<T, N, false>;