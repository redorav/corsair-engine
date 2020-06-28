#include "CrRendering_pch.h"

#include "ICrSwapchain.h"
#include "ICrRenderDevice.h"

#include "Core/Logging/ICrDebug.h"

ICrSwapchain::ICrSwapchain()
	: m_format(cr3d::DataFormat::Invalid)
	, m_sampleCount(cr3d::SampleCount::S1)
{

}

void ICrSwapchain::Create(ICrRenderDevice* renderDevice, uint32_t requestedWidth, uint32_t requestedHeight)
{
	CreatePS(renderDevice, requestedWidth, requestedHeight);

	CrAssertMsg(m_width > 0, "Swapchain must have a width!");
	CrAssertMsg(m_height > 0, "Swapchain must have a height!");
	CrAssertMsg(m_imageCount > 0, "Swapchain must have at least one image!");
	CrAssertMsg(m_format != cr3d::DataFormat::Invalid, "Swapchain must have a texture format!");

	m_waitFences.resize(m_imageCount);

	for (auto& waitFence : m_waitFences)
	{
		waitFence = renderDevice->CreateGPUFence();
	}
}

cr3d::DataFormat::T ICrSwapchain::GetFormat()
{
	return m_format;
}

cr3d::SampleCount ICrSwapchain::GetSampleCount()
{
	return m_sampleCount;
}

uint32_t ICrSwapchain::GetWidth()
{
	return m_width;
}

uint32_t ICrSwapchain::GetHeight()
{
	return m_height;
}

uint32_t ICrSwapchain::GetImageCount()
{
	return (uint32_t)m_textures.size();
}

uint32_t ICrSwapchain::GetCurrentFrameIndex()
{
	return m_currentBufferIndex;
}

const CrGPUFenceSharedHandle& ICrSwapchain::GetCurrentWaitFence()
{
	return m_waitFences[m_currentBufferIndex];
}

CrSwapchainResult ICrSwapchain::AcquireNextImage(const ICrGPUSemaphore* signalSemaphore, uint64_t timeoutNanoseconds)
{
	return AcquireNextImagePS(signalSemaphore, timeoutNanoseconds);
}

void ICrSwapchain::Present(ICrCommandQueue* queue, const ICrGPUSemaphore* waitSemaphore)
{
	CrAssertMsg(queue, "Must have a command queue to present!");
	PresentPS(queue, waitSemaphore);
}

const CrTextureSharedHandle& ICrSwapchain::GetTexture(uint32_t index)
{
	CrAssert(index < m_textures.size());
	return m_textures[index];
}
