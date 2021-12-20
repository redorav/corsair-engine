#pragma once

#include "Rendering/ICrGPUQueryPool.h"

class ICrRenderDevice;

class CrGPUQueryPoolD3D12 final : public ICrGPUQueryPool
{
public:

	CrGPUQueryPoolD3D12(ICrRenderDevice* renderDevice, const CrGPUQueryPoolDescriptor& descriptor);

	~CrGPUQueryPoolD3D12();

	ID3D12QueryHeap* GetD3D12QueryHeap() const { return m_d3d12QueryHeap; }

	const ICrHardwareGPUBuffer* GetResultsBuffer() const { return m_queryBuffer.get(); }

protected:

	virtual void GetTimingDataPS(CrGPUTimestamp* timingData, uint32_t timingCount) override;

	virtual void GetOcclusionDataPS(CrGPUOcclusion* occlusionData, uint32_t count) override;

	// Use the platform-independent code so we don't have to rewrite it
	CrUniquePtr<ICrHardwareGPUBuffer> m_queryBuffer;

	ID3D12QueryHeap* m_d3d12QueryHeap;
};