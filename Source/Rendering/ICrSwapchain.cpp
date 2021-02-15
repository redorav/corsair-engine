#include "CrRendering_pch.h"

#include "ICrSwapchain.h"
#include "ICrRenderDevice.h"

#include "Core/Logging/ICrDebug.h"

CrSwapchainDescriptor::CrSwapchainDescriptor()
	: platformWindow(nullptr)
	, platformHandle(nullptr)
	, requestedWidth(0)
	, requestedHeight(0)
	, requestedBufferCount(0)
	, format(cr3d::DataFormat::Invalid)
{

}

ICrSwapchain::ICrSwapchain(ICrRenderDevice* renderDevice, const CrSwapchainDescriptor& /*swapchainDescriptor*/)
	: m_renderDevice(renderDevice)
{

}

void ICrSwapchain::CreateWaitFences(uint32_t imageCount)
{
	m_waitFences.resize(imageCount);

	for (auto& waitFence : m_waitFences)
	{
		waitFence = m_renderDevice->CreateGPUFence();
	}
}

cr3d::DataFormat::T ICrSwapchain::GetFormat() const
{
	return m_format;
}

uint32_t ICrSwapchain::GetWidth() const
{
	return m_width;
}

uint32_t ICrSwapchain::GetHeight() const
{
	return m_height;
}

uint32_t ICrSwapchain::GetImageCount() const
{
	return (uint32_t)m_textures.size();
}

uint32_t ICrSwapchain::GetCurrentFrameIndex() const
{
	return m_currentBufferIndex;
}

const CrGPUFenceSharedHandle& ICrSwapchain::GetCurrentWaitFence() const
{
	return m_waitFences[m_currentBufferIndex];
}

CrSwapchainResult ICrSwapchain::AcquireNextImage(const ICrGPUSemaphore* signalSemaphore, uint64_t timeoutNanoseconds)
{
	return AcquireNextImagePS(signalSemaphore, timeoutNanoseconds);
}

void ICrSwapchain::Present(ICrCommandQueue* queue, const ICrGPUSemaphore* waitSemaphore)
{
	CrAssertMsg(queue != nullptr, "Must have a command queue to present");
	PresentPS(queue, waitSemaphore);
}

const CrTextureSharedHandle& ICrSwapchain::GetTexture(uint32_t index)
{
	CrAssert(index < m_textures.size());
	return m_textures[index];
}