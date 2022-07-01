#include "CrRendering_pch.h"

#include "ICrSwapchain.h"

#include "Core/Logging/ICrDebug.h"

class ICrRenderDevice;

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
	, m_imageCount(0)
	, m_width(0)
	, m_height(0)
	, m_currentBufferIndex(0)
	, m_imageAcquired(false)
	, m_format(cr3d::DataFormat::Invalid)
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

const CrTextureSharedHandle& ICrSwapchain::GetTexture(uint32_t index)
{
	CrAssert(index < m_textures.size());
	return m_textures[index];
}