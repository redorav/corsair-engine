#include "Graphics/CrRendering_pch.h"

#include "Graphics/GPUDeletable.h"

#include "Graphics/IDevice.h"

namespace crgfx
{
	void GPUDeletable::CrRenderDeviceDeletionFunction(crgfx::IDevice* renderDevice, GPUDeletable* deletable)
	{
		renderDevice->AddToDeletionQueue(deletable);
	}
};