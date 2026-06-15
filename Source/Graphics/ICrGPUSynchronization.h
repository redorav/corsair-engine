#pragma once

#include "Core/CrCoreForwardDeclarations.h"

#include "Graphics/CrGPUDeletable.h"

namespace crgfx
{
	// For synchronization primitives we'll use Vulkan nomenclature, as the naming varies between APIs and it's not always
	// clear which is which. If unclear refer to the descriptions here for clarity

	// Synchronization between host and device
	// Direction is device -> host
	// e.g. GPU signals and CPU waits
	// Vulkan: vkFence
	// D3D12: ID3D12Fence
	class ICrGPUFence : public CrGPUAutoDeletable
	{
	public:

		ICrGPUFence(crgfx::IDevice* renderDevice) : CrGPUAutoDeletable(renderDevice) {}

		virtual ~ICrGPUFence() {}
	};

	// Synchronization between queues
	// Direction is device -> device
	// e.g. Graphics queue signals, compute queue receives
	// e.g. Graphics queue signals, graphics queue receives
	// Vulkan: vkSemaphore
	// D3D12: ID3D12Fence
	class ICrGPUSemaphore : public CrGPUAutoDeletable
	{
	public:

		ICrGPUSemaphore(crgfx::IDevice* renderDevice) : CrGPUAutoDeletable(renderDevice) {}

		virtual ~ICrGPUSemaphore() {}
	};
}