#include "CrRendering_pch.h"

#include "ICrRenderDevice.h"
#include "ICrCommandQueue.h"
#include "ICrSampler.h"
#include "ICrTexture.h"
#include "ICrFramebuffer.h"
#include "ICrRenderPass.h"
#include "ICrShader.h"
#include "ICrPipeline.h"
#include "CrGPUBuffer.h"
#include "CrGPUStackAllocator.h"

#include "Core/CrMacros.h"

// Include all the necessary platforms here
#include "vulkan/CrRenderDevice_vk.h"

#include "d3d12/CrRenderDevice_d3d12.h"

static ICrRenderDevice* g_renderDevice;

ICrRenderDevice::ICrRenderDevice()
{
	
}

ICrRenderDevice::~ICrRenderDevice()
{

}

// TODO Create a factory here where we create the render device from the graphics API. From then on everything is created
// through the render device so it should all have the proper platform
void ICrRenderDevice::Init(cr3d::GraphicsApi::T graphicsApi)
{
	m_renderDeviceProperties.m_graphicsApi = graphicsApi;

	InitPS();
}

CrCommandQueueSharedHandle ICrRenderDevice::CreateCommandQueue(CrCommandQueueType::T type)
{
	return CrCommandQueueSharedHandle(CreateCommandQueuePS(type));
}

CrFramebufferSharedHandle ICrRenderDevice::CreateFramebuffer(const CrFramebufferCreateParams& params)
{
	// TODO Add validation rules here. All attachments must be the same size, etc.
	return CrFramebufferSharedHandle(CreateFramebufferPS(params));
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

CrGraphicsPipelineHandle ICrRenderDevice::CreateGraphicsPipeline(const CrGraphicsPipelineDescriptor& psoDescriptor, const ICrGraphicsShader* graphicsShader, const CrVertexDescriptor& vertexDescriptor, const CrRenderPassDescriptor& renderPassDescriptor)
{
	return CrGraphicsPipelineHandle(CreateGraphicsPipelinePS(psoDescriptor, graphicsShader, vertexDescriptor, renderPassDescriptor));
}

ICrHardwareGPUBuffer* ICrRenderDevice::CreateHardwareGPUBuffer(const CrGPUBufferDescriptor& descriptor)
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
	CrAssertMsg(swapchainDescriptor.platformWindow, "Cannot be null");
	CrAssertMsg(swapchainDescriptor.platformHandle, "Cannot be null");
	CrAssertMsg(swapchainDescriptor.format != cr3d::DataFormat::Invalid, "Must set a data format");

	CrSwapchainSharedHandle swapchain = CrSwapchainSharedHandle(CreateSwapchainPS(swapchainDescriptor));

	CrAssertMsg(swapchain->GetWidth() > 0, "Swapchain must have a width");
	CrAssertMsg(swapchain->GetHeight() > 0, "Swapchain must have a height");
	CrAssertMsg(swapchain->GetImageCount() > 0, "Swapchain must have at least one image");
	CrAssertMsg(swapchain->GetFormat() != cr3d::DataFormat::Invalid, "Swapchain must have a texture format");

	return swapchain;
}

CrTextureSharedHandle ICrRenderDevice::CreateTexture(const CrTextureCreateParams& params)
{
	return CrTextureSharedHandle(CreateTexturePS(params));
}

CrVertexBufferSharedHandle ICrRenderDevice::CreateVertexBuffer(uint32_t numVertices, const CrVertexDescriptor& vertexDescriptor)
{
	return CrVertexBufferSharedHandle(new CrVertexBufferCommon(this, numVertices, vertexDescriptor));
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

ICrRenderDevice* ICrRenderDevice::GetRenderDevice()
{
	return g_renderDevice;
}

void ICrRenderDevice::Create(cr3d::GraphicsApi::T graphicsApi)
{
	// TODO Treat this like a factory (on PC) through the API. That way the rest of the code
	// doesn't need to know about platform-specific code, only the render device.
	if (graphicsApi == cr3d::GraphicsApi::Vulkan)
	{
		g_renderDevice = new CrRenderDeviceVulkan();
	}
	else if (graphicsApi == cr3d::GraphicsApi::D3D12)
	{
		//g_renderDevice = new CrRenderDeviceD3D12();
	}

	g_renderDevice->Init(graphicsApi);
}