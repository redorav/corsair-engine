#include "Rendering/CrRendering_pch.h"

#include "Rendering/ICrGPUQueryPool.h"
#include "Rendering/ICrCommandBuffer.h"

#include "Math/CrMath.h"

ICrGPUQueryPool::ICrGPUQueryPool(ICrRenderDevice* renderDevice, const CrGPUQueryPoolDescriptor& descriptor) : CrGPUAutoDeletable(renderDevice)
, m_descriptor(descriptor)
, m_resolved(false)
, m_querySize(0) // Gets calculated by each platform
, m_currentQuery(0)
, m_timestampPeriod(1.0) // Gets calculated per platform
{}

void ICrGPUQueryPool::Resolve(ICrCommandBuffer* commandBuffer)
{
	commandBuffer->ResolveGPUQueries(this, 0, m_currentQuery);
	m_resolved = true;
}

void ICrGPUQueryPool::GetTimestampData(CrGPUTimestamp* timingData, uint32_t timingCount)
{
	CrAssertMsg(m_descriptor.type == cr3d::QueryType::Timestamp, "Wrong query type");

	if (m_resolved)
	{
		uint32_t resolveCount = CrMin(m_currentQuery, timingCount);
		GetTimingDataPS(timingData, resolveCount);
	}
}

void ICrGPUQueryPool::Reset(ICrCommandBuffer* commandBuffer)
{
	commandBuffer->ResetGPUQueries(this, 0, m_descriptor.count);
	m_currentQuery = 0;
}

double ICrGPUQueryPool::GetDuration(CrGPUTimestamp startTime, CrGPUTimestamp endTime) const
{
	uint64_t tickDelta = 0;

	// Check for overflow
	if (endTime.ticks >= startTime.ticks)
	{
		tickDelta = endTime.ticks - startTime.ticks;
	}
	else
	{
		tickDelta = (uint64_t)(-1) - startTime.ticks + endTime.ticks;
	}

	return tickDelta * m_timestampPeriod;
}
