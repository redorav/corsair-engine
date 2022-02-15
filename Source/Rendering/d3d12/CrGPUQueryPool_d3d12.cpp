#include "CrRendering_pch.h"

#include "CrRenderDevice_d3d12.h"
#include "CrGPUQueryPool_d3d12.h"
#include "CrGPUBuffer_d3d12.h"

#include "Core/Logging/ICrDebug.h"

D3D12_QUERY_HEAP_TYPE GetD3D12QueryType(cr3d::QueryType queryType)
{
	switch (queryType)
	{
		case cr3d::QueryType::Timestamp: return D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		case cr3d::QueryType::Occlusion: return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
		default: return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
	}
}

CrGPUQueryPoolD3D12::CrGPUQueryPoolD3D12(ICrRenderDevice* renderDevice, const CrGPUQueryPoolDescriptor& descriptor) : ICrGPUQueryPool(renderDevice, descriptor)
{
	CrRenderDeviceD3D12* d3d12RenderDevice = static_cast<CrRenderDeviceD3D12*>(renderDevice);

	m_querySize = sizeof(uint64_t);

	D3D12_QUERY_HEAP_DESC queryHeapDescriptor;
	queryHeapDescriptor.Type = GetD3D12QueryType(descriptor.type);
	queryHeapDescriptor.Count = descriptor.count;
	queryHeapDescriptor.NodeMask = 0;
	HRESULT hResult = d3d12RenderDevice->GetD3D12Device()->CreateQueryHeap(&queryHeapDescriptor, IID_PPV_ARGS(&m_d3d12QueryHeap));

	CrAssertMsg(hResult == S_OK, "Failed to create query pool");

	CrHardwareGPUBufferDescriptor queryBufferDescriptor(cr3d::BufferUsage::TransferDst, cr3d::BufferAccess::GPUWriteCPURead, descriptor.count, m_querySize);

	m_queryBuffer = CrUniquePtr<ICrHardwareGPUBuffer>(d3d12RenderDevice->CreateHardwareGPUBuffer(queryBufferDescriptor));
}

CrGPUQueryPoolD3D12::~CrGPUQueryPoolD3D12()
{
	m_d3d12QueryHeap->Release();
}

void CrGPUQueryPoolD3D12::GetTimingDataPS(CrGPUTimestamp* timingData, uint32_t timingCount)
{
	uint64_t* memory = (uint64_t*)m_queryBuffer->Lock();
	{
		memcpy(timingData, memory, timingCount * sizeof(timingData));
	}
	m_queryBuffer->Unlock();
}

void CrGPUQueryPoolD3D12::GetOcclusionDataPS(CrGPUOcclusion* occlusionData, uint32_t occlusionCount)
{
	uint64_t* memory = (uint64_t*)m_queryBuffer->Lock();
	{
		for (uint32_t i = 0; i < occlusionCount; i++)
		{
			occlusionData[i].visibilitySamples = memory[i];
		}
	}
	m_queryBuffer->Unlock();
}
