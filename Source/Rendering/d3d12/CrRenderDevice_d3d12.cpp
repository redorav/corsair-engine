#include "CrRendering_pch.h"

#include "CrRenderDevice_d3d12s.h"

CrRenderDeviceD3D12::CrRenderDeviceD3D12()
	: m_setupCmdBuffer(nullptr)
	, m_numCommandQueues(0)
{

}

CrRenderDeviceD3D12::~CrRenderDeviceD3D12()
{

}

void CrRenderDeviceD3D12::InitPS
(
	void* platformHandle, void* platformWindow
)
{
	
}

void CrRenderDeviceD3D12::WaitForFencePS(const CrGPUFenceD3D12* fence, uint64_t timeoutNanoseconds)
{
	
}

void CrRenderDeviceD3D12::ResetFencePS(const CrGPUFenceD3D12* fence)
{
	
}

void CrRenderDeviceD3D12::PresentPS()
{
	
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

ICrSwapchain* CrRenderDeviceD3D12::CreateSwapchainPS(uint32_t requestedWidth, uint32_t requestedHeight)
{
	return new CrSwapchainD3D12(this, requestedWidth, requestedHeight);
}

ICrTexture* CrRenderDeviceD3D12::CreateTexturePS(const CrTextureCreateParams& params)
{
	return new CrTextureD3D12(this, params);
}

ICrGPUStackAllocator* CrRenderDeviceD3D12::CreateGPUMemoryStreamPS()
{
	return new CrGPUStackAllocatorD3D12(this);
}

ICrHardwareGPUBuffer* CrRenderDeviceD3D12::CreateHardwareGPUBufferPS(const CrGPUBufferCreateParams& params)
{
	return new CrHardwareGPUBufferD3D12(this, params);
}

bool CrRenderDeviceD3D12::GetIsFeatureSupported(CrRenderingFeature::T feature)
{
	switch (feature)
	{
	}

	return false;
}
