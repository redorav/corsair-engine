#include "Graphics/CrRendering_pch.h"

#include "ISwapchain.h"

#include "Core/Logging/ICrDebug.h"

namespace crgfx
{
	class IDevice;

	ISwapchain::ISwapchain(crgfx::IDevice* renderDevice, const crgfx::SwapchainDescriptor& swapchainDescriptor) : GPUAutoDeletable(renderDevice)
		, m_name(swapchainDescriptor.name)
		, m_imageCount(0)
		, m_format(crgfx::DataFormat::Invalid)
		, m_width(0)
		, m_height(0)
		, m_currentBufferIndex(0)
		, m_imageAcquired(false)
	{}

	crgfx::DataFormat::T ISwapchain::GetFormat() const
	{
		return m_format;
	}

	uint32_t ISwapchain::GetWidth() const
	{
		return m_width;
	}

	uint32_t ISwapchain::GetHeight() const
	{
		return m_height;
	}

	uint32_t ISwapchain::GetImageCount() const
	{
		return (uint32_t)m_textures.size();
	}

	uint32_t ISwapchain::GetCurrentFrameIndex() const
	{
		return m_currentBufferIndex;
	}

	crgfx::CrSwapchainResult ISwapchain::AcquireNextImage(uint64_t timeoutNanoseconds)
	{
		return AcquireNextImagePS(timeoutNanoseconds);
	}

	void ISwapchain::Present()
	{
		PresentPS();
	}

	void ISwapchain::Resize(uint32_t width, uint32_t height)
	{
		ResizePS(width, height);
		m_width = width;
		m_height = height;
	}

	const crgfx::TextureHandle& ISwapchain::GetCurrentTexture()
	{
		return m_textures[m_currentBufferIndex];
	}
};