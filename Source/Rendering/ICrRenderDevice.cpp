#include "CrRendering_pch.h"

#include "ICrRenderDevice.h"
#include "ICrCommandQueue.h"
#include "ICrSwapchain.h"
#include "ICrSampler.h"
#include "ICrTexture.h"
#include "ICrFramebuffer.h"
#include "ICrRenderPass.h"
#include "ICrShader.h"
#include "ICrPipeline.h"
#include "CrGPUBuffer.h"
#include "CrGPUStackAllocator.h"

#include "Core/CrMacros.h"

ICrRenderDevice::ICrRenderDevice(const ICrRenderSystem* renderSystem) : m_renderSystem(renderSystem)
{
	
}

ICrRenderDevice::~ICrRenderDevice()
{

}

CrCommandQueueSharedHandle ICrRenderDevice::CreateCommandQueue(CrCommandQueueType::T type)
{
	return CrCommandQueueSharedHandle(CreateCommandQueuePS(type));
}

CrFramebufferSharedHandle ICrRenderDevice::CreateFramebuffer(const CrFramebufferDescriptor& descriptor)
{
	// TODO Add validation rules here. All attachments must be the same size, etc.
	return CrFramebufferSharedHandle(CreateFramebufferPS(descriptor));
}

CrGPUFenceSharedHandle ICrRenderDevice::CreateGPUFence()
{
	return CrGPUFenceSharedHandle(CreateGPUFencePS());
}

CrGPUSemaphoreSharedHandle ICrRenderDevice::CreateGPUSemaphore()
{
	return CrGPUSemaphoreSharedHandle(CreateGPUSemaphorePS());
}

CrGraphicsShaderHandle ICrRenderDevice::CreateGraphicsShader(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor) const
{
	return CrGraphicsShaderHandle(CreateGraphicsShaderPS(graphicsShaderDescriptor));
}

CrComputeShaderHandle ICrRenderDevice::CreateComputeShader(const CrComputeShaderDescriptor& computeShaderDescriptor) const
{
	return CrComputeShaderHandle(CreateComputeShaderPS(computeShaderDescriptor));
}

CrGraphicsPipelineHandle ICrRenderDevice::CreateGraphicsPipeline(const CrGraphicsPipelineDescriptor& pipelineDescriptor, const ICrGraphicsShader* graphicsShader, const CrVertexDescriptor& vertexDescriptor, const CrRenderPassDescriptor& renderPassDescriptor)
{
	return CrGraphicsPipelineHandle(CreateGraphicsPipelinePS(pipelineDescriptor, graphicsShader, vertexDescriptor, renderPassDescriptor));
}

CrComputePipelineHandle ICrRenderDevice::CreateComputePipeline(const CrComputePipelineDescriptor& pipelineDescriptor, const ICrComputeShader* computeShader)
{
	return CrComputePipelineHandle(CreateComputePipelinePS(pipelineDescriptor, computeShader));
}

ICrHardwareGPUBuffer* ICrRenderDevice::CreateHardwareGPUBuffer(const CrHardwareGPUBufferDescriptor& descriptor)
{
	return CreateHardwareGPUBufferPS(descriptor);
}

CrIndexBufferSharedHandle ICrRenderDevice::CreateIndexBuffer(cr3d::DataFormat::T dataFormat, uint32_t numIndices)
{
	return CrIndexBufferSharedHandle(new CrIndexBuffer(this, dataFormat, numIndices));
}

CrRenderPassSharedHandle ICrRenderDevice::CreateRenderPass(const CrRenderPassDescriptor& renderPassDescriptor)
{
	return CrRenderPassSharedHandle(CreateRenderPassPS(renderPassDescriptor));
}

CrSamplerSharedHandle ICrRenderDevice::CreateSampler(const CrSamplerDescriptor& descriptor)
{
	return CrSamplerSharedHandle(CreateSamplerPS(descriptor));
}

CrSwapchainSharedHandle ICrRenderDevice::CreateSwapchain(const CrSwapchainDescriptor& swapchainDescriptor)
{
	CrAssertMsg(swapchainDescriptor.platformWindow, "Platform window cannot be null");
	CrAssertMsg(swapchainDescriptor.platformHandle, "Platform handle cannot be null");
	CrAssertMsg(swapchainDescriptor.format != cr3d::DataFormat::Invalid, "Must set a data format");

	CrSwapchainSharedHandle swapchain = CrSwapchainSharedHandle(CreateSwapchainPS(swapchainDescriptor));

	CrAssertMsg(swapchain->GetWidth() > 0, "Swapchain must have a width");
	CrAssertMsg(swapchain->GetHeight() > 0, "Swapchain must have a height");
	CrAssertMsg(swapchain->GetImageCount() > 0, "Swapchain must have at least one image");
	CrAssertMsg(swapchain->GetFormat() != cr3d::DataFormat::Invalid, "Swapchain must have a texture format");

	return swapchain;
}

CrTextureSharedHandle ICrRenderDevice::CreateTexture(const CrTextureDescriptor& descriptor)
{
	return CrTextureSharedHandle(CreateTexturePS(descriptor));
}

CrVertexBufferSharedHandle ICrRenderDevice::CreateVertexBuffer(uint32_t numVertices, const CrVertexDescriptor& vertexDescriptor)
{
	return CrVertexBufferSharedHandle(new CrVertexBufferCommon(this, numVertices, vertexDescriptor));
}

CrDataBufferSharedHandle ICrRenderDevice::CreateDataBuffer(cr3d::BufferAccess::T access, cr3d::DataFormat::T dataFormat, uint32_t numElements)
{
	return CrDataBufferSharedHandle(new CrDataBuffer(this, access, dataFormat, numElements));
}

cr3d::GPUWaitResult ICrRenderDevice::WaitForFence(ICrGPUFence* fence, uint64_t timeoutNanoseconds)
{
	return WaitForFencePS(fence, timeoutNanoseconds);
}

void ICrRenderDevice::ResetFence(ICrGPUFence* fence)
{
	ResetFencePS(fence);
}

void ICrRenderDevice::WaitIdle()
{
	WaitIdlePS();
}

const CrRenderDeviceProperties& ICrRenderDevice::GetProperties()
{
	return m_renderDeviceProperties;
}