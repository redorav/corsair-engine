#pragma once

#include "stdint.h"

// CRSTL

namespace crstl
{
	class allocator;

	template<typename T, size_t N> class array;

	template<typename T, int N> class basic_fixed_string;

	template<size_t N, typename WordType> class bitset;

	template<typename T, size_t N> class fixed_vector;

	template<typename T> class intrusive_ptr;

	template <typename CharT, typename Allocator> class basic_string;
	typedef basic_string<char, allocator> string;
	typedef basic_string<wchar_t, allocator> wstring;

	template<typename T> class unique_ptr;

	template<typename T, typename Allocator> class vector;

	template<int SizeBytes, typename Return> class fixed_function;
};

// EASTL

namespace eastl
{
	class allocator;
	class dummy_allocator;

	template <typename T> struct less;

	template <typename T> struct hash;

	template <typename T> struct equal_to;

	template <bool bCondition, class ConditionIsTrueType, class ConditionIsFalseType> struct type_select;

	// Containers
	template <typename T, typename Allocator, unsigned kDequeSubarraySize> class deque;

	template <typename Key, typename T, typename Hash, typename Predicate, typename Allocator, bool bCacheHashCode> class hash_map;

	template <typename Key, typename T, typename Hash, typename Predicate, typename Allocator, bool bCacheHashCode> class hash_multimap;

	template <typename Value, typename Hash, typename Predicate, typename Allocator, bool bCacheHashCode> class hash_set;

	template <typename T1, typename T2> struct pair;

	template <typename Key, typename Compare, typename Allocator> class set;

	// Copied from <EASTL/string>. Keep here until CRSTL provides hash and replaces hashmap/set
	template <typename T, typename Allocator>
	struct hash<crstl::basic_string<T, Allocator>>
	{
		size_t operator()(const crstl::basic_string<T, Allocator>& x) const
		{
			const unsigned char* p = (const unsigned char*)x.c_str(); // To consider: limit p to at most 256 chars.
			unsigned int c, result = 2166136261U; // We implement an FNV-like string hash.
			while ((c = *p++) != 0) // Using '!=' disables compiler warnings.
				result = (result * 16777619) ^ c;
			return (size_t)result;
		}
	};

	template <typename T, int N>
	struct hash<crstl::basic_fixed_string<T, N>>
	{
		size_t operator()(const crstl::basic_fixed_string<T, N>& x) const
		{
			const unsigned char* p = (const unsigned char*)x.c_str(); // To consider: limit p to at most 256 chars.
			unsigned int c, result = 2166136261U; // We implement an FNV-like string hash.
			while ((c = *p++) != 0) // Using '!=' disables compiler warnings.
				result = (result * 16777619) ^ c;
			return (size_t)result;
		}
	};
};

// Containers
template<typename T, size_t N>
using CrArray = crstl::array<T, N>;

template<size_t N, typename WordType = size_t>
using CrBitset = crstl::bitset<N, WordType>;

// Copied from EASTL
#if !defined(__GNUC__) || (__GNUC__ >= 3) // GCC 2.x can't handle the declaration below.
#define DEQUE_DEFAULT_SUBARRAY_SIZE(T) ((sizeof(T) <= 4) ? 64 : ((sizeof(T) <= 8) ? 32 : ((sizeof(T) <= 16) ? 16 : ((sizeof(T) <= 32) ? 8 : 4))))
#else
#define DEQUE_DEFAULT_SUBARRAY_SIZE(T) 16
#endif

template<typename T>
using CrDeque = eastl::deque<T, eastl::allocator, DEQUE_DEFAULT_SUBARRAY_SIZE(T)>;

template<typename T, size_t N>
using CrFixedVector = crstl::fixed_vector<T, N>;

template<typename Key, typename S>
using CrHashMap = eastl::hash_map<Key, S, eastl::hash<Key>, eastl::equal_to<Key>, eastl::allocator, false>;

template<typename Key, typename S>
using CrHashMultiMap = eastl::hash_multimap<Key, S, eastl::hash<Key>, eastl::equal_to<Key>, eastl::allocator, false>;

template<typename Value>
using CrHashSet = eastl::hash_set<Value, eastl::hash<Value>, eastl::equal_to<Value>, eastl::allocator, false>;

template<typename T, typename S>
using CrPair = eastl::pair<T, S>;

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

class ICrFile;
using CrFileHandle = CrIntrusivePtr<ICrFile>;
using CrFileUniqueHandle = CrUniquePtr<ICrFile>;

class CrPath;

namespace cr { namespace Platform { enum T : uint32_t; } }