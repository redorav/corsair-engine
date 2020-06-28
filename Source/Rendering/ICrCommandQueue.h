#pragma once

class ICrGPUSemaphore;
class ICrGPUFence;
class ICrCommandBuffer;
class ICrRenderDevice;

namespace CrCommandQueueType
{
	// Queues are each a subset of the other
	//  ______________
	// |   Graphics   |
	// |  __________  |
	// | |  Compute | |
	// | |  ______  | |
	// | | | Copy | | |
	// | | |______| | |
	// | |__________| |
	// |______________|

	enum T : uint8_t
	{
		Graphics,
		Compute,
		Copy
	};
};

class ICrCommandQueue
{
public:

	ICrCommandQueue(ICrRenderDevice* renderDevice);

	virtual ~ICrCommandQueue();

	ICrCommandBuffer* CreateCommandBuffer();

	void DestroyCommandBuffer(const ICrCommandBuffer* commandBuffer);

	void SubmitCommandBuffer(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence);

	const ICrCommandBuffer* GetLastSubmittedCommandBuffer();

	void WaitIdle();

	ICrRenderDevice* GetRenderDevice() const;

protected:

	virtual ICrCommandBuffer* CreateCommandBufferPS() = 0;

	virtual void DestroyCommandBufferPS(const ICrCommandBuffer* commandBuffer) = 0;

	virtual void SubmitCommandBufferPS(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence) = 0;

	virtual void WaitIdlePS() = 0;

	ICrRenderDevice* m_renderDevice = nullptr;

	CrCommandQueueType::T m_type = CrCommandQueueType::Graphics;
};
