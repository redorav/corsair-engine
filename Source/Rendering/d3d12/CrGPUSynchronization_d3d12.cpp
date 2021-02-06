#include "CrRendering_pch.h"

#include "CrGPUSynchronization_d3d12.h"
#include "CrRenderDevice_d3d12.h"

CrGPUFenceD3D12::CrGPUFenceD3D12(ICrRenderDevice* renderDevice)
{
	unused_parameter(renderDevice);
}

CrGPUFenceD3D12::~CrGPUFenceD3D12()
{
	
}

CrGPUSemaphoreD3D12::CrGPUSemaphoreD3D12(ICrRenderDevice* renderDevice)
{
	unused_parameter(renderDevice);
}

CrGPUSemaphoreD3D12::~CrGPUSemaphoreD3D12()
{
	
}
