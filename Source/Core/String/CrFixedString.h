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