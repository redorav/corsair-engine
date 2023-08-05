#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/ICrGPUQueryPool.h"

#include "Core/Containers/CrVector.h"
#include "Core/Containers/CrHashMap.h"
#include "Core/CrHash.h"

struct CrGPUTimingRequest
{
	CrGPUQueryId startQuery;
	CrGPUQueryId endQuery;
};

struct CrGPUInterval
{
	CrGPUInterval() : startTimeNanoseconds(0.0), durationNanoseconds(0.0) {}

	double startTimeNanoseconds;
	double durationNanoseconds;
};

class CrGPUTimingQueryTracker
{
public:

	void Initialize(ICrRenderDevice* renderDevice, uint32_t maxFrames);

	void BeginFrame(ICrCommandBuffer* commandBuffer, uint64_t frameIndex);

	void EndFrame(ICrCommandBuffer* commandBuffer);

	CrGPUTimingRequest AllocateTimingRequest(CrHash hash);

	CrGPUInterval GetResultForFrame(CrHash hash) const;

	CrGPUInterval GetFrameDuration() const;

	ICrGPUQueryPool* GetCurrentQueryPool() const;

private:

	ICrGPUQueryPool* GetOldestQueryPool() const;

	uint64_t m_currentFrame;

	// Maximum number of query pools
	uint64_t m_maxFrames;

	// Current query pool
	uint64_t m_currentPoolIndex;

	// Timing request for the entire frame
	CrGPUTimingRequest m_frameTimingRequest;

	// Collection of query pools, ideally one per frame
	CrVector<CrGPUQueryPoolHandle> m_queryPools;

	// There is a hashmap per frame as the query ids change every frame
	CrVector<CrHashMap<CrHash, CrGPUTimingRequest>> m_timingRequests;

	// Data for timings that were resolved this frame. They'll get stomped next frame
	// This is the raw data for the timestamps
	CrVector<CrGPUTimestamp> m_timestampData;

	// Intervals resolved for the current raw data. Starting time is with respect to the
	// query inserted by the tracker on Begin()
	CrHashMap<CrHash, CrGPUInterval> m_timingIntervals;

	// Total time this frame took
	CrGPUInterval m_totalFrameInterval;
};