#include "CrRendering_pch.h"

#include "CrGPUTimingQueryTracker.h"
#include "Rendering/ICrGPUQueryPool.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/ICrCommandBuffer.h"

void CrGPUTimingQueryTracker::Initialize(ICrRenderDevice* renderDevice, uint32_t maxFrames)
{
	m_maxFrames = maxFrames;

	m_queryPools.resize(maxFrames);

	CrGPUQueryPoolDescriptor queryPoolDescriptor(cr3d::QueryType::Timestamp, 1024);
	for (uint32_t i = 0; i < maxFrames; ++i)
	{
		m_queryPools[i] = renderDevice->CreateGPUQueryPool(queryPoolDescriptor);
	}

	m_timingRequests.resize(maxFrames);
}

void CrGPUTimingQueryTracker::BeginFrame(ICrCommandBuffer* commandBuffer, uint64_t currentFrame)
{
	m_currentFrame = currentFrame;

	m_currentPoolIndex = currentFrame % m_maxFrames;

	// Resolve queries from the oldest frame
	ICrGPUQueryPool* oldestPool = GetOldestQueryPool();
	m_timestampData.resize(oldestPool->GetActiveQueryCount());
	m_timingIntervals.clear();

	if (!m_timestampData.empty())
	{
		// Get timing data
		oldestPool->GetTimestampData(m_timestampData.data(), (uint32_t)m_timestampData.size());

		// Resolve into intervals
		CrGPUTimestamp frameStartTimeTicks = m_timestampData[m_frameTimingRequest.startQuery.id];

		for (const auto& iter : m_timingRequests[(m_currentPoolIndex + 1) % m_maxFrames])
		{
			CrHash hash = iter.first;
			CrGPUTimingRequest request = iter.second;

			CrGPUTimestamp intervalStart = m_timestampData[request.startQuery.id];
			CrGPUTimestamp intervalEnd = m_timestampData[request.endQuery.id];

			CrGPUInterval interval;
			interval.startTimeNanoseconds = oldestPool->GetDuration(frameStartTimeTicks, intervalStart);
			interval.durationNanoseconds = oldestPool->GetDuration(intervalStart, intervalEnd);

			m_timingIntervals.insert({ hash, interval });
		}
	}

	ICrGPUQueryPool* currentPool = GetCurrentQueryPool();
	currentPool->Reset(commandBuffer);

	m_frameTimingRequest.startQuery = currentPool->Allocate();
	m_frameTimingRequest.endQuery = currentPool->Allocate();

	commandBuffer->BeginTimestampQuery(currentPool, m_frameTimingRequest.startQuery);
}

void CrGPUTimingQueryTracker::EndFrame(ICrCommandBuffer* commandBuffer)
{
	ICrGPUQueryPool* currentPool = GetCurrentQueryPool();

	// Insert final query for the frame
	commandBuffer->EndTimestampQuery(currentPool, m_frameTimingRequest.endQuery);

	currentPool->Resolve(commandBuffer);
}

CrGPUTimingRequest CrGPUTimingQueryTracker::AllocateTimingRequest(CrHash hash)
{
	ICrGPUQueryPool* currentPool = GetCurrentQueryPool();

	CrGPUTimingRequest request;
	request.startQuery = currentPool->Allocate();
	request.endQuery = currentPool->Allocate();

	m_timingRequests[m_currentPoolIndex].insert(CrPair<CrHash, CrGPUTimingRequest>(hash, request));

	return request;
}

CrGPUInterval CrGPUTimingQueryTracker::GetResultForFrame(CrHash hash)
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

ICrGPUQueryPool* CrGPUTimingQueryTracker::GetCurrentQueryPool() const
{
	return m_queryPools[m_currentPoolIndex].get();
}

ICrGPUQueryPool* CrGPUTimingQueryTracker::GetOldestQueryPool() const
{
	return m_queryPools[(m_currentPoolIndex + 1) % m_maxFrames].get();
}