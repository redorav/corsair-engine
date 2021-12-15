#pragma once

#include "CrRenderingForwardDeclarations.h"
#include "Core/CrMacros.h"
#include "Core/Logging/ICrDebug.h"
#include "Core/CrTypedId.h"

struct CrGPUTiming
{
	double nanoseconds; // Represents a time difference in nanoseconds
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
class ICrGPUQueryPool
{
public:

	ICrGPUQueryPool(ICrRenderDevice* renderDevice, const CrGPUQueryPoolDescriptor& descriptor)
		: m_renderDevice(renderDevice)
		, m_descriptor(descriptor)
		, m_currentQuery(0)
		, m_querySize(0) // Gets calculated by each platform
		, m_resolved(false)
	{}

	virtual ~ICrGPUQueryPool() {}

	uint32_t GetActiveQueryCount() const
	{
		return m_currentQuery;
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

	void GetTimingData(CrGPUTiming* timingData, uint32_t count);

	void Reset(ICrCommandBuffer* commandBuffer);

protected:

	virtual void GetTimingDataPS(CrGPUTiming* timingData, uint32_t count) = 0;

	virtual void GetOcclusionDataPS(CrGPUOcclusion* data, uint32_t count) = 0;

	CrGPUQueryPoolDescriptor m_descriptor;
	
	bool m_resolved;

	uint32_t m_querySize;

	uint32_t m_currentQuery;

	ICrRenderDevice* m_renderDevice;
};