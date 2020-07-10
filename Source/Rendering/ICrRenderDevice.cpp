#include "CrRendering_pch.h"

#include "ICrRenderDevice.h"
#include "ICrCommandQueue.h"
#include "ICrSampler.h"
#include "ICrTexture.h"
#include "CrGPUBuffer.h"
#include "ICrGPUStackAllocator.h"

#include "Core/CrMacros.h"
#include GRAPHICS_API_PATH(CrRenderDevice)

static ICrRenderDevice* g_renderDevice;

ICrRenderDevice::ICrRenderDevice()
{
	
}

ICrRenderDevice::~ICrRenderDevice()
{

}

// TODO Create a factory here where we create the render device from the graphics API. From then on everything is created
// through the render device so it should all have the proper platform
void ICrRenderDevice::Init(void* platformHandle, void* platformWindow, cr3d::GraphicsApi::T graphicsApi)
{
	m_renderDeviceProperties.m_graphicsApi = graphicsApi;

	InitPS(platformHandle, platformWindow);
}

void ICrRenderDevice::Present()
{
	PresentPS();
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
	CrSwapchainSharedHandle swapchain = CrSwapchainSharedHandle(CreateSwapchainPS(swapchainDescriptor));
	swapchain->Create(this, swapchainDescriptor);
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

ICrHardwareGPUBuffer* ICrRenderDevice::CreateHardwareGPUBuffer(const CrGPUBufferCreateParams& params)
{
	return CreateHardwareGPUBufferPS(params);
}

CrUniquePtr<ICrGPUStackAllocator> ICrRenderDevice::CreateGPUMemoryStream()
{
	return CrUniquePtr<ICrGPUStackAllocator>(CreateGPUMemoryStreamPS());
}

CrGPUFenceSharedHandle ICrRenderDevice::CreateGPUFence()
{
	return CrGPUFenceSharedHandle(CreateGPUFencePS());
}

CrGPUSemaphoreSharedHandle ICrRenderDevice::CreateGPUSemaphore()
{
	return CrGPUSemaphoreSharedHandle(CreateGPUSemaphorePS());
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

CrRenderDeviceVulkan* ICrRenderDevice::GetRenderDevicePS() // TODO Remove
{
	return static_cast<CrRenderDeviceVulkan*>(g_renderDevice);
}

const CrSwapchainSharedHandle& ICrRenderDevice::GetMainSwapchain() const
{
	return m_swapchain;
}

const CrTextureSharedHandle& ICrRenderDevice::GetMainDepthBuffer() const
{
	return m_depthStencilTexture;
}

void ICrRenderDevice::Create(void* platformHandle, void* platformWindow, cr3d::GraphicsApi::T graphicsApi)
{
	g_renderDevice = new CrRenderDeviceVulkan();
	g_renderDevice->Init(platformHandle, platformWindow, graphicsApi);
}