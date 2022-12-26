#pragma once

#include <Core/CRSTL/intrusive_ptr.h>

template<typename T>
using CrIntrusivePtr = crstl::intrusive_ptr<T>;

using CrIntrusivePtrInterface = crstl::intrusive_ptr_interface;