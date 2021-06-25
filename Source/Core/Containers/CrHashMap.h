#pragma once

#include <EASTL/hash_map.h>

template<typename T, typename S>
using CrHashMap = eastl::hash_map<T, S>;

template<typename T, typename S>
using CrHashMultiMap = eastl::hash_multimap<T, S>;