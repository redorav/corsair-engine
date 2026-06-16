#pragma once

#include "Graphics/CrGraphicsForwardDeclarations.h"
#include "Graphics/IGPUQueryPool.h"

#include "Core/CrHash.h"

#include "crstl/open_hashmap.h"
#include "crstl/vector.h"

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

	void Initialize(crgfx::IDevice* renderDevice, uint32_t maxFrames);

	void BeginFrame(crgfx::ICommandBuffer* commandBuffer, uint64_t frameIndex);

	void EndFrame(crgfx::ICommandBuffer* commandBuffer);

	CrGPUTimingRequest AllocateTimingRequest(CrHash hash);

	CrGPUInterval GetResultForFrame(CrHash hash) const;

	CrGPUInterval GetFrameDuration() const;

	crgfx::IGPUQueryPool* GetCurrentQueryPool() const;

private:

	crgfx::IGPUQueryPool* GetOldestQueryPool() const;

	uint64_t m_currentFrame;

	// Maximum number of query pools
	uint64_t m_maxFrames;

	// Current query pool
	uint64_t m_currentPoolIndex;

	// Timing request for the entire frame
	CrGPUTimingRequest m_frameTimingRequest;

	// Collection of query pools, ideally one per frame
	crstl::vector<crgfx::CrGPUQueryPoolHandle> m_queryPools;

	// There is a hashmap per frame as the query ids change every frame
	crstl::vector<crstl::open_hashmap<CrHash, CrGPUTimingRequest>> m_timingRequests;

	// Data for timings that were resolved this frame. They'll get stomped next frame
	// This is the raw data for the timestamps
	crstl::vector<crgfx::GPUTimestamp> m_timestampData;

	// Intervals resolved for the current raw data. Starting time is with respect to the
	// query inserted by the tracker on Begin()
	crstl::open_hashmap<CrHash, CrGPUInterval> m_timingIntervals;

	// Total time this frame took
	CrGPUInterval m_totalFrameInterval;
};