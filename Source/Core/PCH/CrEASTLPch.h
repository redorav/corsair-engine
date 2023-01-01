#pragma once

#include "Core/CrMacros.h"

#include "Core/CRSTL/array.h"
#include <Core/CRSTL/bitset.h>
#include <Core/CRSTL/fixed_vector.h>

warnings_off
#include <EASTL/fixed_string.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include <EASTL/hash_map.h>
#include <EASTL/set.h>
#include <EASTL/deque.h>

#include <EASTL/unique_ptr.h>
warnings_on

#include <Core/CRSTL/intrusive_ptr.h>

// Explicit instantiations of commonly used templates
template class eastl::fixed_string<char, 32, false, eastl::allocator>;
template class eastl::fixed_string<char, 64, false, eastl::allocator>;
template class eastl::fixed_string<char, 128, false, eastl::allocator>;
template class eastl::fixed_string<char, 512, false, eastl::allocator>;