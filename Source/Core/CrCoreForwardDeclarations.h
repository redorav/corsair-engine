#pragma once

// EASTL

namespace eastl
{
	class allocator;
	class dummy_allocator;
	
	template <bool bCondition, class ConditionIsTrueType, class ConditionIsFalseType> struct type_select;
	
	// Containers
	template<typename T, size_t N> struct array;

	template <size_t N, typename WordType> class bitset;

	template<typename T, typename Allocator> class vector;

	template<typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator> class fixed_vector;

	// Smart Pointers
	template <typename T> class shared_ptr;

	template <typename T> struct default_delete;

	template <typename T, typename Deleter> class unique_ptr;

	// Strings
	template <typename T, typename Allocator> class basic_string;
	typedef basic_string<char, allocator> string;

	template<int N, typename R>
	class fixed_function;
}

template<typename T>
using CrSharedPtr = eastl::shared_ptr<T>;

template <typename T, typename D = eastl::default_delete<T>>
using CrUniquePtr = eastl::unique_ptr<T, D>;

template<typename T, size_t N>
using CrArray = eastl::array<T, N>;

template<size_t N, typename WordType = EASTL_BITSET_WORD_TYPE_DEFAULT>
using CrBitset = eastl::bitset<N, WordType>;

template<typename T>
using CrVector = eastl::vector<T, eastl::allocator>;

template<typename T, size_t N>
using CrFixedVector = eastl::fixed_vector<T, N, false, eastl::dummy_allocator>;

using CrString = eastl::string;

template<int SIZE_IN_BYTES, typename R>
using CrFixedFunction = eastl::fixed_function<SIZE_IN_BYTES, R>;

class CrHash;

class ICrFile;
using CrFileSharedHandle = CrSharedPtr<ICrFile>;
using CrFileUniqueHandle = CrUniquePtr<ICrFile>;

class CrPath;