#pragma once

#include "CrRenderingForwardDeclarations.h"

#include "Rendering/ICrGPUSynchronization.h"

#include "crstl/deque.h"
#include "crstl/fixed_vector.h"

class CrGPUDeletable;

struct CrDeletionList
{
	CrGPUFenceHandle fence;
	CrVector<CrGPUDeletable*> deletables;
};

// A queue that manages deletion of GPU objects. Anything added to this queue
// will eventually get destroyed
class CrGPUDeletionQueue
{
public:

	~CrGPUDeletionQueue();

	static const uint32_t MaximumDeletionLists = 4;

	void Initialize(ICrRenderDevice* renderDevice);

	void AddToQueue(CrGPUDeletable* deletable);

	// Processes pending requests and adds current requests so that they're waited on
	// This is intended for in-flight resources during the frame
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
	crstl::deque<CrDeletionList*> m_activeDeletionLists;

	// Available deletion lists are there to be reused
	crstl::fixed_vector<CrDeletionList*, MaximumDeletionLists> m_availableDeletionLists;

	// These are the actual contents of the CrDeletionList
	// The other lists point to the contents here. We cannot
	// have reallocation on these lists so we use fixed size vectors
	crstl::fixed_vector<CrDeletionList, MaximumDeletionLists> m_deletionLists;
};