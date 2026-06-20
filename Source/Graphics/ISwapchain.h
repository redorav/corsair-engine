#pragma once

#include "IGPUSynchronization.h"

#include "Graphics/CrGraphicsForwardDeclarations.h"
#include "Graphics/ITexture.h"

#include "crstl/fixed_string.h"
#include "crstl/vector.h"

class CrOSWindow;

namespace crgfx
{
	enum class CrSwapchainResult : uint32_t
	{
		Success,
		Invalid
	};

	struct SwapchainDescriptor
	{
		SwapchainDescriptor()
			: name("")
			, window(nullptr)
			, requestedWidth(0)
			, requestedHeight(0)
			, requestedBufferCount(0)
			, format(crgfx::DataFormat::Invalid)
		{}

		const char* name;
		CrOSWindow* window;
		uint32_t requestedWidth;
		uint32_t requestedHeight;
		uint32_t requestedBufferCount; // How many surfaces to request for this swapchain
		crgfx::DataFormat::T format;
	};

	class ISwapchain : public GPUAutoDeletable
	{
	public:

		ISwapchain(crgfx::IDevice* renderDevice, const crgfx::SwapchainDescriptor& swapchainDescriptor);

		virtual ~ISwapchain() {}

		crgfx::DataFormat::T GetFormat() const;

		uint32_t GetWidth() const;

		uint32_t GetHeight() const;

		uint32_t GetImageCount() const;

		uint32_t GetCurrentFrameIndex() const;

		crgfx::CrSwapchainResult AcquireNextImage(uint64_t timeoutNanoseconds = UINT64_MAX);

		void Present();

		void Resize(uint32_t width, uint32_t height);

		const crgfx::TextureHandle& GetCurrentTexture();

	protected:

		virtual void PresentPS() = 0;

		virtual void ResizePS(uint32_t width, uint32_t height) = 0;

		virtual crgfx::CrSwapchainResult AcquireNextImagePS(uint64_t timeoutNanoseconds = UINT64_MAX) = 0;

		crstl::vector<crgfx::TextureHandle> m_textures;

		crstl::fixed_string32 m_name;

		uint32_t m_imageCount;

		crgfx::DataFormat::T m_format;

		uint32_t m_width;

		uint32_t m_height;

		uint32_t m_currentBufferIndex; // Active frame buffer index

		bool m_imageAcquired;
	};
}