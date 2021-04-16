#include "CrRendering_pch.h"

#include "Rendering/ICrRenderDevice.h"

#include "Rendering/ICrCommandQueue.h"

#include "Rendering/CrGPUDeletionQueue.h"

#include "Core/Logging/ICrDebug.h"

#include "Core/CrFrameTime.h"

static const bool DebugDeletionQueues = false;

CrGPUDeletionQueue::~CrGPUDeletionQueue()
{
	
}

void CrGPUDeletionQueue::Initialize(ICrRenderDevice* renderDevice)
{
	m_renderDevice = renderDevice;

	// We need as many as the system requests plus an extra one for when they're
	// all in flight (and the system reserves "the next one"). We could lazily
	// allocate in the AddToQueue function but this is simpler to reason about
	m_deletionLists.resize(MaximumDeletionLists);

	for (uint32_t i = 0; i < m_deletionLists.size(); ++i)
	{
		// Create fences
		m_deletionLists[i].fence = m_renderDevice->CreateGPUFence();

		m_availableDeletionLists.push_back(&m_deletionLists[i]);
	}

	// Pop from the vector to get an available list
	m_currentDeletionList = m_availableDeletionLists.back();
	m_availableDeletionLists.pop_back();
}

void CrGPUDeletionQueue::AddToQueue(CrGPUDeletable* deletable)
{
	// TODO Add synchronization for multithreading
	m_currentDeletionList->deletables.push_back(deletable);
}

void CrGPUDeletionQueue::Process()
{
	// 1. Loop through the active lists and delete any objects that are guaranteed
	// to have been processed by the GPU

	if (DebugDeletionQueues)
	{
		CrLog("CrGPUDeletionQueue Current Frame %i", CrFrameTime::GetFrameCount());
		CrLog("%i active deletion lists", m_activeDeletionLists.size());
	}

	while (!m_activeDeletionLists.empty())
	{
		// Get the last list we pushed into the active list
		CrDeletionList* deletionList = m_activeDeletionLists.front();

		if (m_renderDevice->GetFenceStatus(deletionList->fence.get()) == cr3d::GPUFenceResult::Success)
		{
			for (CrGPUDeletable* deletable : deletionList->deletables)
			{
				delete deletable;
			}

			// Reset the deletion list's properties
			deletionList->deletables.clear();
			m_renderDevice->ResetFence(deletionList->fence.get());

			// Put back in the available list and remove from active
			m_availableDeletionLists.push_back(deletionList);
			m_activeDeletionLists.pop_front();

			if (DebugDeletionQueues)
			{
				CrLog("Deletion list emptied");
			}
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
	if (!m_currentDeletionList->deletables.empty())
	{
		if (DebugDeletionQueues)
		{
			CrLog("Added current deletion list to active lists");
		}

		m_activeDeletionLists.push_back(m_currentDeletionList);
		m_renderDevice->GetMainCommandQueue()->SignalFence(m_currentDeletionList->fence.get());
		m_currentDeletionList = nullptr;
	}

	// 3. Find an available list and assign it to the current deletion list
	if (!m_currentDeletionList && !m_availableDeletionLists.empty())
	{
		if (DebugDeletionQueues)
		{
			CrLog("Deletion list reserved from available list");
		}

		m_currentDeletionList = m_availableDeletionLists.back();
		m_availableDeletionLists.pop_back();
	}

	CrAssertMsg(m_currentDeletionList, "Current deletion list cannot be null");
}

void CrGPUDeletionQueue::Finalize()
{
	// Push current list to the main queue and signal it. These are the last remaining resources in flight
	m_activeDeletionLists.push_back(m_currentDeletionList);
	m_renderDevice->GetMainCommandQueue()->SignalFence(m_currentDeletionList->fence.get());

	// Clear the rest of the resources that have been added to the lists, and wait for them
	// instead of just checking whether the fence has been signaled at this point. There is
	// no real risk of locking up here because we have certainty that all fences were queued
	for (CrDeletionList* deletionList : m_activeDeletionLists)
	{
		if (m_renderDevice->WaitForFence(deletionList->fence.get(), UINT64_MAX) == cr3d::GPUFenceResult::Success)
		{
			for (CrGPUDeletable* deletable : deletionList->deletables)
			{
				delete deletable;
			}

			deletionList->deletables.clear();
		}
	}

	m_currentDeletionList = nullptr;
	m_activeDeletionLists.clear();
}