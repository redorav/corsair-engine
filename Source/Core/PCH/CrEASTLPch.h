#pragma once

#include "Core/CrMacros.h"

#include <crstl/array.h>
#include <crstl/bitset.h>
#include <crstl/fixed_vector.h>
#include <crstl/fixed_string.h>

warnings_off
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/hash_map.h>
#include <EASTL/set.h>
#include <EASTL/deque.h>

#include <EASTL/unique_ptr.h>
warnings_on

#include <crstl/intrusive_ptr.h>

// Explicit instantiations of commonly used templates
template class crstl::basic_fixed_string<char, 32>;
template class crstl::basic_fixed_string<char, 64>;
template class crstl::basic_fixed_string<char, 128>;
template class crstl::basic_fixed_string<char, 512>;