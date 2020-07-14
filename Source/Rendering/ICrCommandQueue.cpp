#include "CrRendering_pch.h"

#include "ICrCommandQueue.h"
#include "ICrCommandBuffer.h"

#include "Core/Logging/ICrDebug.h"

ICrCommandQueue::ICrCommandQueue(ICrRenderDevice* renderDevice) 
	: m_renderDevice(renderDevice)
{

}

ICrCommandQueue::~ICrCommandQueue()
{

}

CrCommandBufferSharedHandle ICrCommandQueue::CreateCommandBuffer()
{
	return CrCommandBufferSharedHandle(CreateCommandBufferPS());
}

void ICrCommandQueue::SubmitCommandBuffer(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence)
{
	SubmitCommandBufferPS(commandBuffer, waitSemaphore, signalSemaphore, signalFence);
}

void ICrCommandQueue::WaitIdle()
{
	WaitIdlePS();
}

ICrRenderDevice* ICrCommandQueue::GetRenderDevice() const
{
	return m_renderDevice;
}
