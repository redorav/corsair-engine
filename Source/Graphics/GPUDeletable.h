#pragma once

#include "crstl/intrusive_ptr.h"

namespace crgfx
{
	class IDevice;

	class CrGPUDeletable
	{
	public:

		void CrRenderDeviceDeletionFunction(crgfx::IDevice* renderDevice, CrGPUDeletable* deletable);

		CrGPUDeletable(crgfx::IDevice* renderDevice) : m_renderDevice(renderDevice) {}

		virtual ~CrGPUDeletable() {}

		crgfx::IDevice* m_renderDevice = nullptr;
	};

	class CrGPUAutoDeletable : public crstl::intrusive_ptr_interface_delete, public CrGPUDeletable
	{
	public:

		CrGPUAutoDeletable(crgfx::IDevice* renderDevice) : CrGPUDeletable(renderDevice) {}

		virtual ~CrGPUAutoDeletable() {}

		template<typename T>
		void intrusive_ptr_delete_callback()
		{
			CrRenderDeviceDeletionFunction(m_renderDevice, this);
		}
	};
};