#pragma once

#include "d3d12.h"

#include "Graphics/CrGraphicsForwardDeclarations.h"

namespace crgfx
{
	namespace CommandQueueType { enum T : uint32_t; }
}

namespace crd3d
{
	struct TextureBarrierInfoD3D12
	{
		D3D12_BARRIER_SYNC sync     = D3D12_BARRIER_SYNC_NONE;
		D3D12_BARRIER_ACCESS access = D3D12_BARRIER_ACCESS_COMMON;
	};

	struct BufferBarrierInfoD3D12
	{
		D3D12_BARRIER_SYNC sync = D3D12_BARRIER_SYNC_NONE;
		D3D12_BARRIER_ACCESS access = D3D12_BARRIER_ACCESS_COMMON;
	};

	DXGI_FORMAT GetDXGIFormat(crgfx::DataFormat::T format);

	D3D12_TEXTURE_ADDRESS_MODE GetD3DAddressMode(crgfx::AddressMode addressMode);
	
	D3D12_FILTER GetD3DFilter(crgfx::Filter minFilter, crgfx::Filter magFilter, crgfx::Filter mipFilter, bool anisotropic, bool comparison);

	D3D12_BLEND_OP GetD3DBlendOp(crgfx::BlendOp blendOp);

	D3D12_BLEND GetD3DBlendFactor(crgfx::BlendFactor blendFactor);

	D3D12_COMPARISON_FUNC GetD3DCompareOp(crgfx::CompareOp compareOp);

	D3D12_STENCIL_OP GetD3DStencilOp(crgfx::StencilOp stencilOp);

	uint32_t GetD3D12SampleCount(crgfx::SampleCount sampleCount);

	D3D_PRIMITIVE_TOPOLOGY GetD3D12PrimitiveTopology(crgfx::PrimitiveTopology primitiveTopology);

	D3D12_PRIMITIVE_TOPOLOGY_TYPE GetD3D12PrimitiveTopologyType(crgfx::PrimitiveTopology primitiveTopology);

	D3D12_FILL_MODE GetD3D12PolygonFillMode(crgfx::PolygonFillMode fillMode);

	D3D12_CULL_MODE GetD3D12PolygonCullMode(crgfx::PolygonCullMode cullMode);

	D3D12_COMMAND_LIST_TYPE GetD3D12CommandQueueType(crgfx::CommandQueueType::T commandQueueType);

	D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE GetD3D12BeginningAccessType(crgfx::RenderTargetLoadOp loadOp);

	D3D12_RENDER_PASS_ENDING_ACCESS_TYPE GetD3D12EndingAccessType(crgfx::RenderTargetStoreOp storeOp);

	TextureBarrierInfoD3D12 GetD3D12TextureBarrierInfo(const crgfx::TextureState& textureState);

	BufferBarrierInfoD3D12 GetD3D12BufferBarrierInfo(crgfx::BufferState::T bufferState, crgfx::ShaderStageFlags::T shaderStages);

	D3D12_BARRIER_LAYOUT GetD3D12BarrierTextureLayout(const crgfx::TextureLayout::T textureLayout);

	// https://github.com/microsoft/DirectX-Headers/blob/main/include/directx/d3dx12.h
	// Copied from D3D12CalcSubresource
	constexpr UINT CalculateSubresource(UINT MipSlice, UINT ArraySlice, UINT PlaneSlice, UINT MipLevels, UINT ArraySize)
	{
		return MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize;
	}

	// Copied from D3D12DecomposeSubresource
	constexpr void DecomposeSubresource(UINT Subresource, UINT MipLevels, UINT ArraySize, UINT& MipSlice, UINT& ArraySlice, UINT& PlaneSlice) noexcept
	{
		MipSlice = Subresource % MipLevels;
		ArraySlice = (Subresource / MipLevels) % ArraySize;
		PlaneSlice = Subresource / (MipLevels * ArraySize);
	}

	// A shader-visible heap will have two handles, CPU and GPU. The CPU handle is what we use to update
	// the data in the descriptor, the GPU handle is what we use to bind it to the command buffer

	struct DescriptorD3D12
	{
		DescriptorD3D12 operator + (size_t offset) const
		{
			DescriptorD3D12 descriptor = *this;
			descriptor.cpuHandle.ptr += offset;
			descriptor.gpuHandle.ptr += offset;
			return descriptor;
		}

		DescriptorD3D12& operator += (size_t offset)
		{
			*this = *this + offset;
			return *this;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	};
}
