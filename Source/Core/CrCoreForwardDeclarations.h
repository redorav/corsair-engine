#pragma once

#include <cstdint>

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
	template<typename T, size_t N> struct array;

	template <size_t N, typename WordType> class bitset;

	template <typename T, typename Allocator, unsigned kDequeSubarraySize> class deque;

	template <typename Key, typename T, size_t nodeCount, bool bEnableOverflow, typename Compare, typename OverflowAllocator> class fixed_map;

	template <typename Key, size_t nodeCount, bool bEnableOverflow, typename Compare, typename OverflowAllocator> class fixed_set;

	template<typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator> class fixed_vector;

	template <typename Key, typename T, typename Hash, typename Predicate, typename Allocator, bool bCacheHashCode> class hash_map;

	template <typename Key, typename T, typename Hash, typename Predicate, typename Allocator, bool bCacheHashCode> class hash_multimap;

	template <typename Value, typename Hash, typename Predicate, typename Allocator, bool bCacheHashCode> class hash_set;

	template <typename T1, typename T2> struct pair;

	template <typename Key, typename Compare, typename Allocator> class set;

	template<typename T, typename Allocator> class vector;

	template <typename Key, typename T, typename Compare, typename Allocator, typename RandomAccessContainer> class vector_map;

	template <typename Key, typename Compare, typename Allocator, typename RandomAccessContainer> class vector_set;

	// Smart Pointers
	template <typename T> class shared_ptr;

	template <typename T> struct default_delete;

	template <typename T, typename Deleter> class unique_ptr;

	// Strings
	template <typename T, typename Allocator> class basic_string;
	typedef basic_string<char, allocator> string;

	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator> class fixed_string;

	// Keep here until EASTL natively provides this functionality. Copied from <EASTL/string>
	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	struct hash<fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>>
	{
		size_t operator()(const fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>& x) const
		{
			const unsigned char* p = (const unsigned char*)x.c_str(); // To consider: limit p to at most 256 chars.
			unsigned int c, result = 2166136261U; // We implement an FNV-like string hash.
			while ((c = *p++) != 0) // Using '!=' disables compiler warnings.
				result = (result * 16777619) ^ c;
			return (size_t)result;
		}
	};

	// Functional
	template<int N, typename R>
	class fixed_function;
}

// Containers
template<typename T, size_t N>
using CrArray = eastl::array<T, N>;

template<size_t N, typename WordType = size_t>
using CrBitset = eastl::bitset<N, WordType>;

// Copied from EASTL
#if !defined(__GNUC__) || (__GNUC__ >= 3) // GCC 2.x can't handle the declaration below.
#define DEQUE_DEFAULT_SUBARRAY_SIZE(T) ((sizeof(T) <= 4) ? 64 : ((sizeof(T) <= 8) ? 32 : ((sizeof(T) <= 16) ? 16 : ((sizeof(T) <= 32) ? 8 : 4))))
#else
#define DEQUE_DEFAULT_SUBARRAY_SIZE(T) 16
#endif

template<typename T>
using CrDeque = eastl::deque<T, eastl::allocator, DEQUE_DEFAULT_SUBARRAY_SIZE(T)>;

template<typename Key, typename U, size_t N>
using CrFixedMap = eastl::fixed_map<Key, U, N, false, eastl::less<Key>, eastl::dummy_allocator>;

template<typename Key, size_t N>
using CrFixedSet = eastl::fixed_set<Key, N, false, eastl::less<Key>, eastl::dummy_allocator>;

template<typename T, size_t N>
using CrFixedVector = eastl::fixed_vector<T, N, false, eastl::dummy_allocator>;

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
using CrVector = eastl::vector<T, eastl::allocator>;

template<typename Key, typename T>
using CrVectorMap = eastl::vector_map<Key, T, eastl::less<Key>, eastl::allocator, eastl::vector<eastl::pair<Key, T>, eastl::allocator>>;

template<typename Key>
using CrVectorSet = eastl::vector_set<Key, eastl::less<Key>, eastl::allocator, eastl::vector<Key, eastl::allocator>>;

// Smart Pointers
template<typename T>
using CrSharedPtr = eastl::shared_ptr<T>;

template <typename T, typename D = eastl::default_delete<T>>
using CrUniquePtr = eastl::unique_ptr<T, D>;

// Strings
using CrString = eastl::string;

// Take care to take the null terminator into account, i.e. a fixed string 
// of 16 has 15 usable characters

template<int N>
using CrFixedString = eastl::fixed_string<char, N, false, eastl::allocator>;

using CrFixedString8     = CrFixedString<8>;
using CrFixedString16    = CrFixedString<16>;
using CrFixedString32    = CrFixedString<32>;
using CrFixedString64    = CrFixedString<64>;
using CrFixedString128   = CrFixedString<128>;
using CrFixedString256   = CrFixedString<256>;
using CrFixedString512   = CrFixedString<512>;
using CrFixedString1024  = CrFixedString<1024>;

template<int N>
using CrFixedWString     = eastl::fixed_string<wchar_t, N, false, eastl::allocator>;

using CrFixedWString8    = CrFixedWString<8>;
using CrFixedWString16   = CrFixedWString<16>;
using CrFixedWString32   = CrFixedWString<32>;
using CrFixedWString64   = CrFixedWString<64>;
using CrFixedWString128  = CrFixedWString<128>;
using CrFixedWString256  = CrFixedWString<256>;
using CrFixedWString512  = CrFixedWString<512>;
using CrFixedWString1024 = CrFixedWString<1024>;

// Functional
template<int SIZE_IN_BYTES, typename R>
using CrFixedFunction = eastl::fixed_function<SIZE_IN_BYTES, R>;

class CrHash;

class ICrFile;
using CrFileSharedHandle = CrSharedPtr<ICrFile>;
using CrFileUniqueHandle = CrUniquePtr<ICrFile>;

class CrPath;

namespace cr { namespace Platform { enum T : uint32_t; } }