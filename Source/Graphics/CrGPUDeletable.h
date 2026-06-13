#pragma once

#include "crstl/intrusive_ptr.h"

namespace crgfx
{
	class ICrRenderDevice;
};

class CrGPUDeletable
{
public:

	void CrRenderDeviceDeletionFunction(crgfx::ICrRenderDevice* renderDevice, CrGPUDeletable* deletable);

	CrGPUDeletable(crgfx::ICrRenderDevice* renderDevice) : m_renderDevice(renderDevice) {}

	virtual ~CrGPUDeletable() {}

	crgfx::ICrRenderDevice* m_renderDevice = nullptr;
};

class CrGPUAutoDeletable : public crstl::intrusive_ptr_interface_delete, public CrGPUDeletable
{
public:

	CrGPUAutoDeletable(crgfx::ICrRenderDevice* renderDevice) : CrGPUDeletable(renderDevice) {}

	virtual ~CrGPUAutoDeletable() {}

	template<typename T>
	void intrusive_ptr_delete_callback()
	{
		CrRenderDeviceDeletionFunction(m_renderDevice, this);
	}
};