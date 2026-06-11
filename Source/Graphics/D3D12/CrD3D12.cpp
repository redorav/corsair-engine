#include "Graphics/CrRendering_pch.h"
#include "CrD3D12.h"

#include "Core/Logging/ICrDebug.h"

#include "Graphics/CrRendering.h"

DXGI_FORMAT crd3d::GetDXGIFormat(crgfx::DataFormat::T format)
{
	switch (format)
	{
		// 8-bit formats
		case crgfx::DataFormat::R8_Unorm:          return DXGI_FORMAT_R8_UNORM;
		case crgfx::DataFormat::R8_Snorm:          return DXGI_FORMAT_R8_SNORM;
		case crgfx::DataFormat::R8_Uint:           return DXGI_FORMAT_R8_UINT;
		case crgfx::DataFormat::R8_Sint:           return DXGI_FORMAT_R8_SINT;

		case crgfx::DataFormat::RG8_Unorm:         return DXGI_FORMAT_R8G8_UNORM;
		case crgfx::DataFormat::RG8_Snorm:         return DXGI_FORMAT_R8G8_SNORM;
		case crgfx::DataFormat::RG8_Uint:          return DXGI_FORMAT_R8G8_UINT;
		case crgfx::DataFormat::RG8_Sint:          return DXGI_FORMAT_R8G8_SINT;

		case crgfx::DataFormat::RGBA8_Unorm:       return DXGI_FORMAT_R8G8B8A8_UNORM;
		case crgfx::DataFormat::RGBA8_Snorm:       return DXGI_FORMAT_R8G8B8A8_SNORM;
		case crgfx::DataFormat::RGBA8_Uint:        return DXGI_FORMAT_R8G8B8A8_UINT;
		case crgfx::DataFormat::RGBA8_Sint:        return DXGI_FORMAT_R8G8B8A8_SINT;
		case crgfx::DataFormat::RGBA8_SRGB:        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		case crgfx::DataFormat::BGRA8_Unorm:       return DXGI_FORMAT_B8G8R8A8_UNORM;
		case crgfx::DataFormat::BGRA8_SRGB:        return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

		// 16-bit integer formats
		case crgfx::DataFormat::R16_Unorm:         return DXGI_FORMAT_R16_UNORM;
		case crgfx::DataFormat::R16_Snorm:         return DXGI_FORMAT_R16_SNORM;
		case crgfx::DataFormat::R16_Uint:          return DXGI_FORMAT_R16_UINT;
		case crgfx::DataFormat::R16_Sint:          return DXGI_FORMAT_R16_SINT;

		case crgfx::DataFormat::RG16_Unorm:        return DXGI_FORMAT_R16G16_UNORM;
		case crgfx::DataFormat::RG16_Snorm:        return DXGI_FORMAT_R16G16_SNORM;
		case crgfx::DataFormat::RG16_Uint:         return DXGI_FORMAT_R16G16_UINT;
		case crgfx::DataFormat::RG16_Sint:         return DXGI_FORMAT_R16G16_SINT;

		case crgfx::DataFormat::RGBA16_Unorm:      return DXGI_FORMAT_R16G16B16A16_UNORM;
		case crgfx::DataFormat::RGBA16_Snorm:      return DXGI_FORMAT_R16G16B16A16_SNORM;
		case crgfx::DataFormat::RGBA16_Uint:       return DXGI_FORMAT_R16G16B16A16_UINT;
		case crgfx::DataFormat::RGBA16_Sint:       return DXGI_FORMAT_R16G16B16A16_SINT;

		// 16-bit float formats
		case crgfx::DataFormat::R16_Float:         return DXGI_FORMAT_R16_FLOAT;
		case crgfx::DataFormat::RG16_Float:        return DXGI_FORMAT_R16G16_FLOAT;
		case crgfx::DataFormat::RGBA16_Float:      return DXGI_FORMAT_R16G16B16A16_FLOAT;

		case crgfx::DataFormat::R32_Uint:          return DXGI_FORMAT_R32_UINT;
		case crgfx::DataFormat::R32_Sint:          return DXGI_FORMAT_R32_SINT;
		case crgfx::DataFormat::RG32_Uint:         return DXGI_FORMAT_R32G32_UINT;
		case crgfx::DataFormat::RG32_Sint:         return DXGI_FORMAT_R32G32_SINT;
		case crgfx::DataFormat::RGB32_Uint:        return DXGI_FORMAT_R32G32B32_UINT;
		case crgfx::DataFormat::RGB32_Sint:        return DXGI_FORMAT_R32G32B32_SINT;
		case crgfx::DataFormat::RGBA32_Uint:       return DXGI_FORMAT_R32G32B32A32_UINT;
		case crgfx::DataFormat::RGBA32_Sint:       return DXGI_FORMAT_R32G32B32A32_SINT;

		// 32-bit float formats
		case crgfx::DataFormat::R32_Float:         return DXGI_FORMAT_R32_FLOAT;
		case crgfx::DataFormat::RG32_Float:        return DXGI_FORMAT_R32G32_FLOAT;
		case crgfx::DataFormat::RGB32_Float:       return DXGI_FORMAT_R32G32B32_FLOAT;
		case crgfx::DataFormat::RGBA32_Float:      return DXGI_FORMAT_R32G32B32A32_FLOAT;

		// Compressed formats
		case crgfx::DataFormat::BC1_RGB_Unorm:     return DXGI_FORMAT_BC1_UNORM;
		case crgfx::DataFormat::BC1_RGB_SRGB:      return DXGI_FORMAT_BC1_UNORM_SRGB;
		case crgfx::DataFormat::BC1_RGBA_Unorm:    return DXGI_FORMAT_BC1_UNORM;
		case crgfx::DataFormat::BC1_RGBA_SRGB:     return DXGI_FORMAT_BC1_UNORM_SRGB;

		case crgfx::DataFormat::BC2_Unorm:         return DXGI_FORMAT_BC2_UNORM;
		case crgfx::DataFormat::BC2_SRGB:          return DXGI_FORMAT_BC2_UNORM_SRGB;

		case crgfx::DataFormat::BC3_Unorm:         return DXGI_FORMAT_BC3_UNORM;
		case crgfx::DataFormat::BC3_SRGB:          return DXGI_FORMAT_BC3_UNORM_SRGB;

		case crgfx::DataFormat::BC4_Unorm:         return DXGI_FORMAT_BC4_UNORM;
		case crgfx::DataFormat::BC4_Snorm:         return DXGI_FORMAT_BC4_SNORM;

		case crgfx::DataFormat::BC5_Unorm:         return DXGI_FORMAT_BC5_UNORM;
		case crgfx::DataFormat::BC5_Snorm:         return DXGI_FORMAT_BC5_SNORM;

		case crgfx::DataFormat::BC6H_UFloat:       return DXGI_FORMAT_BC6H_UF16;
		case crgfx::DataFormat::BC6H_SFloat:       return DXGI_FORMAT_BC6H_SF16;

		case crgfx::DataFormat::BC7_Unorm:         return DXGI_FORMAT_BC7_UNORM;
		case crgfx::DataFormat::BC7_SRGB:          return DXGI_FORMAT_BC7_UNORM_SRGB;

		// Depth-stencil formats
		case crgfx::DataFormat::D16_Unorm:         return DXGI_FORMAT_D16_UNORM;
		case crgfx::DataFormat::D24_Unorm_S8_Uint: return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case crgfx::DataFormat::D24_Unorm_X8:      return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case crgfx::DataFormat::D32_Float:         return DXGI_FORMAT_D32_FLOAT;
		case crgfx::DataFormat::D32_Float_S8_Uint: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

		// Packed Formats
		case crgfx::DataFormat::RG11B10_Float:  return DXGI_FORMAT_R11G11B10_FLOAT;
		case crgfx::DataFormat::RGB10A2_Unorm:  return DXGI_FORMAT_R10G10B10A2_UNORM;
		case crgfx::DataFormat::RGB10A2_Uint:   return DXGI_FORMAT_R10G10B10A2_UINT;
		case crgfx::DataFormat::B5G6R5_Unorm:   return DXGI_FORMAT_B5G6R5_UNORM;
		case crgfx::DataFormat::B5G5R5A1_Unorm: return DXGI_FORMAT_B5G5R5A1_UNORM;
		case crgfx::DataFormat::BGRA4_Unorm:    return DXGI_FORMAT_B4G4R4A4_UNORM;
		case crgfx::DataFormat::RGB9E5_Float:   return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

		default:
			CrAssertMsg(false, "Format not found");
			return DXGI_FORMAT_UNKNOWN;
	}
}

D3D12_TEXTURE_ADDRESS_MODE crd3d::GetD3DAddressMode(crgfx::AddressMode addressMode)
{
	switch (addressMode)
	{
		case crgfx::AddressMode::ClampToEdge:
			return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		case crgfx::AddressMode::ClampToBorder:
			return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		case crgfx::AddressMode::Wrap:
			return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		case crgfx::AddressMode::Mirror:
			return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		case crgfx::AddressMode::MirrorOnce:
			return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
	}

	return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
}

D3D12_FILTER crd3d::GetD3DFilter(crgfx::Filter minFilter, crgfx::Filter magFilter, crgfx::Filter mipFilter, bool anisotropic, bool comparison)
{
	if (comparison)
	{
		if (anisotropic)
		{
			return D3D12_FILTER_COMPARISON_ANISOTROPIC;
		}
		else if (minFilter == crgfx::Filter::Point && magFilter == crgfx::Filter::Point)
		{
			return mipFilter == crgfx::Filter::Point ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
		}
		else if (minFilter == crgfx::Filter::Linear && magFilter == crgfx::Filter::Point)
		{
			return mipFilter == crgfx::Filter::Point ? D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT : D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		}
		else if (minFilter == crgfx::Filter::Point && magFilter == crgfx::Filter::Linear)
		{
			return mipFilter == crgfx::Filter::Point ? D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT : D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
		}
		else
		{
			return mipFilter == crgfx::Filter::Point ? D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT : D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		}
	}
	else
	{
		if (anisotropic)
		{
			return D3D12_FILTER_ANISOTROPIC;
		}
		else if (minFilter == crgfx::Filter::Point && magFilter == crgfx::Filter::Point)
		{
			return mipFilter == crgfx::Filter::Point ? D3D12_FILTER_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		}
		else if (minFilter == crgfx::Filter::Linear && magFilter == crgfx::Filter::Point)
		{
			return mipFilter == crgfx::Filter::Point ? D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT : D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		}
		else if (minFilter == crgfx::Filter::Point && magFilter == crgfx::Filter::Linear)
		{
			return mipFilter == crgfx::Filter::Point ? D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT : D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		}
		else
		{
			return mipFilter == crgfx::Filter::Point ? D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		}
	}
}

D3D12_BLEND_OP crd3d::GetD3DBlendOp(crgfx::BlendOp blendOp)
{
	switch (blendOp)
	{
		case crgfx::BlendOp::Add:
			return D3D12_BLEND_OP_ADD;
		case crgfx::BlendOp::Subtract:
			return D3D12_BLEND_OP_SUBTRACT;
		case crgfx::BlendOp::ReverseSubtract:
			return D3D12_BLEND_OP_REV_SUBTRACT;
		case crgfx::BlendOp::Min:
			return D3D12_BLEND_OP_MIN;
		case crgfx::BlendOp::Max:
			return D3D12_BLEND_OP_MAX;
	}
	return D3D12_BLEND_OP_ADD;
}

D3D12_BLEND crd3d::GetD3DBlendFactor(crgfx::BlendFactor blendFactor)
{
	switch (blendFactor)
	{
		case crgfx::BlendFactor::Zero:
			return D3D12_BLEND_ZERO;
		case crgfx::BlendFactor::One:
			return D3D12_BLEND_ONE;
		case crgfx::BlendFactor::SrcColor:
			return D3D12_BLEND_SRC_COLOR;
		case crgfx::BlendFactor::OneMinusSrcColor:
			return D3D12_BLEND_INV_SRC_COLOR;
		case crgfx::BlendFactor::DstColor:
			return D3D12_BLEND_DEST_COLOR;
		case crgfx::BlendFactor::OneMinusDstColor:
			return D3D12_BLEND_INV_DEST_COLOR;
		case crgfx::BlendFactor::SrcAlpha:
			return D3D12_BLEND_SRC_ALPHA;
		case crgfx::BlendFactor::OneMinusSrcAlpha:
			return D3D12_BLEND_INV_SRC_ALPHA;
		case crgfx::BlendFactor::DstAlpha:
			return D3D12_BLEND_DEST_ALPHA;
		case crgfx::BlendFactor::OneMinusDstAlpha:
			return D3D12_BLEND_INV_DEST_ALPHA;
		case crgfx::BlendFactor::Constant:
			return D3D12_BLEND_BLEND_FACTOR;
		case crgfx::BlendFactor::OneMinusConstant:
			return D3D12_BLEND_INV_BLEND_FACTOR;
		case crgfx::BlendFactor::SrcAlphaSaturate:
			return D3D12_BLEND_SRC_ALPHA_SAT;
		case crgfx::BlendFactor::Src1Color:
			return D3D12_BLEND_SRC1_COLOR;
		case crgfx::BlendFactor::OneMinusSrc1Color:
			return D3D12_BLEND_INV_SRC1_COLOR;
		case crgfx::BlendFactor::Src1Alpha:
			return D3D12_BLEND_SRC1_ALPHA;
		case crgfx::BlendFactor::OneMinusSrc1Alpha:
			return D3D12_BLEND_INV_SRC1_ALPHA;
	}
	return D3D12_BLEND_ONE;
}

D3D12_COMPARISON_FUNC crd3d::GetD3DCompareOp(crgfx::CompareOp compareOp)
{
	switch (compareOp)
	{
		case crgfx::CompareOp::Never:
			return D3D12_COMPARISON_FUNC_NEVER;
		case crgfx::CompareOp::Less:
			return D3D12_COMPARISON_FUNC_LESS;
		case crgfx::CompareOp::Equal:
			return D3D12_COMPARISON_FUNC_EQUAL;
		case crgfx::CompareOp::LessOrEqual:
			return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case crgfx::CompareOp::Greater:
			return D3D12_COMPARISON_FUNC_GREATER;
		case crgfx::CompareOp::NotEqual:
			return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case crgfx::CompareOp::GreaterOrEqual:
			return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case crgfx::CompareOp::Always:
			return D3D12_COMPARISON_FUNC_ALWAYS;
	}
	return D3D12_COMPARISON_FUNC_ALWAYS;
}

D3D12_STENCIL_OP crd3d::GetD3DStencilOp(crgfx::StencilOp stencilOp)
{
	switch (stencilOp)
	{
		case crgfx::StencilOp::Keep:
			return D3D12_STENCIL_OP_KEEP;
		case crgfx::StencilOp::Zero:
			return D3D12_STENCIL_OP_ZERO;
		case crgfx::StencilOp::Replace:
			return D3D12_STENCIL_OP_REPLACE;
		case crgfx::StencilOp::IncrementSaturate:
			return D3D12_STENCIL_OP_INCR_SAT;
		case crgfx::StencilOp::DecrementSaturate:
			return D3D12_STENCIL_OP_DECR_SAT;
		case crgfx::StencilOp::Invert:
			return D3D12_STENCIL_OP_INVERT;
		case crgfx::StencilOp::IncrementAndWrap:
			return D3D12_STENCIL_OP_INCR;
		case crgfx::StencilOp::DecrementAndWrap:
			return D3D12_STENCIL_OP_DECR;
	}
	return D3D12_STENCIL_OP_KEEP;
}

uint32_t crd3d::GetD3D12SampleCount(crgfx::SampleCount sampleCount)
{
	switch (sampleCount)
	{
		case crgfx::SampleCount::S1:
			return 1;
		case crgfx::SampleCount::S2:
			return 2;
		case crgfx::SampleCount::S4:
			return 4;
		case crgfx::SampleCount::S8:
			return 8;
		case crgfx::SampleCount::S16:
			return 16;
		case crgfx::SampleCount::S32:
			return 32;
		case crgfx::SampleCount::S64:
			return 64;
	}
	return 1;
}

D3D_PRIMITIVE_TOPOLOGY crd3d::GetD3D12PrimitiveTopology(crgfx::PrimitiveTopology primitiveTopology)
{
	switch (primitiveTopology)
	{
		case crgfx::PrimitiveTopology::PointList:
			return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case crgfx::PrimitiveTopology::LineList:
			return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		case crgfx::PrimitiveTopology::LineStrip:
			return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case crgfx::PrimitiveTopology::TriangleList:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case crgfx::PrimitiveTopology::TriangleStrip:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		case crgfx::PrimitiveTopology::LineListAdjacency:
			return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
		case crgfx::PrimitiveTopology::LineStripAdjacency:
			return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
		case crgfx::PrimitiveTopology::TriangleListAdjacency:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
		case crgfx::PrimitiveTopology::TriangleStripAdjacency:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
		case crgfx::PrimitiveTopology::PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST; // TODO This is incorrect
	}

	return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE crd3d::GetD3D12PrimitiveTopologyType(crgfx::PrimitiveTopology primitiveTopology)
{
	switch (primitiveTopology)
	{
		case crgfx::PrimitiveTopology::PointList:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		case crgfx::PrimitiveTopology::LineList:
		case crgfx::PrimitiveTopology::LineStrip:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case crgfx::PrimitiveTopology::TriangleList:
		case crgfx::PrimitiveTopology::TriangleStrip:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		default:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
	}
}

D3D12_FILL_MODE crd3d::GetD3D12PolygonFillMode(crgfx::PolygonFillMode fillMode)
{
	switch (fillMode)
	{
		case crgfx::PolygonFillMode::Fill:
			return D3D12_FILL_MODE_SOLID;
		case crgfx::PolygonFillMode::Line:
			return D3D12_FILL_MODE_WIREFRAME;
		default:
			return D3D12_FILL_MODE_SOLID;
	}
}

D3D12_CULL_MODE crd3d::GetD3D12PolygonCullMode(crgfx::PolygonCullMode cullMode)
{
	switch (cullMode)
	{
		case crgfx::PolygonCullMode::Back:
			return D3D12_CULL_MODE_BACK;
		case crgfx::PolygonCullMode::Front:
			return D3D12_CULL_MODE_FRONT;
		case crgfx::PolygonCullMode::None:
			return D3D12_CULL_MODE_NONE;
		default:
			return D3D12_CULL_MODE_NONE;
	}
}

D3D12_COMMAND_LIST_TYPE crd3d::GetD3D12CommandQueueType(CrCommandQueueType::T commandQueueType)
{
	switch (commandQueueType)
	{
		case CrCommandQueueType::Graphics:
			return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case CrCommandQueueType::Compute:
			return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		case CrCommandQueueType::Copy:
			return D3D12_COMMAND_LIST_TYPE_COPY;
		default:
			return D3D12_COMMAND_LIST_TYPE_DIRECT;
	}
}

D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE crd3d::GetD3D12BeginningAccessType(CrRenderTargetLoadOp loadOp)
{
	switch (loadOp)
	{
		case CrRenderTargetLoadOp::Clear:
			return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
		case CrRenderTargetLoadOp::DontCare:
			return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
		case CrRenderTargetLoadOp::Load:
			return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
		default:
			return D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
	}
}

D3D12_RENDER_PASS_ENDING_ACCESS_TYPE crd3d::GetD3D12EndingAccessType(CrRenderTargetStoreOp storeOp)
{
	switch (storeOp)
	{
		case CrRenderTargetStoreOp::DontCare:
			return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
		case CrRenderTargetStoreOp::Store:
			return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
		default:
			return D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
	}
}

D3D12_BARRIER_SYNC GetD3D12BarrierShaderSyncFlags(crgfx::ShaderStageFlags::T stages)
{
	D3D12_BARRIER_SYNC barrierSync = D3D12_BARRIER_SYNC_NONE;

	if (stages & (crgfx::ShaderStageFlags::Vertex | crgfx::ShaderStageFlags::Geometry | crgfx::ShaderStageFlags::Hull | crgfx::ShaderStageFlags::Domain))
	{
		barrierSync |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
	}

	if (stages & crgfx::ShaderStageFlags::Pixel)
	{
		barrierSync |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
	}

	if (stages & crgfx::ShaderStageFlags::Compute)
	{
		barrierSync |= D3D12_BARRIER_SYNC_COMPUTE_SHADING;
	}

	return barrierSync;
}

crd3d::TextureBarrierInfoD3D12 crd3d::GetD3D12TextureBarrierInfo(const crgfx::TextureState& textureState)
{
	crd3d::TextureBarrierInfoD3D12 textureBarrierInfo;
	textureBarrierInfo.sync = D3D12_BARRIER_SYNC_NONE;
	textureBarrierInfo.access = D3D12_BARRIER_ACCESS_COMMON;

	switch (textureState.layout)
	{
		case crgfx::TextureLayout::RenderTarget:
		{
			textureBarrierInfo.sync = D3D12_BARRIER_SYNC_RENDER_TARGET;
			textureBarrierInfo.access = D3D12_BARRIER_ACCESS_RENDER_TARGET;
			break;
		}
		case crgfx::TextureLayout::DepthStencilReadWrite:
		case crgfx::TextureLayout::DepthStencilWrite:
		case crgfx::TextureLayout::StencilWriteDepthReadOnly:
		case crgfx::TextureLayout::DepthWriteStencilReadOnly:
		{
			textureBarrierInfo.sync = D3D12_BARRIER_SYNC_DEPTH_STENCIL;
			textureBarrierInfo.access = D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
			break;
		}
		case crgfx::TextureLayout::DepthStencilReadOnly:
		{
			textureBarrierInfo.sync = D3D12_BARRIER_SYNC_DEPTH_STENCIL;
			textureBarrierInfo.access = D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
			break;
		}
		case crgfx::TextureLayout::ShaderInput:
		{
			textureBarrierInfo.sync |= GetD3D12BarrierShaderSyncFlags(textureState.stages);
			textureBarrierInfo.access = D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
			break;
		}
		case crgfx::TextureLayout::RWTexture:
		{
			textureBarrierInfo.sync |= GetD3D12BarrierShaderSyncFlags(textureState.stages);
			textureBarrierInfo.access = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
			break;
		}
		case crgfx::TextureLayout::CopySource:
		{
			textureBarrierInfo.sync = D3D12_BARRIER_SYNC_COPY;
			textureBarrierInfo.access = D3D12_BARRIER_ACCESS_COPY_SOURCE;
			break;
		}
		case crgfx::TextureLayout::CopyDestination:
		{
			textureBarrierInfo.sync = D3D12_BARRIER_SYNC_COPY;
			textureBarrierInfo.access = D3D12_BARRIER_ACCESS_COPY_DEST;
			break;
		}
		case crgfx::TextureLayout::Present:
		{
			textureBarrierInfo.sync = D3D12_BARRIER_SYNC_ALL;
			textureBarrierInfo.access = D3D12_BARRIER_ACCESS_COMMON;
			break;
		}
		case crgfx::TextureLayout::Undefined:
		{
			textureBarrierInfo.sync = D3D12_BARRIER_SYNC_NONE;
			textureBarrierInfo.access = D3D12_BARRIER_ACCESS_NO_ACCESS;
			break;
		}
		default:
			CrAssertMsg(false, "Unhandled texture state");
			break;
	}

	return textureBarrierInfo;
}

crd3d::BufferBarrierInfoD3D12 crd3d::GetD3D12BufferBarrierInfo(crgfx::BufferState::T bufferState, crgfx::ShaderStageFlags::T shaderStages)
{
	crd3d::BufferBarrierInfoD3D12 bufferBarrierInfo;

	switch (bufferState)
	{
		case crgfx::BufferState::ShaderInput:
			bufferBarrierInfo.sync = GetD3D12BarrierShaderSyncFlags(shaderStages);
			bufferBarrierInfo.access = D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
			break;
		case crgfx::BufferState::ReadWrite:
			bufferBarrierInfo.sync = GetD3D12BarrierShaderSyncFlags(shaderStages);
			bufferBarrierInfo.access = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
			break;
		case crgfx::BufferState::CopySource:
			bufferBarrierInfo.sync = D3D12_BARRIER_SYNC_COPY;
			bufferBarrierInfo.access = D3D12_BARRIER_ACCESS_COPY_SOURCE;
			break;
		case crgfx::BufferState::CopyDestination:
			bufferBarrierInfo.sync = D3D12_BARRIER_SYNC_COPY;
			bufferBarrierInfo.access = D3D12_BARRIER_ACCESS_COPY_DEST;
			break;
		case crgfx::BufferState::IndirectArgument:
			bufferBarrierInfo.sync = D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;
			bufferBarrierInfo.access = D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;
			break;
		case crgfx::BufferState::Undefined:
			bufferBarrierInfo.sync = D3D12_BARRIER_SYNC_NONE;
			bufferBarrierInfo.access = D3D12_BARRIER_ACCESS_NO_ACCESS;
			break;
		default:
			CrAssertMsg(false, "Unhandled buffer state");
			break;
	}

	return bufferBarrierInfo;
}

D3D12_BARRIER_LAYOUT crd3d::GetD3D12BarrierTextureLayout(const crgfx::TextureLayout::T textureLayout)
{
	switch (textureLayout)
	{
		case crgfx::TextureLayout::ShaderInput:
			return D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
		case crgfx::TextureLayout::RenderTarget:
			return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
		case crgfx::TextureLayout::RWTexture:
			return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
		case crgfx::TextureLayout::Present:
			return D3D12_BARRIER_LAYOUT_PRESENT;
		case crgfx::TextureLayout::CopySource:
			return D3D12_BARRIER_LAYOUT_COPY_SOURCE;
		case crgfx::TextureLayout::CopyDestination:
			return D3D12_BARRIER_LAYOUT_COPY_DEST;
		case crgfx::TextureLayout::DepthStencilReadWrite:
		case crgfx::TextureLayout::DepthStencilWrite:
			return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
		case crgfx::TextureLayout::Undefined:
			return D3D12_BARRIER_LAYOUT_UNDEFINED;
		default:
			CrAssertMsg(false, "Unhandled texture layout");
			return D3D12_BARRIER_LAYOUT_COMMON;
	}
}
