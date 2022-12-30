#pragma once

#define crstl_constexpr constexpr

#define crstl_noexcept noexcept

#define crstl_nodiscard [[nodiscard]]

#if defined(__cpp_if_constexpr)

	#define crstl_constexpr_if(x) if constexpr(x)

#else

	#if defined(_MSC_VER)
		
		// warning C4127: conditional expression is constant
		// Disable because we always use these in a template context
		// Builds that don't support constexpr optimize them away
		#define crstl_constexpr_if(x) \
		__pragma(warning(push)) \
		__pragma(warning(disable : 4127)) \
		if(x) \
		__pragma(warning(pop))

	#else

		#define crstl_constexpr_if(x) if(x)

	#endif

#endif

#if defined(_MSC_VER)

#else

#endif