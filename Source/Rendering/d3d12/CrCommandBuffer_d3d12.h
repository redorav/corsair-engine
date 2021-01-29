#pragma once

#include "ICrCommandBuffer.h"
#include "CrGPUBuffer_d3d12.h"
#include "CrD3D12.h"
#include "CrRendering.h"

class CrVertexBufferCommon;
class CrIndexBufferCommon;
class ICrGraphicsPipeline;
class CrTextureD3D12;

class CrCommandBufferD3D12 final : public ICrCommandBuffer
{
public:

	CrCommandBufferD3D12(ICrCommandQueue* commandQueue);

private:

	virtual void BeginPS() override;

	virtual void EndPS() override;

	virtual void BindGraphicsPipelineStatePS(const ICrGraphicsPipeline* pipelineState) override;

	virtual void ClearRenderTargetPS(const ICrTexture* renderTarget, const float4& color, uint32_t level, uint32_t slice, uint32_t levelCount, uint32_t sliceCount) override;

	virtual void DrawPS(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;

	//void DrawIndirectPS(CrIndirectArgs* indirectArgs, uint32_t indirectArgsOffset) override;

	virtual void DrawIndexedPS(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) override;

	//void DrawIndexedIndirectPS(CrIndirectArgs* indirectArgs, uint32_t indirectArgsOffset) override;

	virtual void DispatchPS(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) override;

	//void DispatchIndirectPS(CrIndirectArgs* indirectArgs) override;

	virtual void BeginDebugEventPS(const char* eventName, const float4& color) override;

	virtual void EndDebugEventPS() override;

	virtual void TransitionTexturePS(const ICrTexture* texture, cr3d::ResourceState::T initialState, cr3d::ResourceState::T destinationState) override;

	virtual void UpdateResourceTablesPS() override;

	virtual void BeginRenderPassPS(const ICrRenderPass* renderPass, const ICrFramebuffer* frameBuffer, const CrRenderPassBeginParams& renderPassParams) override;

	virtual void EndRenderPassPS() override;
};

inline void CrCommandBufferD3D12::BindGraphicsPipelineStatePS(const ICrGraphicsPipeline* pipelineState)
{
	
}

inline void CrCommandBufferD3D12::DrawPS(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	
}

inline void CrCommandBufferD3D12::DrawIndexedPS(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	
}

inline void CrCommandBufferD3D12::DispatchPS(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
{
	
}

inline void CrCommandBufferD3D12::BeginDebugEventPS(const char* eventName, const float4& color)
{
	
}

inline void CrCommandBufferD3D12::EndDebugEventPS()
{
	
}
