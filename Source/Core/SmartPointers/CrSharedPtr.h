#pragma once

#include <EASTL/shared_ptr.h>

template<typename T>
using CrSharedPtr = eastl::shared_ptr<T>;

template <typename T, typename... Args>
inline CrSharedPtr<T> CrMakeShared(Args&&... args) { return eastl::make_shared<T>(eastl::forward<Args>(args)...); }