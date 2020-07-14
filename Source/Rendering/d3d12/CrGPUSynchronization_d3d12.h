#pragma once

#include "ICrGPUSynchronization.h"
#include <d3d12.h>

class ICrRenderDevice;

class CrGPUFenceD3D12 final : public ICrGPUFence
{
public:

	CrGPUFenceD3D12(ICrRenderDevice* renderDevice);

	~CrGPUFenceD3D12();

private:

};

class CrGPUSemaphoreD3D12 final : public ICrGPUSemaphore
{
public:

	CrGPUSemaphoreD3D12(ICrRenderDevice* renderDevice);

	~CrGPUSemaphoreD3D12();

private:

};