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

	const CrGPUFenceSharedHandle& GetCurrentWaitFence() const;

	CrSwapchainResult AcquireNextImage(const ICrGPUSemaphore* signalSemaphore, uint64_t timeoutNanoseconds = UINT64_MAX);

	void Present(ICrCommandQueue* queue, const ICrGPUSemaphore* waitSemaphore);

	const CrTextureSharedHandle& GetTexture(uint32_t index);

protected:

	virtual void PresentPS(ICrCommandQueue* queue, const ICrGPUSemaphore* waitSemaphore) = 0;

	virtual CrSwapchainResult AcquireNextImagePS(const ICrGPUSemaphore* signalSemaphore, uint64_t timeoutNanoseconds = UINT64_MAX) = 0;

	void CreateWaitFences(uint32_t imageCount);

	ICrRenderDevice*					m_renderDevice = nullptr;

	CrVector<CrTextureSharedHandle>		m_textures;

	CrVector<CrGPUFenceSharedHandle>	m_waitFences;

	uint32_t							m_imageCount	= 0;

	cr3d::DataFormat::T					m_format;

	uint32_t m_width					= 0;

	uint32_t m_height					= 0;

	uint32_t m_currentBufferIndex		= 0; // Active frame buffer index
};
