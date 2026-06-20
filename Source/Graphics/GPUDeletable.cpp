#include "Graphics/CrRendering_pch.h"

#include "Graphics/GPUDeletable.h"

#include "Graphics/IDevice.h"

namespace crgfx
{
	void CrGPUDeletable::CrRenderDeviceDeletionFunction(crgfx::IDevice* renderDevice, CrGPUDeletable* deletable)
	{
		renderDevice->AddToDeletionQueue(deletable);
	}
};