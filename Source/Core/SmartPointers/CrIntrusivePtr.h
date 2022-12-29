#pragma once

#include <Core/CRSTL/intrusive_ptr.h>

template<typename T>
using CrIntrusivePtr = crstl::intrusive_ptr<T>;

template<typename T>
using CrIntrusivePtrInterface = crstl::intrusive_ptr_interface_delete<T>;

using CrIntrusivePtrInterfaceBase = crstl::intrusive_ptr_interface_base;