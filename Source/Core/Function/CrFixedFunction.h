#pragma once

#include <EASTL/fixed_function.h>

template<int SIZE_IN_BYTES, typename R>
using CrFixedFunction = eastl::fixed_function<SIZE_IN_BYTES, R>;
