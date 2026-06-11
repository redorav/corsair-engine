#include "Rendering/CrRendering_pch.h"

#include "ICrSwapchain.h"

#include "Core/Logging/ICrDebug.h"

class ICrRenderDevice;

CrSwapchainDescriptor::CrSwapchainDescriptor()
	: name("")
	, window(nullptr)
	, requestedWidth(0)
	, requestedHeight(0)
	, requestedBufferCount(0)
	, format(cr3d::DataFormat::Invalid)
{

}

ICrSwapchain::ICrSwapchain(ICrRenderDevice* renderDevice, const CrSwapchainDescriptor& swapchainDescriptor) : CrGPUAutoDeletable(renderDevice)
	, m_name(swapchainDescriptor.name)
	, m_imageCount(0)
	, m_format(cr3d::DataFormat::Invalid)
	, m_width(0)
	, m_height(0)
	, m_currentBufferIndex(0)
	, m_imageAcquired(false)
{

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

CrSwapchainResult ICrSwapchain::AcquireNextImage(uint64_t timeoutNanoseconds)
{
	return AcquireNextImagePS(timeoutNanoseconds);
}

void ICrSwapchain::Present()
{
	PresentPS();
}

void ICrSwapchain::Resize(uint32_t width, uint32_t height)
{
	ResizePS(width, height);
	m_width = width;
	m_height = height;
}

const CrTextureHandle& ICrSwapchain::GetCurrentTexture()
{
	return m_textures[m_currentBufferIndex];
}
