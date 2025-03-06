#pragma once

#include "ICrGPUSynchronization.h"

#include "Core/Containers/CrVector.h"

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/ICrTexture.h"

#include "crstl/fixed_string.h"

class CrOSWindow;

enum class CrSwapchainResult : uint32_t
{
	Success,
	Invalid
};

struct CrSwapchainDescriptor
{
	CrSwapchainDescriptor();

	const char* name;
	CrOSWindow* window;
	uint32_t requestedWidth;
	uint32_t requestedHeight;
	uint32_t requestedBufferCount; // How many surfaces to request for this swapchain
	cr3d::DataFormat::T format;
};

class ICrSwapchain : public CrGPUAutoDeletable
{
public:

	ICrSwapchain(ICrRenderDevice* renderDevice, const CrSwapchainDescriptor& swapchainDescriptor);

	virtual ~ICrSwapchain() {}

	cr3d::DataFormat::T GetFormat() const;

	uint32_t GetWidth() const;

	uint32_t GetHeight() const;

	uint32_t GetImageCount() const;

	uint32_t GetCurrentFrameIndex() const;

	CrSwapchainResult AcquireNextImage(uint64_t timeoutNanoseconds = UINT64_MAX);

	void Present();

	void Resize(uint32_t width, uint32_t height);

	const CrTextureHandle& GetCurrentTexture();

protected:

	virtual void PresentPS() = 0;

	virtual void ResizePS(uint32_t width, uint32_t height) = 0;

	virtual CrSwapchainResult AcquireNextImagePS(uint64_t timeoutNanoseconds = UINT64_MAX) = 0;

	CrVector<CrTextureHandle> m_textures;

	crstl::fixed_string32 m_name;

	uint32_t m_imageCount;

	cr3d::DataFormat::T m_format;

	uint32_t m_width;

	uint32_t m_height;

	uint32_t m_currentBufferIndex; // Active frame buffer index

	bool m_imageAcquired;
};
