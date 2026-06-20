#pragma once

#include "Core/CrCoreForwardDeclarations.h"

#include "Graphics/GPUDeletable.h"

namespace crgfx
{
	// For synchronization primitives we'll use Vulkan nomenclature, as the naming varies between APIs and it's not always
	// clear which is which. If unclear refer to the descriptions here for clarity

	// Synchronization between host and device
	// Direction is device -> host
	// e.g. GPU signals and CPU waits
	// Vulkan: vkFence
	// D3D12: ID3D12Fence
	class IGPUFence : public CrGPUAutoDeletable
	{
	public:

		IGPUFence(crgfx::IDevice* renderDevice) : CrGPUAutoDeletable(renderDevice) {}

		virtual ~IGPUFence() {}
	};

	// Synchronization between queues
	// Direction is device -> device
	// e.g. Graphics queue signals, compute queue receives
	// e.g. Graphics queue signals, graphics queue receives
	// Vulkan: vkSemaphore
	// D3D12: ID3D12Fence
	class IGPUSemaphore : public CrGPUAutoDeletable
	{
	public:

		IGPUSemaphore(crgfx::IDevice* renderDevice) : CrGPUAutoDeletable(renderDevice) {}

		virtual ~IGPUSemaphore() {}
	};
}