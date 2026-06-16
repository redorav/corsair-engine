#include "Graphics/CrRendering_pch.h"

#include "Graphics/CrGPUTimingQueryTracker.h"
#include "Graphics/IDevice.h"
#include "Graphics/ICommandBuffer.h"

void CrGPUTimingQueryTracker::Initialize(crgfx::IDevice* renderDevice, uint32_t maxFrames)
{
	m_maxFrames = maxFrames;

	m_queryPools.resize(maxFrames);

	crgfx::GPUQueryPoolDescriptor queryPoolDescriptor(crgfx::QueryType::Timestamp, 1024);
	for (uint32_t i = 0; i < maxFrames; ++i)
	{
		m_queryPools[i] = renderDevice->CreateGPUQueryPool(queryPoolDescriptor);
	}

	m_timingRequests.resize(maxFrames);
}

void CrGPUTimingQueryTracker::BeginFrame(crgfx::ICommandBuffer* commandBuffer, uint64_t frameIndex)
{
	m_currentFrame = frameIndex;

	m_currentPoolIndex = frameIndex % m_maxFrames;

	// Resolve queries from the oldest frame
	crgfx::IGPUQueryPool* oldestPool = GetOldestQueryPool();
	m_timestampData.resize(oldestPool->GetActiveQueryCount());
	m_timingIntervals.clear();
	m_timingRequests[m_currentPoolIndex].clear();

	if (!m_timestampData.empty())
	{
		// Get timing data
		oldestPool->GetTimestampData(m_timestampData.data(), (uint32_t)m_timestampData.size());

		// Resolve into intervals
		crgfx::GPUTimestamp frameStartTimeTicks = m_timestampData[m_frameTimingRequest.startQuery.id];
		crgfx::GPUTimestamp frameEndTimeTicks   = m_timestampData[m_frameTimingRequest.endQuery.id];

		for (const auto& iter : m_timingRequests[(m_currentPoolIndex + 1) % m_maxFrames])
		{
			CrHash hash = iter.first;
			CrGPUTimingRequest request = iter.second;

			crgfx::GPUTimestamp intervalStart = m_timestampData[request.startQuery.id];
			crgfx::GPUTimestamp intervalEnd = m_timestampData[request.endQuery.id];

			CrGPUInterval interval;
			interval.startTimeNanoseconds = oldestPool->GetDuration(frameStartTimeTicks, intervalStart);
			interval.durationNanoseconds = oldestPool->GetDuration(intervalStart, intervalEnd);

			m_timingIntervals.insert(hash, interval);
		}

		m_totalFrameInterval.startTimeNanoseconds = oldestPool->GetDuration(frameStartTimeTicks, frameStartTimeTicks);
		m_totalFrameInterval.durationNanoseconds  = oldestPool->GetDuration(frameStartTimeTicks, frameEndTimeTicks);
	}

	crgfx::IGPUQueryPool* currentPool = GetCurrentQueryPool();
	currentPool->Reset(commandBuffer);

	m_frameTimingRequest.startQuery = currentPool->Allocate();
	m_frameTimingRequest.endQuery = currentPool->Allocate();

	commandBuffer->BeginTimestampQuery(currentPool, m_frameTimingRequest.startQuery);
}

void CrGPUTimingQueryTracker::EndFrame(crgfx::ICommandBuffer* commandBuffer)
{
	crgfx::IGPUQueryPool* currentPool = GetCurrentQueryPool();

	// Insert final query for the frame
	commandBuffer->EndTimestampQuery(currentPool, m_frameTimingRequest.endQuery);

	currentPool->Resolve(commandBuffer);
}

CrGPUTimingRequest CrGPUTimingQueryTracker::AllocateTimingRequest(CrHash hash)
{
	crgfx::IGPUQueryPool* currentPool = GetCurrentQueryPool();

	CrGPUTimingRequest request;
	request.startQuery = currentPool->Allocate();
	request.endQuery = currentPool->Allocate();

	m_timingRequests[m_currentPoolIndex].insert(hash, request);

	return request;
}

CrGPUInterval CrGPUTimingQueryTracker::GetResultForFrame(CrHash hash) const
{
	const auto& queryIter = m_timingIntervals.find(hash);
	if (queryIter != m_timingIntervals.end())
	{
		return queryIter->second;
	}
	else
	{
		return CrGPUInterval();
	}
}

CrGPUInterval CrGPUTimingQueryTracker::GetFrameDuration() const
{
	return m_totalFrameInterval;
}

crgfx::IGPUQueryPool* CrGPUTimingQueryTracker::GetCurrentQueryPool() const
{
	return m_queryPools[m_currentPoolIndex].get();
}

crgfx::IGPUQueryPool* CrGPUTimingQueryTracker::GetOldestQueryPool() const
{
	return m_queryPools[(m_currentPoolIndex + 1) % m_maxFrames].get();
}
