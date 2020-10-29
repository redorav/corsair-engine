#pragma once

// EASTL

namespace eastl
{
	class allocator;
	class dummy_allocator;
	
	template <bool bCondition, class ConditionIsTrueType, class ConditionIsFalseType> struct type_select;
	
	// Containers
	template<typename T, typename Allocator> class vector;

	template <typename T> class shared_ptr;

	template <typename T> struct default_delete;

	template <typename T, typename Deleter> class unique_ptr;

	// Strings
	template <typename T, typename Allocator> class basic_string;
	typedef basic_string<char, allocator> string;
}

template<typename T>
using CrSharedPtr = eastl::shared_ptr<T>;

template <typename T>
using CrUniquePtr = eastl::unique_ptr<T, eastl::default_delete<T>>;

template<typename T>
using CrVector = eastl::vector<T, eastl::allocator>;

using CrString = eastl::string;

// STD

namespace std
{
	namespace filesystem
	{
		class path;
	}
}

using CrPath = std::filesystem::path;

class CrHash;

class ICrFile;
using CrFileSharedHandle = CrSharedPtr<ICrFile>;
using CrFileUniqueHandle = CrUniquePtr<ICrFile>;