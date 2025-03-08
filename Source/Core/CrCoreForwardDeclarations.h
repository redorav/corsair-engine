#pragma once

#include "stdint.h"

#include "crstl/forward_declarations.h"

// Containers

template<typename Key, typename Value>
using CrHashMap = crstl::open_hashmap<Key, Value, crstl::hash<Key>, crstl::allocator>;

template<typename Key, typename Value, size_t NodeCount>
using CrFixedHashMap = crstl::fixed_open_hashmap<Key, Value, NodeCount, crstl::hash<Key>>;

template<typename Key, typename Value>
using CrHashMultiMap = crstl::open_multi_hashmap<Key, Value, crstl::hash<Key>, crstl::allocator>;

template<typename Key>
using CrHashSet = crstl::open_hashset<Key, crstl::hash<Key>, crstl::allocator>;

class CrHash;

using CrFixedPath = crstl::fixed_path512;

namespace cr { namespace Platform { enum T : uint32_t; } }