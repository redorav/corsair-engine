#pragma once

#include <EASTL/string.h>

using CrString = eastl::string;
using CrWString = eastl::wstring;
struct eastl::string::CtorDoNotInitialize;
using CrStringNoInitialize = eastl::string::CtorDoNotInitialize;