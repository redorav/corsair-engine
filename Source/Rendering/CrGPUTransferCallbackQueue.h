#pragma once

#include "CrRenderingForwardDeclarations.h"

#include "Core/Containers/CrFixedVector.h"

#include "Core/Containers/CrDeque.h"

#include "Core/SmartPointers/CrSharedPtr.h"

struct CrGPUDownloadCallback
{
	CrGPUTransferCallbackType callback;
	CrHardwareGPUBufferHandle buffer;
};

struct CrDownloadCallbackList
{
	CrGPUFenceHandle fence;
	CrVector<CrGPUDownloadCallback> callbacks;
};

class CrGPUTransferCallbackQueue
{
public:

	~CrGPUTransferCallbackQueue();

	static const uint32_t MaximumCallbackLists = 4;

	void Initialize(ICrRenderDevice* renderDevice);

	void AddToQueue(const CrGPUDownloadCallback& callback);

	void Process();

private:
	
	ICrRenderDevice* m_renderDevice = nullptr;

	// Callbacks to be executed after a successful download operation, copying from GPU to CPU
	CrDownloadCallbackList* m_currentCallbackList = nullptr;

	// Active callback lists have had fence signal commands scheduled on
	// the GPU, and are waiting to receive that on the CPU. We want to
	// push back, but then get front to query the fence
	CrDeque<CrDownloadCallbackList*> m_activeCallbackLists;

	// Available callback lists are there to be reused
	CrFixedVector<CrDownloadCallbackList*, MaximumCallbackLists> m_availableCallbackLists;

	CrFixedVector<CrDownloadCallbackList, MaximumCallbackLists> m_callbackLists;
};