#pragma once

#include "crstl/intrusive_ptr.h"

class ICrRenderDevice;

class CrGPUDeletable
{
public:

	void CrRenderDeviceDeletionFunction(ICrRenderDevice* renderDevice, CrGPUDeletable* deletable);

	CrGPUDeletable(ICrRenderDevice* renderDevice) : m_renderDevice(renderDevice) {}

	virtual ~CrGPUDeletable() {}

	ICrRenderDevice* m_renderDevice = nullptr;
};

class CrGPUAutoDeletable : public crstl::intrusive_ptr_interface_delete, public CrGPUDeletable
{
public:

	CrGPUAutoDeletable(ICrRenderDevice* renderDevice) : CrGPUDeletable(renderDevice) {}

	virtual ~CrGPUAutoDeletable() {}

	template<typename T>
	void intrusive_ptr_delete_callback()
	{
		CrRenderDeviceDeletionFunction(m_renderDevice, this);
	}
};