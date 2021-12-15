#include "CrRendering_pch.h"

#include "ICrGPUQueryPool.h"
#include "ICrCommandBuffer.h"

void ICrGPUQueryPool::Resolve(ICrCommandBuffer* commandBuffer)
{
	commandBuffer->ResolveGPUQueries(this, 0, m_currentQuery);
	m_resolved = true;
}

void ICrGPUQueryPool::GetTimingData(CrGPUTiming* timingData, uint32_t timingCount)
{
	CrAssertMsg(m_descriptor.type == cr3d::QueryType::Timing, "Wrong query type");

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
