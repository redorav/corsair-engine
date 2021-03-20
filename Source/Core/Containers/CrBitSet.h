#pragma once

#include <EASTL/bitset.h>

template<size_t N, typename WordType = EASTL_BITSET_WORD_TYPE_DEFAULT>
using CrBitset = eastl::bitset<N, WordType>;