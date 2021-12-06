#pragma once

#include <d3d12.h>

namespace CrCommandQueueType { enum T : uint32_t; }

namespace crd3d
{
	DXGI_FORMAT GetDXGIFormat(cr3d::DataFormat::T format);

	D3D12_TEXTURE_ADDRESS_MODE GetD3DAddressMode(cr3d::AddressMode addressMode);
	
	D3D12_FILTER GetD3DFilter(cr3d::Filter minFilter, cr3d::Filter magFilter, cr3d::Filter mipFilter, bool anisotropic, bool comparison);

	D3D12_BLEND_OP GetD3DBlendOp(cr3d::BlendOp blendOp);

	D3D12_BLEND GetD3DBlendFactor(cr3d::BlendFactor blendFactor);

	D3D12_COMPARISON_FUNC GetD3DCompareOp(cr3d::CompareOp compareOp);

	D3D12_STENCIL_OP GetD3DStencilOp(cr3d::StencilOp stencilOp);

	uint32_t GetD3D12SampleCount(cr3d::SampleCount sampleCount);

	D3D_PRIMITIVE_TOPOLOGY GetD3D12PrimitiveTopology(cr3d::PrimitiveTopology primitiveTopology);

	D3D12_PRIMITIVE_TOPOLOGY_TYPE GetD3D12PrimitiveTopologyType(cr3d::PrimitiveTopology primitiveTopology);

	D3D12_FILL_MODE GetD3D12PolygonFillMode(cr3d::PolygonFillMode fillMode);

	D3D12_CULL_MODE GetD3D12PolygonCullMode(cr3d::PolygonCullMode cullMode);

	D3D12_COMMAND_LIST_TYPE GetD3D12CommandQueueType(CrCommandQueueType::T commandQueueType);

	// A shader-visible heap will have two handles, CPU and GPU. The CPU handle is what we use to update
	// the data in the descriptor, the GPU handle is what we use to bind it to the command buffer
	struct DescriptorD3D12
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	};
}
