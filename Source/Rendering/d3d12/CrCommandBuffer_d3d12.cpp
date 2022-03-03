#include "CrRendering_pch.h"

#include "CrCommandBuffer_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrTexture_d3d12.h"
#include "CrSampler_d3d12.h"

#include "Core/Logging/ICrDebug.h"

CrCommandBufferD3D12::CrCommandBufferD3D12(ICrRenderDevice* renderDevice, CrCommandQueueType::T queueType)
	: ICrCommandBuffer(renderDevice, queueType)
{
	m_d3d12Device = static_cast<CrRenderDeviceD3D12*>(renderDevice)->GetD3D12Device();

	D3D12_COMMAND_LIST_TYPE d3dc12CommandListType = crd3d::GetD3D12CommandQueueType(queueType);

	m_d3d12Device->CreateCommandAllocator(d3dc12CommandListType, IID_PPV_ARGS(&m_d3d12CommandAllocator));

	m_d3d12Device->CreateCommandList(0, d3dc12CommandListType, m_d3d12CommandAllocator, nullptr, IID_PPV_ARGS(&m_d3d12GraphicsCommandList));
}

void CrCommandBufferD3D12::BeginRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor)
{
	unused_parameter(renderPassDescriptor);
}

void CrCommandBufferD3D12::EndRenderPassPS()
{
	
}

void CrCommandBufferD3D12::ResetGPUQueriesPS(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count)
{
	unused_parameter(queryPool);
	unused_parameter(start);
	unused_parameter(count);
}

void CrCommandBufferD3D12::ResolveGPUQueriesPS(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count)
{
	const CrGPUQueryPoolD3D12* d3d12QueryPool = static_cast<const CrGPUQueryPoolD3D12*>(queryPool);
	const CrHardwareGPUBufferD3D12* d3d12GPUBuffer = static_cast<const CrHardwareGPUBufferD3D12*>(d3d12QueryPool->GetResultsBuffer());

	m_d3d12GraphicsCommandList->ResolveQueryData(d3d12QueryPool->GetD3D12QueryHeap(), D3D12_QUERY_TYPE_TIMESTAMP, start, count, d3d12GPUBuffer->GetD3D12Buffer(), start * sizeof(uint64_t));
}

void CrCommandBufferD3D12::FlushGraphicsRenderStatePS()
{

}

void CrCommandBufferD3D12::FlushComputeRenderStatePS()
{

}

void CrCommandBufferD3D12::BeginPS()
{
	
}

void CrCommandBufferD3D12::ClearRenderTargetPS(const ICrTexture* renderTarget, const float4& color, uint32_t level, uint32_t slice, uint32_t levelCount, uint32_t sliceCount)
{
	unused_parameter(renderTarget);
	unused_parameter(color);
	unused_parameter(level);
	unused_parameter(slice);
	unused_parameter(levelCount);
	unused_parameter(sliceCount);
}

void CrCommandBufferD3D12::EndPS()
{
	
}
