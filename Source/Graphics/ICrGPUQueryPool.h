#pragma once

#include "Graphics/CrGraphicsForwardDeclarations.h"
#include "Graphics/CrGPUDeletable.h"

#include "Core/CrMacros.h"
#include "Core/Logging/ICrDebug.h"
#include "Core/CrTypedId.h"

namespace crgfx
{
	struct GPUTimestamp
	{
		GPUTimestamp() { ticks = 0; }
		GPUTimestamp(uint64_t ticks) : ticks(ticks) {}

		uint64_t ticks;
	};

	struct GPUOcclusion
	{
		uint64_t visibilitySamples; // Number of samples passed by this query
	};

	struct GPUQueryPoolDescriptor
	{
		GPUQueryPoolDescriptor(crgfx::QueryType type, uint32_t count) : type(type), count(count) {}

		crgfx::QueryType type;
		uint32_t count;
	};

	// Pool that contains queries of the type passed in as a template
	// The template must be one of the interfaces provided above
	class IGPUQueryPool : public CrGPUAutoDeletable
	{
	public:

		IGPUQueryPool(crgfx::IDevice* renderDevice, const GPUQueryPoolDescriptor& descriptor);

		virtual ~IGPUQueryPool() {}

		uint32_t GetActiveQueryCount() const
		{
			return m_currentQuery;
		}

		uint32_t GetTotalQueryCount() const
		{
			return m_descriptor.count;
		}

		uint32_t GetQuerySize() const
		{
			return m_querySize;
		}

		crgfx::QueryType GetType() const
		{
			return m_descriptor.type;
		}

		// Allocate a GPU query id. We can use this to begin or end a query on a command buffer
		CrGPUQueryId Allocate()
		{
			// TODO Check bounds
			uint32_t currentQuery = m_currentQuery;
			m_currentQuery++;
			return CrGPUQueryId(currentQuery);
		}

		// Resolve all pending queries using this command buffer to copy data across
		// This doesn't mean the data is available now, it just means the GPU will
		// copy it across as soon as it can. Then we can use the GetData() function
		// to retrieve it on the CPU
		void Resolve(crgfx::ICommandBuffer* commandBuffer);

		void GetTimestampData(GPUTimestamp* timingData, uint32_t count);

		void Reset(crgfx::ICommandBuffer* commandBuffer);

		// Computes the duration in nanoseconds between two timestamps
		double GetDuration(GPUTimestamp startTime, GPUTimestamp endTime) const;

	protected:

		virtual void GetTimingDataPS(GPUTimestamp* timingData, uint32_t count) = 0;

		virtual void GetOcclusionDataPS(GPUOcclusion* data, uint32_t count) = 0;

		GPUQueryPoolDescriptor m_descriptor;

		bool m_resolved;

		// Size of each query in bytes
		uint32_t m_querySize;

		uint32_t m_currentQuery;

		// Not all platforms work the same way, so this is the multiplier that takes us from raw ticks to nanoseconds. In common cases it will just be 1.0, but it could be 10.0 depending on
		// the timer resolution of the current device we're working with
		double m_timestampPeriod;
	};
};