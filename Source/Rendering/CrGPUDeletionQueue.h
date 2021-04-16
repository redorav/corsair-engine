#pragma once

#include "CrRenderingForwardDeclarations.h"

#include "Rendering/CrGPUDeletable.h"

#include "Core/Containers/CrFixedVector.h"

#include "Core/Containers/CrDeque.h"

#include "Core/SmartPointers/CrSharedPtr.h"

struct CrDeletionList
{
	CrGPUFenceSharedHandle fence;
	CrVector<CrGPUDeletable*> deletables;
};

// A queue that manages deletion of GPU objects. Anything added to this queue
// will eventually get destroyed
class CrGPUDeletionQueue
{
public:

	~CrGPUDeletionQueue();

	static const uint32_t MaximumDeletionLists = 8;

	void Initialize(ICrRenderDevice* renderDevice);

	void AddToQueue(CrGPUDeletable* deletable);

	void Process();

	void Finalize();

private:
	
	ICrRenderDevice* m_renderDevice = nullptr;

	// The current deletion list points to all the resources being deleted
	// before we execute the fence. Once we execute the fence, we add it
	// to the active deletion lists to be waited on
	CrDeletionList* m_currentDeletionList = nullptr;

	// Active deletion lists have had fence signal commands scheduled on
	// the GPU, and are waiting to receive that on the CPU. We want to
	// push back, but then get front to query the fence
	CrDeque<CrDeletionList*> m_activeDeletionLists;

	// Available deletion lists are there to be reused
	CrFixedVector<CrDeletionList*, MaximumDeletionLists> m_availableDeletionLists;

	// These are the actual contents of the CrDeletionList
	// The other lists point to the contents here. We cannot
	// have reallocation on these lists so we use fixed size vectors
	CrFixedVector<CrDeletionList, MaximumDeletionLists> m_deletionLists;
};