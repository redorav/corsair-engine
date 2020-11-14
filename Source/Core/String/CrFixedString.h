#pragma once

#include <EASTL/fixed_string.h>

// Take care to take the null terminator into account, i.e. a fixed string 
// of 16 has 15 usable characters
using CrFixedString16 = eastl::fixed_string<char, 16>;
using CrFixedString32 = eastl::fixed_string<char, 32>;
using CrFixedString64 = eastl::fixed_string<char, 64>;
using CrFixedString128 = eastl::fixed_string<char, 128>;
using CrFixedString256 = eastl::fixed_string<char, 256>;
using CrFixedString512 = eastl::fixed_string<char, 512>;
using CrFixedString1024 = eastl::fixed_string<char, 1024>;

template<int N>
using CrFixedString = eastl::fixed_string<char, N>;

using CrFixedWString16 = eastl::fixed_string<wchar_t, 16>;
using CrFixedWString32 = eastl::fixed_string<wchar_t, 32>;
using CrFixedWString64 = eastl::fixed_string<wchar_t, 64>;
using CrFixedWString128 = eastl::fixed_string<wchar_t, 128>;
using CrFixedWString256 = eastl::fixed_string<wchar_t, 256>;
using CrFixedWString512 = eastl::fixed_string<wchar_t, 512>;
using CrFixedWString1024 = eastl::fixed_string<wchar_t, 1024>;

template<int N>
using CrFixedWString = eastl::fixed_string<wchar_t, N>;