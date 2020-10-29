#pragma once

#include <EASTL/shared_ptr.h>

#include "Core/CrCoreForwardDeclarations.h"

template <typename T, typename... Args>
inline CrSharedPtr<T> CrMakeShared(Args&&... args)
{
	return eastl::make_shared<T>(eastl::forward<Args>(args)...);
}