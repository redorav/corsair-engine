#pragma once

#include <EASTL/array.h>
#include <EASTL/bitset.h>
#include <EASTL/deque.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/hash_map.h>
#include <EASTL/set.h>
#include <EASTL/vector.h>

#include <EASTL/fixed_string.h>
#include <EASTL/string.h>

#include <EASTL/shared_ptr.h>
#include <EASTL/unique_ptr.h>

// Explicit instantiations of commonly used templates
template class eastl::fixed_string<char, 128, false, eastl::allocator>;
template class eastl::fixed_string<char, 512, false, eastl::allocator>;

template class eastl::fixed_string<char, 128, false, eastl::allocator>;
template class eastl::fixed_string<char, 512, false, eastl::allocator>;