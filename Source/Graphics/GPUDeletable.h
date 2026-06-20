#pragma once

#include "crstl/intrusive_ptr.h"

namespace crgfx
{
	class IDevice;

	class GPUDeletable
	{
	public:

		void CrRenderDeviceDeletionFunction(crgfx::IDevice* renderDevice, GPUDeletable* deletable);

		GPUDeletable(crgfx::IDevice* renderDevice) : m_renderDevice(renderDevice) {}

		virtual ~GPUDeletable() {}

		crgfx::IDevice* m_renderDevice = nullptr;
	};

	class GPUAutoDeletable : public crstl::intrusive_ptr_interface_delete, public GPUDeletable
	{
	public:

		GPUAutoDeletable(crgfx::IDevice* renderDevice) : GPUDeletable(renderDevice) {}

		virtual ~GPUAutoDeletable() {}

		template<typename T>
		void intrusive_ptr_delete_callback()
		{
			CrRenderDeviceDeletionFunction(m_renderDevice, this);
		}
	};
};