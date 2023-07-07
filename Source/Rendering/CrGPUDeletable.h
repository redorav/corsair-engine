#pragma once

#include "Core/SmartPointers/CrIntrusivePtr.h"

class ICrRenderDevice;

class CrGPUDeletable : public CrIntrusivePtrInterfaceBase
{
public:

	void CrRenderDeviceDeletionFunction(ICrRenderDevice* renderDevice, CrGPUDeletable* deletable);

	CrGPUDeletable(ICrRenderDevice* renderDevice) : m_renderDevice(renderDevice) {}

	virtual ~CrGPUDeletable() {}

	template<typename T>
	void intrusive_ptr_delete_callback()
	{
		CrRenderDeviceDeletionFunction(m_renderDevice, this);
	}

	ICrRenderDevice* m_renderDevice = nullptr;
};