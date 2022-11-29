#include "CrRendering_pch.h"

#include "Rendering/ICrRenderDevice.h"

#include "Rendering/CrGPUTransferCallbackQueue.h"

#include "Core/Logging/ICrDebug.h"

#include "Core/CrFrameTime.h"

CrGPUTransferCallbackQueue::~CrGPUTransferCallbackQueue()
{

}

void CrGPUTransferCallbackQueue::Initialize(ICrRenderDevice* renderDevice)
{
	m_renderDevice = renderDevice;
	
	// We need as many as the system requests plus an extra one for when they're
	// all in flight (and the system reserves "the next one"). We could lazily
	// allocate in the AddToQueue function but this is simpler to reason about
	m_callbackLists.resize(MaximumCallbackLists);

	for (uint32_t i = 0; i < m_callbackLists.size(); ++i)
	{
		m_callbackLists[i].fence = m_renderDevice->CreateGPUFence();

		m_availableCallbackLists.push_back(&m_callbackLists[i]);
	}

	// Pop from the vector to get an available list
	m_currentCallbackList = m_availableCallbackLists.back();
	m_availableCallbackLists.pop_back();
}

void CrGPUTransferCallbackQueue::AddToQueue(const CrGPUDownloadCallback& callback)
{
	// TODO Add synchronization for multithreading
	m_currentCallbackList->callbacks.push_back(callback);
}

void CrGPUTransferCallbackQueue::Process()
{
	while (!m_activeCallbackLists.empty())
	{
		// Get the last list we pushed into the active list
		CrDownloadCallbackList* callbackList = m_activeCallbackLists.front();

		if (m_renderDevice->GetFenceStatus(callbackList->fence.get()) == cr3d::GPUFenceResult::Success)
		{
			for (const CrGPUDownloadCallback& callback : callbackList->callbacks)
			{
				callback.callback(callback.buffer);
			}

			// Reset the deletion list's properties
			callbackList->callbacks.clear();
			m_renderDevice->ResetFence(callbackList->fence.get());

			// Put back in the available list and remove from active
			m_availableCallbackLists.push_back(callbackList);
			m_activeCallbackLists.pop_front();
		}
		else
		{
			// Our lists are sequential, i.e. if this one has not been signaled,
			// it is guaranteed the others won't have been
			break;
		}
	}

	// 2. If there are any elements to delete, add the current list to the active
	// list and submit a fence signal to the queue
	if (!m_currentCallbackList->callbacks.empty())
	{
		m_activeCallbackLists.push_back(m_currentCallbackList);
		m_renderDevice->SignalFence(CrCommandQueueType::Graphics, m_currentCallbackList->fence.get());
		m_currentCallbackList = nullptr;
	}

	if (!m_currentCallbackList && !m_availableCallbackLists.empty())
	{
		m_currentCallbackList = m_availableCallbackLists.back();
		m_availableCallbackLists.pop_back();
	}

	CrAssertMsg(m_currentCallbackList, "Current deletion list cannot be null");
}