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

ICrCommandBuffer* ICrCommandQueue::CreateCommandBuffer()
{
	return CreateCommandBufferPS();
}

void ICrCommandQueue::DestroyCommandBuffer(const ICrCommandBuffer* commandBuffer)
{
	CrAssertMsg(commandBuffer->m_ownerCommandQueue == this, "Command buffer should be created and destroyed by the same command queue!");

	DestroyCommandBufferPS(commandBuffer);
	delete commandBuffer;
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
