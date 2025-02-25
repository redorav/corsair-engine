#pragma once

#include "stdint.h"

// CRSTL

namespace crstl
{
	class allocator;

	template<typename T, size_t N> class array;

	template<typename T, int N> class basic_fixed_string;
	typedef basic_fixed_string<char, 512> fixed_string512;

	template<size_t N, typename WordType> class bitset;

	template<typename T, typename Allocator, size_t ChunkSize> class deque;

	template<typename T, size_t N> class fixed_deque;

	template<typename T> struct hash;

	template<typename Key, typename T, size_t NodeCount,typename Hasher> class fixed_open_hashmap;

	template<typename Key, typename T, typename Hasher, typename Allocator> class open_hashmap;

	template<typename Key, typename Hasher, typename Allocator> class open_hashset;

	template<typename Key, typename T, typename Hasher, typename Allocator> class open_multi_hashmap;

	template<typename T, size_t N> class fixed_vector;

	template<typename T> class intrusive_ptr;

	template<typename T1, typename T2> class pair;

	template <typename CharT, typename Allocator> class basic_string;
	typedef basic_string<char, allocator> string;
	typedef basic_string<wchar_t, allocator> wstring;

	template<typename T> class unique_ptr;

	template<typename T, typename Allocator> class vector;

	template<int SizeBytes, typename Return> class fixed_function;

	template<typename StringInterface> class path_base;
	typedef path_base<fixed_string512> fixed_path512;

	class file;

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

// EASTL

namespace eastl
{
	class allocator;
	class dummy_allocator;

	template <typename T> struct less;
	template <typename Key, typename Compare, typename Allocator> class set;
};

// Containers
template<typename T, size_t N>
using CrArray = crstl::array<T, N>;

template<size_t N, typename WordType = size_t>
using CrBitset = crstl::bitset<N, WordType>;

template<typename T>
using CrDeque = crstl::deque<T, crstl::allocator, 16>;

template<typename T, size_t N>
using CrFixedDeque = crstl::fixed_deque<T, N>;

template<typename T, size_t N>
using CrFixedVector = crstl::fixed_vector<T, N>;

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

class ICrFile;
using CrFileHandle = CrIntrusivePtr<ICrFile>;
using CrFileUniqueHandle = CrUniquePtr<ICrFile>;

using CrFixedPath = crstl::fixed_path512;

namespace cr { namespace Platform { enum T : uint32_t; } }