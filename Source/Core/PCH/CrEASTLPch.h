#pragma once

#include "Core/CrMacros.h"

#include "crstl/array.h"
#include "crstl/bitset.h"
#include "crstl/fixed_vector.h"
#include "crstl/fixed_string.h"
#include "crstl/unique_ptr.h"
#include "crstl/intrusive_ptr.h"
#include "crstl/string.h"
#include "crstl/vector.h"

warnings_off
#include <EASTL/set.h>
warnings_on

// Explicit instantiations of commonly used templates
template class crstl::basic_string<char, crstl::allocator>;
template class crstl::basic_fixed_string<char, 32>;
template class crstl::basic_fixed_string<char, 64>;
template class crstl::basic_fixed_string<char, 128>;
template class crstl::basic_fixed_string<char, 512>;