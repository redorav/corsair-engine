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

void ICrSwapchain::CreatePresentSemaphores(uint32_t imageCount)
{
	m_presentCompleteSemaphores.resize(imageCount);

	for (CrGPUSemaphoreSharedHandle& waitSemaphore : m_presentCompleteSemaphores)
	{
		waitSemaphore = m_renderDevice->CreateGPUSemaphore();
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

const CrGPUSemaphoreSharedHandle& ICrSwapchain::GetCurrentPresentCompleteSemaphore() const
{
	return m_presentCompleteSemaphores[m_currentSemaphoreIndex];
}

CrSwapchainResult ICrSwapchain::AcquireNextImage(uint64_t timeoutNanoseconds)
{
	m_currentSemaphoreIndex = (m_currentSemaphoreIndex + 1) % m_imageCount;
	CrSwapchainResult swapchainResult = AcquireNextImagePS(m_presentCompleteSemaphores[m_currentSemaphoreIndex].get(), timeoutNanoseconds);
	return swapchainResult;
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