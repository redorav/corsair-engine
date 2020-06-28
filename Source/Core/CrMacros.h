#pragma once

#define STRINGIFY(x)		#x


#define AUX_PATH2(x, y, z)		STRINGIFY(x/##y##_##z##.h)
#define AUX_PATH(x, y, z)		AUX_PATH2(x, y, z)

#define GRAPHICS_API_PATH(x)	AUX_PATH(GRAPHICS_API_DIR, x, GRAPHICS_API_SUFFIX)

#if defined(_MSC_VER)

#define optimize_off __pragma(optimize("", off));
#define optimize_on __pragma(optimize("", on));

#elif defined(__clang__)

#define optimize_off _Pragma("clang optimize off")
#define optimize_on _Pragma("clang optimize on")

#elif defined(__GNUC__)

#define optimize_off _Pragma("GCC push_options") _Pragma("GCC optimize(\"O0\")")
#define optimize_on _Pragma("GCC pop_options")

#else

#define optimize_off
#define optimize_on

#endif