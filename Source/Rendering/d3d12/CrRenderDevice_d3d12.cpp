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

CrRenderDeviceD3D12::CrRenderDeviceD3D12(const ICrRenderSystem* renderSystem) : ICrRenderDevice(renderSystem)
{
	unused_parameter(renderSystem);
}

CrRenderDeviceD3D12::~CrRenderDeviceD3D12()
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

ICrGPUSemaphore* CrRenderDeviceD3D12::CreateGPUSemaphorePS()
{
	return nullptr;
}

ICrGraphicsShader* CrRenderDeviceD3D12::CreateGraphicsShaderPS(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor) const
{
	unused_parameter(graphicsShaderDescriptor);
	return nullptr;
}

ICrComputeShader* CrRenderDeviceD3D12::CreateComputeShaderPS(const CrComputeShaderDescriptor& computeShaderDescriptor) const
{
	unused_parameter(computeShaderDescriptor);
	return nullptr;
}

ICrHardwareGPUBuffer* CrRenderDeviceD3D12::CreateHardwareGPUBufferPS(const CrHardwareGPUBufferDescriptor& params)
{
	return new CrHardwareGPUBufferD3D12(this, params);
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

ICrGraphicsPipeline* CrRenderDeviceD3D12::CreateGraphicsPipelinePS(const CrGraphicsPipelineDescriptor& pipelineDescriptor, const ICrGraphicsShader* graphicsShader, const CrVertexDescriptor& vertexDescriptor, const CrRenderPassDescriptor& renderPassDescriptor)
{
	unused_parameter(pipelineDescriptor);
	unused_parameter(graphicsShader);
	unused_parameter(vertexDescriptor);
	unused_parameter(renderPassDescriptor);
	return nullptr;
}

ICrComputePipeline* CrRenderDeviceD3D12::CreateComputePipelinePS(const CrComputePipelineDescriptor& pipelineDescriptor, const ICrComputeShader* computeShader)
{
	unused_parameter(pipelineDescriptor);
	unused_parameter(computeShader);
	return nullptr;
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

void CrRenderDeviceD3D12::WaitIdlePS()
{

}

bool CrRenderDeviceD3D12::GetIsFeatureSupported(CrRenderingFeature::T feature) const
{
	unused_parameter(feature);
	return false;
}