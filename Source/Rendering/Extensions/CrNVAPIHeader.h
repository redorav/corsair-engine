#pragma once

#if defined(CR_PLATFORM_WINDOWS)

#define CR_NVAPI_SUPPORTED

// 5204: class has virtual functions, but its trivial destructor is not virtual; instances of objects derived from this class may not be destructed correctly
#pragma warning(disable:5204)
#include "nvapi.h"
#pragma warning(default:5204)

#endif