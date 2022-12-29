#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrGPUDeletable.h"

#include "Core/CrMacros.h"
#include "Core/Logging/ICrDebug.h"
#include "Core/CrTypedId.h"

struct CrGPUTimestamp
{
	CrGPUTimestamp() { ticks = 0; }
	CrGPUTimestamp(uint64_t ticks) : ticks(ticks) {}

	uint64_t ticks;
};

struct CrGPUOcclusion
{
	uint64_t visibilitySamples; // Number of samples passed by this query
};

struct CrGPUQueryPoolDescriptor
{
	CrGPUQueryPoolDescriptor(cr3d::QueryType type, uint32_t count) : type(type), count(count) {}

	cr3d::QueryType type;
	uint32_t count;
};

// Pool that contains queries of the type passed in as a template
// The template must be one of the interfaces provided above
class ICrGPUQueryPool : public CrGPUDeletable
{
public:

	ICrGPUQueryPool(ICrRenderDevice* renderDevice, const CrGPUQueryPoolDescriptor& descriptor);

	virtual ~ICrGPUQueryPool() {}

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

	cr3d::QueryType GetType() const
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
	void Resolve(ICrCommandBuffer* commandBuffer);

	void GetTimestampData(CrGPUTimestamp* timingData, uint32_t count);

	void Reset(ICrCommandBuffer* commandBuffer);

	// Computes the duration in nanoseconds between two timestamps
	double GetDuration(CrGPUTimestamp startTime, CrGPUTimestamp endTime) const;

protected:

	virtual void GetTimingDataPS(CrGPUTimestamp* timingData, uint32_t count) = 0;

	virtual void GetOcclusionDataPS(CrGPUOcclusion* data, uint32_t count) = 0;

	CrGPUQueryPoolDescriptor m_descriptor;
	
	bool m_resolved;

	// Size of each query in bytes
	uint32_t m_querySize;

	uint32_t m_currentQuery;

	// Not all platforms work the same way, so this is just a multiplier that takes from
	// raw ticks to nanoseconds. In many common cases it will just be 1.0
	double m_timestampPeriod;
};