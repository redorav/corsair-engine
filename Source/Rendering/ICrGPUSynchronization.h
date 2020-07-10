#pragma once

#include "Core/CrMacros.h"
#include "Core/CrCoreForwardDeclarations.h"

// For synchronization primitives we'll use Vulkan nomenclature, as the naming varies between APIs and it's not always
// clear which is which. If unclear refer to the descriptions here for clarity

// Synchronization between host and device
class ICrGPUFence
{
public:
};

// Synchronization between queues
class ICrGPUSemaphore
{
public:
};