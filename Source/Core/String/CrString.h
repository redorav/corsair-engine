#pragma once

#include "Core/CrMacros.h"

warnings_off
#include <EASTL/string.h>
warnings_on

using CrString = eastl::string;
using CrWString = eastl::wstring;
using CrStringNoInitialize = eastl::string::CtorDoNotInitialize;