// Don't add pragma once here. It needs to be included more than once for the implementation

#include "Core/CrMacros.h"
#include "Core/Logging/ICrDebug.h"

warnings_off

#define VMA_DEBUG_LOG(format, ...) CrLog(format, __VA_ARGS__)

#include <vk_mem_alloc.h>
warnings_on