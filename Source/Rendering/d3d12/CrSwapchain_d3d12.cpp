#include "CrRendering_pch.h"

#include "CrSwapchain_d3d12.h"
#include "CrCommandQueue_d3d12.h"
#include "CrTexture_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrGPUSynchronization_d3d12.h"
#include "CrD3D12.h"

#include "Core/Logging/ICrDebug.h"

CrSwapchainD3D12::CrSwapchainD3D12(ICrRenderDevice* renderDevice, const CrSwapchainDescriptor& swapchainDescriptor) : ICrSwapchain(renderDevice, swapchainDescriptor)
{

}

CrSwapchainD3D12::~CrSwapchainD3D12()
{

}

CrSwapchainResult CrSwapchainD3D12::AcquireNextImagePS(const ICrGPUSemaphore* signalSemaphore, uint64_t timeoutNanoseconds)
{
	unused_parameter(signalSemaphore);
	unused_parameter(timeoutNanoseconds);
	return CrSwapchainResult::Success;
}

void CrSwapchainD3D12::PresentPS(ICrCommandQueue* queue, const ICrGPUSemaphore* waitSemaphore)
{
	unused_parameter(queue);
	unused_parameter(waitSemaphore);
}