#pragma once

#include "ICrGPUSynchronization.h"

#include "Core/Containers/CrVector.h"

#include "Core/SmartPointers/CrSharedPtr.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

enum class CrSwapchainResult : uint32_t
{
	Success,
	Invalid
};

struct CrSwapchainDescriptor
{
	CrSwapchainDescriptor();

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

	const CrGPUSemaphoreSharedHandle& GetCurrentPresentCompleteSemaphore() const;

	CrSwapchainResult AcquireNextImage(uint64_t timeoutNanoseconds = UINT64_MAX);

	void Present(const ICrGPUSemaphore* waitSemaphore);

	const CrTextureSharedHandle& GetTexture(uint32_t index);

protected:

	virtual void PresentPS(const ICrGPUSemaphore* waitSemaphore) = 0;

	virtual CrSwapchainResult AcquireNextImagePS(const ICrGPUSemaphore* signalSemaphore, uint64_t timeoutNanoseconds = UINT64_MAX) = 0;

	void CreatePresentSemaphores(uint32_t imageCount);

	ICrRenderDevice*						m_renderDevice = nullptr;

	CrVector<CrTextureSharedHandle>			m_textures;

	// Semaphores are signaled when present completes
	CrVector<CrGPUSemaphoreSharedHandle>	m_presentCompleteSemaphores;

	uint32_t								m_imageCount	= 0;

	cr3d::DataFormat::T						m_format;

	uint32_t m_width						= 0;

	uint32_t m_height						= 0;

	// We need to have another index for the semaphore because we don't really know
	// the buffer index until we have acquired the image, but we need to signal the
	// semaphore during that call. The initialization value is important to start
	// at 0 on the first call to present
	uint32_t m_currentSemaphoreIndex		= (uint32_t)-1;

	uint32_t m_currentBufferIndex			= 0; // Active frame buffer index
};
