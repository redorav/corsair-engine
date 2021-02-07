#include "CrRendering_pch.h"

#include "CrRenderDevice_d3d12.h"

#include "CrCommandQueue_d3d12.h"
#include "CrFramebuffer_d3d12.h"
#include "CrRenderPass_d3d12.h"
#include "CrSampler_d3d12.h"
#include "CrSwapchain_d3d12.h"
#include "CrTexture_d3d12.h"
#include "CrGPUBuffer_d3d12.h"
#include "CrGPUSynchronization_d3d12.h"

CrRenderDeviceD3D12::CrRenderDeviceD3D12()
{

}

CrRenderDeviceD3D12::~CrRenderDeviceD3D12()
{

}

void CrRenderDeviceD3D12::InitPS(const CrRenderDeviceDescriptor& renderDeviceDescriptor)
{
	unused_parameter(renderDeviceDescriptor);
//	IDXGIFactory4 dxgiFactory;
//
//	UINT createFactoryFlags = 0;
//#if defined(_DEBUG)
//	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
//#endif
//
//	HRESULT hResult = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory));
//
}

cr3d::GPUWaitResult CrRenderDeviceD3D12::WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds)
{
	unused_parameter(fence);
	unused_parameter(timeoutNanoseconds);
	return cr3d::GPUWaitResult::Success;
}

void CrRenderDeviceD3D12::ResetFencePS(const ICrGPUFence* fence)
{
	unused_parameter(fence);
}

ICrCommandQueue* CrRenderDeviceD3D12::CreateCommandQueuePS(CrCommandQueueType::T type)
{
	return new CrCommandQueueD3D12(this, type);
}

ICrFramebuffer* CrRenderDeviceD3D12::CreateFramebufferPS(const CrFramebufferCreateParams& params)
{
	return new CrFramebufferD3D12(this, params);
}

ICrGPUFence* CrRenderDeviceD3D12::CreateGPUFencePS()
{
	return new CrGPUFenceD3D12(this);
}

ICrRenderPass* CrRenderDeviceD3D12::CreateRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor)
{
	return new CrRenderPassD3D12(this, renderPassDescriptor);
}

ICrSampler* CrRenderDeviceD3D12::CreateSamplerPS(const CrSamplerDescriptor& descriptor)
{
	return new CrSamplerD3D12(this, descriptor);
}

ICrSwapchain* CrRenderDeviceD3D12::CreateSwapchainPS(const CrSwapchainDescriptor& swapchainDescriptor)
{
	return new CrSwapchainD3D12(this, swapchainDescriptor);
}

ICrTexture* CrRenderDeviceD3D12::CreateTexturePS(const CrTextureCreateParams& params)
{
	return new CrTextureD3D12(this, params);
}

ICrHardwareGPUBuffer* CrRenderDeviceD3D12::CreateHardwareGPUBufferPS(const CrHardwareGPUBufferDescriptor& params)
{
	return new CrHardwareGPUBufferD3D12(this, params);
}

bool CrRenderDeviceD3D12::GetIsFeatureSupported(CrRenderingFeature::T feature) const
{
	unused_parameter(feature);
	return false;
}
