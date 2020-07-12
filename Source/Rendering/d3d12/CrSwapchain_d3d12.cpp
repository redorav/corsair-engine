#include "CrRendering_pch.h"

#include "CrSwapchain_d3d12.h"
#include "CrCommandQueue_d3d12.h"
#include "CrTexture_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrGPUSynchronization_d3d12.h"
#include "CrD3D12.h"

#include "Core/Logging/ICrDebug.h"

CrSwapchainD3D12::CrSwapchainD3D12(ICrRenderDevice* renderDevice, const CrSwapchainDescriptor& swapchainDescriptor)
{
	renderDevice; swapchainDescriptor;
}

CrSwapchainD3D12::~CrSwapchainD3D12()
{

}

void CrSwapchainD3D12::CreatePS(ICrRenderDevice* renderDevice, const CrSwapchainDescriptor& swapchainDescriptor)
{

}

CrSwapchainResult CrSwapchainD3D12::AcquireNextImagePS(const ICrGPUSemaphore* signalSemaphore, uint64_t timeoutNanoseconds)
{
	return CrSwapchainResult::Success;
}

void CrSwapchainD3D12::PresentPS(ICrCommandQueue* queue, const ICrGPUSemaphore* waitSemaphore)
{
	
}