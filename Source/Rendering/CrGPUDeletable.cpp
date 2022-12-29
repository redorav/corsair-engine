#include "Rendering/CrRendering_pch.h"

#include "Rendering/CrGPUDeletable.h"

#include "Rendering/ICrRenderDevice.h"

void CrGPUDeletable::CrRenderDeviceDeletionFunction(ICrRenderDevice* renderDevice, CrGPUDeletable* deletable)
{
	renderDevice->AddToDeletionQueue(deletable);
}