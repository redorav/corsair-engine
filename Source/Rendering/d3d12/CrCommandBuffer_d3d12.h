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

	virtual void BeginPS() final override;

	virtual void EndPS() final override;

	virtual void SetViewportPS(const CrViewport& viewport) final override;

	virtual void SetScissorPS(uint32_t topLeftX, uint32_t topLeftY, uint32_t width, uint32_t height) final override;

	virtual void BindIndexBufferPS(const ICrHardwareGPUBuffer* indexBuffer) final override;

	virtual void BindVertexBuffersPS(const ICrHardwareGPUBuffer* vertexBuffer, uint32_t bindPoint) final override;

	virtual void BindGraphicsPipelineStatePS(const ICrGraphicsPipeline* pipelineState) final override;

	virtual void ClearRenderTargetPS(const ICrTexture* renderTarget, const float4& color, uint32_t level, uint32_t slice, uint32_t levelCount, uint32_t sliceCount) final override;

	virtual void DrawPS(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) final override;

	//void DrawIndirectPS(CrIndirectArgs* indirectArgs, uint32_t indirectArgsOffset) final override;

	virtual void DrawIndexedPS(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) final override;

	//void DrawIndexedIndirectPS(CrIndirectArgs* indirectArgs, uint32_t indirectArgsOffset) final override;

	virtual void DispatchPS(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) final override;

	//void DispatchIndirectPS(CrIndirectArgs* indirectArgs) final override;

	virtual void BeginDebugEventPS(const char* eventName, const float4& color) final override;

	virtual void EndDebugEventPS() final override;

	virtual void TransitionTexturePS(const ICrTexture* texture, cr3d::ResourceState::T initialState, cr3d::ResourceState::T destinationState) final override;

	virtual void UpdateResourceTablesPS() final override;

	virtual void BeginRenderPassPS(const ICrRenderPass* renderPass, const ICrFramebuffer* frameBuffer, const CrRenderPassBeginParams& renderPassParams) final override;

	virtual void EndRenderPassPS(const ICrRenderPass* renderPass) final override;
};

inline void CrCommandBufferD3D12::SetViewportPS(const CrViewport& viewport)
{
	
}

inline void CrCommandBufferD3D12::SetScissorPS(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	
}

inline void CrCommandBufferD3D12::BindIndexBufferPS(const ICrHardwareGPUBuffer* indexBuffer)
{
	
}

inline void CrCommandBufferD3D12::BindVertexBuffersPS(const ICrHardwareGPUBuffer* vertexBuffer, uint32_t bindPoint)
{
	
}

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
