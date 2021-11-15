#pragma once

#if defined(__clang__)

#error Not implemented

#define optimize_off _Pragma("clang optimize off")
#define optimize_on _Pragma("clang optimize on")

#define warnings_off \
_Pragma("clang diagnostic push") \
_Pragma("clang diagnostic ignored \"-Weverything\"")

#define warnings_on _Pragma("clang diagnostic pop") \

#define pragma_pack_start
#define pragma_pack_end

#elif defined(__GNUC__)

#error Not implemented

#define optimize_off _Pragma("GCC push_options") _Pragma("GCC optimize(\"O0\")")
#define optimize_on _Pragma("GCC pop_options")

#define warnings_off
#define warnings_on

#define pragma_pack_start
#define pragma_pack_end

#elif defined(_MSC_VER)

#define optimize_off __pragma(optimize("", off));
#define optimize_on __pragma(optimize("", on));

#define warnings_off __pragma(warning(push, 0));
#define warnings_on __pragma(warning(pop));

#define pragma_pack_start __pragma( pack(push, 1) )
#define pragma_pack_end __pragma( pack(pop) )

#else

#define optimize_off
#define optimize_on

#endif

#define unused_parameter(x) (x)

template<int N> struct static_sizeof_dummy;
#define static_sizeof(T) static_sizeof_dummy<sizeof(T)>;