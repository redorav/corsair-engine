#pragma once

#include <EASTL/fixed_string.h>

// Take care to take the null terminator into account, i.e. a fixed string 
// of 16 has 15 usable characters

template<int N>
using CrFixedString = eastl::fixed_string<char, N, false>;

using CrFixedString8 = eastl::fixed_string<char, 8, false>;
using CrFixedString16 = eastl::fixed_string<char, 16, false>;
using CrFixedString32 = eastl::fixed_string<char, 32, false>;
using CrFixedString64 = eastl::fixed_string<char, 64, false>;
using CrFixedString128 = eastl::fixed_string<char, 128, false>;
using CrFixedString256 = eastl::fixed_string<char, 256, false>;
using CrFixedString512 = eastl::fixed_string<char, 512, false>;
using CrFixedString1024 = eastl::fixed_string<char, 1024, false>;

template<int N>
using CrFixedWString = eastl::fixed_string<wchar_t, N, false>;

using CrFixedWString8 = eastl::fixed_string<wchar_t, 8, false>;
using CrFixedWString16 = eastl::fixed_string<wchar_t, 16, false>;
using CrFixedWString32 = eastl::fixed_string<wchar_t, 32, false>;
using CrFixedWString64 = eastl::fixed_string<wchar_t, 64, false>;
using CrFixedWString128 = eastl::fixed_string<wchar_t, 128, false>;
using CrFixedWString256 = eastl::fixed_string<wchar_t, 256, false>;
using CrFixedWString512 = eastl::fixed_string<wchar_t, 512, false>;
using CrFixedWString1024 = eastl::fixed_string<wchar_t, 1024, false>;