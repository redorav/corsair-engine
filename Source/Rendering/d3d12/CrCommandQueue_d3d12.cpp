#include "CrRendering_pch.h"

#include "CrRenderDevice_d3d12.h"
#include "CrCommandQueue_d3d12.h"
#include "CrCommandBuffer_d3d12.h"
#include "CrGPUSynchronization_d3d12.h"

#include "Core/Logging/ICrDebug.h"

CrCommandQueueD3D12::CrCommandQueueD3D12(ICrRenderDevice* renderDevice, CrCommandQueueType::T/* type*/) 
	: ICrCommandQueue(renderDevice)
{
	
}

ICrCommandBuffer* CrCommandQueueD3D12::CreateCommandBufferPS()
{
	return new CrCommandBufferD3D12(this);
}

void CrCommandQueueD3D12::SubmitCommandBufferPS(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence)
{
	unused_parameter(commandBuffer);
	unused_parameter(waitSemaphore);
	unused_parameter(signalSemaphore);
	unused_parameter(signalFence);
}

void CrCommandQueueD3D12::SignalFencePS(const ICrGPUFence* signalFence)
{
	unused_parameter(signalFence);
}

void CrCommandQueueD3D12::WaitIdlePS()
{
	
}