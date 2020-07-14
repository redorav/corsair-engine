#include "CrRendering_pch.h"

#include "CrCommandQueue_d3d12.h"
#include "CrCommandBuffer_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrTexture_d3d12.h"
#include "CrSampler_d3d12.h"
#include "CrShaderManager_d3d12.h"
#include "CrShaderGen.h" // TODO remove
#include "CrRenderPass_d3d12.h"
#include "CrFramebuffer_d3d12.h"

#include "Core/Containers/CrArray.h"

#include "Core/Logging/ICrDebug.h"

CrCommandBufferD3D12::CrCommandBufferD3D12(ICrCommandQueue* commandQueue)
	: ICrCommandBuffer(commandQueue)
{
	
}

void CrCommandBufferD3D12::UpdateResourceTablesPS()
{
	
}

void CrCommandBufferD3D12::BeginRenderPassPS(const ICrRenderPass* renderPass, const ICrFramebuffer* frameBuffer, const CrRenderPassBeginParams& renderPassParams)
{
	
}

void CrCommandBufferD3D12::EndRenderPassPS(const ICrRenderPass* /* renderPass*/)
{
	
}

void CrCommandBufferD3D12::TransitionTexturePS(const ICrTexture* texture, cr3d::ResourceState::T initialState, cr3d::ResourceState::T destinationState)
{
	
}

void CrCommandBufferD3D12::BeginPS()
{
	
}

void CrCommandBufferD3D12::ClearRenderTargetPS(const ICrTexture* renderTarget, const float4& color, uint32_t level, uint32_t slice, uint32_t levelCount, uint32_t sliceCount)
{
	
}

void CrCommandBufferD3D12::EndPS()
{
	
}