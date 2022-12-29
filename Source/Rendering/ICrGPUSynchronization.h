#pragma once

#include "Core/CrCoreForwardDeclarations.h"

#include "Rendering/CrGPUDeletable.h"

// For synchronization primitives we'll use Vulkan nomenclature, as the naming varies between APIs and it's not always
// clear which is which. If unclear refer to the descriptions here for clarity

// Synchronization between host and device
// Direction is device -> host
// e.g. GPU signals and CPU waits
// Vulkan: vkFence
// D3D12: ID3D12Fence
class ICrGPUFence : public CrGPUDeletable
{
public:

	ICrGPUFence(ICrRenderDevice* renderDevice) : CrGPUDeletable(renderDevice) {}

	virtual ~ICrGPUFence() {}
};

// Synchronization between queues
// Direction is device -> device
// e.g. Graphics queue signals, compute queue receives
// e.g. Graphics queue signals, graphics queue receives
// Vulkan: vkSemaphore
// D3D12: ID3D12Fence
class ICrGPUSemaphore : public CrGPUDeletable
{
public:

	ICrGPUSemaphore(ICrRenderDevice* renderDevice) : CrGPUDeletable(renderDevice) {}

	virtual ~ICrGPUSemaphore() {}
};