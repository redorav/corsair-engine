#pragma once

#include "Graphics/IGPUQueryPool.h"

namespace crgfx
{
	class IDevice;

	class CrGPUQueryPoolD3D12 final : public IGPUQueryPool
	{
	public:

		CrGPUQueryPoolD3D12(crgfx::IDevice* renderDevice, const GPUQueryPoolDescriptor& descriptor);

		~CrGPUQueryPoolD3D12();

		ID3D12QueryHeap* GetD3D12QueryHeap() const { return m_d3d12QueryHeap; }

		const ICrHardwareGPUBuffer* GetResultsBuffer() const { return m_queryBuffer.get(); }

	protected:

		virtual void GetTimingDataPS(GPUTimestamp* timingData, uint32_t timingCount) override;

		virtual void GetOcclusionDataPS(GPUOcclusion* occlusionData, uint32_t count) override;

		// Use the platform-independent code so we don't have to rewrite it
		CrHardwareGPUBufferHandle m_queryBuffer;

		ID3D12QueryHeap* m_d3d12QueryHeap;
	};
};