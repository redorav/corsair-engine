#pragma once

#include "ICrGPUSynchronization.h"

#include "Core/Containers/CrVector.h"
#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/String/CrFixedString.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

enum class CrSwapchainResult : uint32_t
{
	Success,
	Invalid
};

struct CrSwapchainDescriptor
{
	CrSwapchainDescriptor();

	CrFixedString64 name;
	void* platformWindow;
	void* platformHandle;
	uint32_t requestedWidth;
	uint32_t requestedHeight;
	uint32_t requestedBufferCount; // How many surfaces to request for this swapchain
	cr3d::DataFormat::T format;
};

class ICrSwapchain
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

	const CrTextureSharedHandle& GetTexture(uint32_t index);

protected:

	virtual void PresentPS() = 0;

	virtual CrSwapchainResult AcquireNextImagePS(uint64_t timeoutNanoseconds = UINT64_MAX) = 0;

	ICrRenderDevice*						m_renderDevice = nullptr;

	CrVector<CrTextureSharedHandle>			m_textures;

	uint32_t								m_imageCount;

	cr3d::DataFormat::T						m_format;

	uint32_t m_width;

	uint32_t m_height;

	uint32_t m_currentBufferIndex; // Active frame buffer index

	bool m_imageAcquired;
};
