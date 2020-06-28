#pragma once

#include <EASTL/unique_ptr.h>

template<typename T>
using CrUniquePtr = eastl::unique_ptr<T>;

template <typename T, typename... Args>
inline CrUniquePtr<T> CrMakeUnique(Args&&... args) { return eastl::make_unique<T>(eastl::forward<Args>(args)...); }