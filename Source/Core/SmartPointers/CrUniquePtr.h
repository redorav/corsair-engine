#pragma once

#include <EASTL/unique_ptr.h>

#include "Core/CrCoreForwardDeclarations.h"

template <typename T, typename... Args>
inline CrUniquePtr<T> CrMakeUnique(Args&&... args)
{
	return eastl::make_unique<T>(eastl::forward<Args>(args)...);
}