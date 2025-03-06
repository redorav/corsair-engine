#pragma once

#include "stdint.h"

#include "crstl/forward_declarations.h"

// EASTL

namespace eastl
{
	class allocator;

	template <typename T> struct less;
	template <typename Key, typename Compare, typename Allocator> class set;
};

// Containers

template<typename Key, typename Value>
using CrHashMap = crstl::open_hashmap<Key, Value, crstl::hash<Key>, crstl::allocator>;

template<typename Key, typename Value, size_t NodeCount>
using CrFixedHashMap = crstl::fixed_open_hashmap<Key, Value, NodeCount, crstl::hash<Key>>;

template<typename Key, typename Value>
using CrHashMultiMap = crstl::open_multi_hashmap<Key, Value, crstl::hash<Key>, crstl::allocator>;

template<typename Key>
using CrHashSet = crstl::open_hashset<Key, crstl::hash<Key>, crstl::allocator>;

template<typename T, typename S>
using CrPair = crstl::pair<T, S>;

template<typename Key>
using CrSet = eastl::set<Key, eastl::less<Key>, eastl::allocator>;

template<typename T>
using CrVector = crstl::vector<T, crstl::allocator>;

// Smart Pointers

template <typename T>
using CrUniquePtr = crstl::unique_ptr<T>;

template<typename T>
using CrIntrusivePtr = crstl::intrusive_ptr<T>;

// Strings
using CrString = crstl::string;
using CrWString = crstl::wstring;

// Take care to take the null terminator into account, i.e. a fixed string 
// of 16 has 15 usable characters

template<int N>
using CrFixedString = crstl::basic_fixed_string<char, N>;

using CrFixedString8     = CrFixedString<8>;
using CrFixedString16    = CrFixedString<16>;
using CrFixedString32    = CrFixedString<32>;
using CrFixedString64    = CrFixedString<64>;
using CrFixedString128   = CrFixedString<128>;
using CrFixedString256   = CrFixedString<256>;
using CrFixedString512   = CrFixedString<512>;
using CrFixedString1024  = CrFixedString<1024>;
using CrFixedString2048  = CrFixedString<2048>;
using CrFixedString4096  = CrFixedString<4096>;

template<int N>
using CrFixedWString     = crstl::basic_fixed_string<wchar_t, N>;

using CrFixedWString8    = CrFixedWString<8>;
using CrFixedWString16   = CrFixedWString<16>;
using CrFixedWString32   = CrFixedWString<32>;
using CrFixedWString64   = CrFixedWString<64>;
using CrFixedWString128  = CrFixedWString<128>;
using CrFixedWString256  = CrFixedWString<256>;
using CrFixedWString512  = CrFixedWString<512>;
using CrFixedWString1024 = CrFixedWString<1024>;
using CrFixedWString2048 = CrFixedWString<2048>;
using CrFixedWString4096 = CrFixedWString<4096>;

// Functional
template<int SIZE_IN_BYTES, typename R>
using CrFixedFunction = crstl::fixed_function<SIZE_IN_BYTES, R>;

class CrHash;

using CrFixedPath = crstl::fixed_path512;

namespace cr { namespace Platform { enum T : uint32_t; } }