#pragma once

#include "Rendering/ICrGPUSynchronization.h"
#include <d3d12.h>

class ICrRenderDevice;

class CrGPUFenceD3D12 final : public ICrGPUFence
{
public:

	CrGPUFenceD3D12(ICrRenderDevice* renderDevice);

	~CrGPUFenceD3D12();

private:

	ID3D12Fence* m_d3d12Fence;
};

class CrGPUSemaphoreD3D12 final : public ICrGPUSemaphore
{
public:

	CrGPUSemaphoreD3D12(ICrRenderDevice* renderDevice);

	~CrGPUSemaphoreD3D12();

private:

	ID3D12Fence* m_d3d12Fence;
};