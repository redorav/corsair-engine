#include "Graphics/CrRendering_pch.h"

#include "Graphics/IGPUQueryPool.h"
#include "Graphics/ICommandBuffer.h"

#include "Math/CrMath.h"

namespace crgfx
{
	IGPUQueryPool::IGPUQueryPool(crgfx::IDevice* renderDevice, const GPUQueryPoolDescriptor& descriptor) : GPUAutoDeletable(renderDevice)
		, m_descriptor(descriptor)
		, m_resolved(false)
		, m_querySize(0) // Gets calculated by each platform
		, m_currentQuery(0)
		, m_timestampPeriod(1.0) // Gets calculated per platform
	{}

	void IGPUQueryPool::Resolve(crgfx::ICommandBuffer* commandBuffer)
	{
		commandBuffer->ResolveGPUQueries(this, 0, m_currentQuery);
		m_resolved = true;
	}

	void IGPUQueryPool::GetTimestampData(GPUTimestamp* timingData, uint32_t timingCount)
	{
		CrAssertMsg(m_descriptor.type == crgfx::QueryType::Timestamp, "Wrong query type");

		if (m_resolved)
		{
			uint32_t resolveCount = CrMin(m_currentQuery, timingCount);
			GetTimingDataPS(timingData, resolveCount);
		}
	}

	void IGPUQueryPool::Reset(crgfx::ICommandBuffer* commandBuffer)
	{
		commandBuffer->ResetGPUQueries(this, 0, m_descriptor.count);
		m_currentQuery = 0;
	}

	double IGPUQueryPool::GetDuration(GPUTimestamp startTime, GPUTimestamp endTime) const
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
};