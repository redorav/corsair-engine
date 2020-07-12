#include "CrRendering_pch.h"

#include "CrGPUStackAllocator_d3d12.h"

#include "CrRenderDevice_d3d12.h"

#include "CrGPUBuffer.h"

#include "Core/Logging/ICrDebug.h"

#include <d3d12.h>

CrGPUStackAllocatorD3D12::CrGPUStackAllocatorD3D12(ICrRenderDevice* renderDevice) 
	: ICrGPUStackAllocator(renderDevice)
{
	
}

CrGPUStackAllocatorD3D12::~CrGPUStackAllocatorD3D12()
{
	
}

void CrGPUStackAllocatorD3D12::InitPS(size_t size)
{
	
}

void* CrGPUStackAllocatorD3D12::BeginPS()
{
	return m_hardwareBuffer->Lock();
}

void CrGPUStackAllocatorD3D12::EndPS()
{
	m_hardwareBuffer->Unlock();
}
