#pragma once

#include "Rendering/ICrCommandBuffer.h"
#include "Rendering/CrRendering.h"
#include "CrGPUBuffer_d3d12.h"
#include "CrGPUQueryPool_d3d12.h"
#include "CrDescriptorHeap_d3d12.h"
#include "CrD3D12.h"

#include "Core/CrMacros.h"

class CrTextureD3D12;

typedef CrFixedVector<D3D12_RESOURCE_BARRIER, CrRenderPassDescriptor::MaxTransitionCount + cr3d::MaxRenderTargets> CrBarrierVectorD3D12;

class CrCommandBufferD3D12 final : public ICrCommandBuffer
{
public:

	CrCommandBufferD3D12(ICrRenderDevice* renderDevice, const CrCommandBufferDescriptor& descriptor);

	ID3D12GraphicsCommandList4* GetD3D12CommandList() const { return m_d3d12GraphicsCommandList; }

private:

	virtual void BeginPS() override;

	virtual void EndPS() override;

	virtual void ClearRenderTargetPS(const ICrTexture* renderTarget, const float4& color, uint32_t mip, uint32_t slice, uint32_t mipCount, uint32_t sliceCount) override;

	virtual void DrawPS(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;

	virtual void DrawIndexedPS(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) override;

	virtual void DispatchPS(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) override;

	virtual void DrawIndirectPS(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count) override;

	virtual void DrawIndexedIndirectPS(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count) override;

	virtual void DispatchIndirectPS(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset) override;

	virtual void BeginDebugEventPS(const char* eventName, const float4& color) override;

	virtual void EndDebugEventPS() override;

	virtual void InsertDebugMarkerPS(const char* markerName, const float4& color) override;

	virtual void BeginTimestampQueryPS(const ICrGPUQueryPool* queryPool, CrGPUQueryId query) override;

	virtual void EndTimestampQueryPS(const ICrGPUQueryPool* queryPool, CrGPUQueryId query) override;

	virtual void ResetGPUQueriesPS(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count) override;

	virtual void ResolveGPUQueriesPS(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count) override;

	virtual void FlushGraphicsRenderStatePS() override;

	virtual void FlushComputeRenderStatePS() override;

	virtual void BeginRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor) override;

	virtual void EndRenderPassPS() override;

	void ProcessTextureAndBufferBarriers
	(
		const CrRenderPassDescriptor::BufferTransitionVector& buffers, 
		const CrRenderPassDescriptor::TextureTransitionVector& textures,
		CrBarrierVectorD3D12& transitions
	);

	void ProcessRenderTargetBarrier
	(
		const CrRenderTargetDescriptor& renderTargetDescriptor, 
		const cr3d::TextureState& initialState,
		const cr3d::TextureState& finalState,
		CrBarrierVectorD3D12& resourceBarriers
	);

	void WriteCBV(const ConstantBufferBinding& binding, crd3d::DescriptorD3D12 cbvHandle);

	void WriteTextureSRV(const ICrTexture* texture, crd3d::DescriptorD3D12 srvHandle);

	void WriteSamplerView(const ICrSampler* sampler, crd3d::DescriptorD3D12 samplerHandle);

	void WriteRWTextureUAV(const RWTextureBinding& rwTextureBinding, crd3d::DescriptorD3D12 uavHandle);

	void WriteStorageBufferSRV(const StorageBufferBinding& binding, crd3d::DescriptorD3D12 srvHandle);

	void WriteRWStorageBufferUAV(const StorageBufferBinding& binding, crd3d::DescriptorD3D12 uavHandle);

	D3D12_PRIMITIVE_TOPOLOGY m_primitiveTopology;

	CrDescriptorStreamD3D12 m_CBV_SRV_UAV_ShaderVisibleDescriptorStream;

	CrDescriptorStreamD3D12 m_CBV_SRV_UAV_DescriptorStream;

	CrDescriptorStreamD3D12 m_samplerShaderVisibleDescriptorStream;

	CrDescriptorStreamD3D12 m_samplerDescriptorStream;

	ID3D12DescriptorHeap* m_CBV_SRV_UAV_DescriptorHeap;

	ID3D12DescriptorHeap* m_samplerDescriptorHeap;

	ID3D12CommandAllocator* m_d3d12CommandAllocator;

	ID3D12GraphicsCommandList4* m_d3d12GraphicsCommandList;
};

inline void CrCommandBufferD3D12::DrawPS(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	m_d3d12GraphicsCommandList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
}

inline void CrCommandBufferD3D12::DrawIndexedPS(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	m_d3d12GraphicsCommandList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

inline void CrCommandBufferD3D12::DispatchPS(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
{
	m_d3d12GraphicsCommandList->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

inline void CrCommandBufferD3D12::BeginTimestampQueryPS(const ICrGPUQueryPool* queryPool, CrGPUQueryId query)
{
	const CrGPUQueryPoolD3D12* d3d12QueryPool = static_cast<const CrGPUQueryPoolD3D12*>(queryPool);
	m_d3d12GraphicsCommandList->EndQuery(d3d12QueryPool->GetD3D12QueryHeap(), D3D12_QUERY_TYPE_TIMESTAMP, query.id);
}

inline void CrCommandBufferD3D12::EndTimestampQueryPS(const ICrGPUQueryPool* queryPool, CrGPUQueryId query)
{
	BeginTimestampQueryPS(queryPool, query);
}