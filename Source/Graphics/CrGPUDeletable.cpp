#include "Graphics/CrRendering_pch.h"

#include "Graphics/CrGPUDeletable.h"

#include "Graphics/IDevice.h"

void CrGPUDeletable::CrRenderDeviceDeletionFunction(crgfx::IDevice* renderDevice, CrGPUDeletable* deletable)
{
	renderDevice->AddToDeletionQueue(deletable);
}