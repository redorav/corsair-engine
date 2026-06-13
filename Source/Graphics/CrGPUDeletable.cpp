#include "Graphics/CrRendering_pch.h"

#include "Graphics/CrGPUDeletable.h"

#include "Graphics/ICrRenderDevice.h"

void CrGPUDeletable::CrRenderDeviceDeletionFunction(crgfx::ICrRenderDevice* renderDevice, CrGPUDeletable* deletable)
{
	renderDevice->AddToDeletionQueue(deletable);
}