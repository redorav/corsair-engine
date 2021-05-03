#include "CrRendering_pch.h"

#include "CrCommandQueue_d3d12.h"
#include "CrCommandBuffer_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrTexture_d3d12.h"
#include "CrSampler_d3d12.h"

#include "Core/Containers/CrArray.h"

#include "Core/Logging/ICrDebug.h"

CrCommandBufferD3D12::CrCommandBufferD3D12(ICrCommandQueue* commandQueue)
	: ICrCommandBuffer(commandQueue)
{
	
}

void CrCommandBufferD3D12::BeginRenderPassPS(const CrRenderPassDescriptor& descriptor)
{
	unused_parameter(descriptor);
}

void CrCommandBufferD3D12::EndRenderPassPS()
{
	
}

void CrCommandBufferD3D12::TextureBarrierPS(const ICrTexture* texture, const CrTextureBarrier& resourceTransition)
{
	unused_parameter(texture);
	unused_parameter(resourceTransition);
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
