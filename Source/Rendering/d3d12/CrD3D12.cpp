#include "Rendering/CrRendering_pch.h"
#include "CrD3D12.h"

#include "Core/Logging/ICrDebug.h"

#include "Rendering/CrRendering.h"

DXGI_FORMAT crd3d::GetDXGIFormat(cr3d::DataFormat::T format)
{
	switch (format)
	{
		// 8-bit formats
		case cr3d::DataFormat::R8_Unorm:          return DXGI_FORMAT_R8_UNORM;
		case cr3d::DataFormat::R8_Snorm:          return DXGI_FORMAT_R8_SNORM;
		case cr3d::DataFormat::R8_Uint:           return DXGI_FORMAT_R8_UINT;
		case cr3d::DataFormat::R8_Sint:           return DXGI_FORMAT_R8_SINT;

		case cr3d::DataFormat::RG8_Unorm:         return DXGI_FORMAT_R8G8_UNORM;
		case cr3d::DataFormat::RG8_Snorm:         return DXGI_FORMAT_R8G8_SNORM;
		case cr3d::DataFormat::RG8_Uint:          return DXGI_FORMAT_R8G8_UINT;
		case cr3d::DataFormat::RG8_Sint:          return DXGI_FORMAT_R8G8_SINT;

		case cr3d::DataFormat::RGBA8_Unorm:       return DXGI_FORMAT_R8G8B8A8_UNORM;
		case cr3d::DataFormat::RGBA8_Snorm:       return DXGI_FORMAT_R8G8B8A8_SNORM;
		case cr3d::DataFormat::RGBA8_Uint:        return DXGI_FORMAT_R8G8B8A8_UINT;
		case cr3d::DataFormat::RGBA8_Sint:        return DXGI_FORMAT_R8G8B8A8_SINT;
		case cr3d::DataFormat::RGBA8_SRGB:        return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		case cr3d::DataFormat::BGRA8_Unorm:       return DXGI_FORMAT_B8G8R8A8_UNORM;
		case cr3d::DataFormat::BGRA8_SRGB:        return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

		// 16-bit integer formats
		case cr3d::DataFormat::R16_Unorm:         return DXGI_FORMAT_R16_UNORM;
		case cr3d::DataFormat::R16_Snorm:         return DXGI_FORMAT_R16_SNORM;
		case cr3d::DataFormat::R16_Uint:          return DXGI_FORMAT_R16_UINT;
		case cr3d::DataFormat::R16_Sint:          return DXGI_FORMAT_R16_SINT;

		case cr3d::DataFormat::RG16_Unorm:        return DXGI_FORMAT_R16G16_UNORM;
		case cr3d::DataFormat::RG16_Snorm:        return DXGI_FORMAT_R16G16_SNORM;
		case cr3d::DataFormat::RG16_Uint:         return DXGI_FORMAT_R16G16_UINT;
		case cr3d::DataFormat::RG16_Sint:         return DXGI_FORMAT_R16G16_SINT;

		case cr3d::DataFormat::RGBA16_Unorm:      return DXGI_FORMAT_R16G16B16A16_UNORM;
		case cr3d::DataFormat::RGBA16_Snorm:      return DXGI_FORMAT_R16G16B16A16_SNORM;
		case cr3d::DataFormat::RGBA16_Uint:       return DXGI_FORMAT_R16G16B16A16_UINT;
		case cr3d::DataFormat::RGBA16_Sint:       return DXGI_FORMAT_R16G16B16A16_SINT;

		// 16-bit float formats
		case cr3d::DataFormat::R16_Float:         return DXGI_FORMAT_R16_FLOAT;
		case cr3d::DataFormat::RG16_Float:        return DXGI_FORMAT_R16G16_FLOAT;
		case cr3d::DataFormat::RGBA16_Float:      return DXGI_FORMAT_R16G16B16A16_FLOAT;

		case cr3d::DataFormat::R32_Uint:          return DXGI_FORMAT_R32_UINT;
		case cr3d::DataFormat::R32_Sint:          return DXGI_FORMAT_R32_SINT;
		case cr3d::DataFormat::RG32_Uint:         return DXGI_FORMAT_R32G32_UINT;
		case cr3d::DataFormat::RG32_Sint:         return DXGI_FORMAT_R32G32_SINT;
		case cr3d::DataFormat::RGB32_Uint:        return DXGI_FORMAT_R32G32B32_UINT;
		case cr3d::DataFormat::RGB32_Sint:        return DXGI_FORMAT_R32G32B32_SINT;
		case cr3d::DataFormat::RGBA32_Uint:       return DXGI_FORMAT_R32G32B32A32_UINT;
		case cr3d::DataFormat::RGBA32_Sint:       return DXGI_FORMAT_R32G32B32A32_SINT;

		// 32-bit float formats
		case cr3d::DataFormat::R32_Float:         return DXGI_FORMAT_R32_FLOAT;
		case cr3d::DataFormat::RG32_Float:        return DXGI_FORMAT_R32G32_FLOAT;
		case cr3d::DataFormat::RGB32_Float:       return DXGI_FORMAT_R32G32B32_FLOAT;
		case cr3d::DataFormat::RGBA32_Float:      return DXGI_FORMAT_R32G32B32A32_FLOAT;

		// Compressed formats
		case cr3d::DataFormat::BC1_RGB_Unorm:     return DXGI_FORMAT_BC1_UNORM;
		case cr3d::DataFormat::BC1_RGB_SRGB:      return DXGI_FORMAT_BC1_UNORM_SRGB;
		case cr3d::DataFormat::BC1_RGBA_Unorm:    return DXGI_FORMAT_BC1_UNORM;
		case cr3d::DataFormat::BC1_RGBA_SRGB:     return DXGI_FORMAT_BC1_UNORM_SRGB;

		case cr3d::DataFormat::BC2_Unorm:         return DXGI_FORMAT_BC2_UNORM;
		case cr3d::DataFormat::BC2_SRGB:          return DXGI_FORMAT_BC2_UNORM_SRGB;

		case cr3d::DataFormat::BC3_Unorm:         return DXGI_FORMAT_BC3_UNORM;
		case cr3d::DataFormat::BC3_SRGB:          return DXGI_FORMAT_BC3_UNORM_SRGB;

		case cr3d::DataFormat::BC4_Unorm:         return DXGI_FORMAT_BC4_UNORM;
		case cr3d::DataFormat::BC4_Snorm:         return DXGI_FORMAT_BC4_SNORM;

		case cr3d::DataFormat::BC5_Unorm:         return DXGI_FORMAT_BC5_UNORM;
		case cr3d::DataFormat::BC5_Snorm:         return DXGI_FORMAT_BC5_SNORM;

		case cr3d::DataFormat::BC6H_UFloat:       return DXGI_FORMAT_BC6H_UF16;
		case cr3d::DataFormat::BC6H_SFloat:       return DXGI_FORMAT_BC6H_SF16;

		case cr3d::DataFormat::BC7_Unorm:         return DXGI_FORMAT_BC7_UNORM;
		case cr3d::DataFormat::BC7_SRGB:          return DXGI_FORMAT_BC7_UNORM_SRGB;

		// Depth-stencil formats
		case cr3d::DataFormat::D16_Unorm:         return DXGI_FORMAT_D16_UNORM;
		case cr3d::DataFormat::D24_Unorm_S8_Uint: return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case cr3d::DataFormat::D24_Unorm_X8:      return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case cr3d::DataFormat::D32_Float:         return DXGI_FORMAT_D32_FLOAT;
		case cr3d::DataFormat::D32_Float_S8_Uint: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

		// Packed Formats
		case cr3d::DataFormat::RG11B10_Float:  return DXGI_FORMAT_R11G11B10_FLOAT;
		case cr3d::DataFormat::RGB10A2_Unorm:  return DXGI_FORMAT_R10G10B10A2_UNORM;
		case cr3d::DataFormat::RGB10A2_Uint:   return DXGI_FORMAT_R10G10B10A2_UINT;
		case cr3d::DataFormat::B5G6R5_Unorm:   return DXGI_FORMAT_B5G6R5_UNORM;
		case cr3d::DataFormat::B5G5R5A1_Unorm: return DXGI_FORMAT_B5G5R5A1_UNORM;
		case cr3d::DataFormat::BGRA4_Unorm:    return DXGI_FORMAT_B4G4R4A4_UNORM;
		case cr3d::DataFormat::RGB9E5_Float:   return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

		default:
			CrAssertMsg(false, "Format not found");
			return DXGI_FORMAT_UNKNOWN;
	}
}

D3D12_TEXTURE_ADDRESS_MODE crd3d::GetD3DAddressMode(cr3d::AddressMode addressMode)
{
	switch (addressMode)
	{
		case cr3d::AddressMode::ClampToEdge:
			return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		case cr3d::AddressMode::ClampToBorder:
			return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		case cr3d::AddressMode::Wrap:
			return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		case cr3d::AddressMode::Mirror:
			return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		case cr3d::AddressMode::MirrorOnce:
			return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
	}

	return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
}

D3D12_FILTER crd3d::GetD3DFilter(cr3d::Filter minFilter, cr3d::Filter magFilter, cr3d::Filter mipFilter, bool anisotropic, bool comparison)
{
	if (comparison)
	{
		if (anisotropic)
		{
			return D3D12_FILTER_COMPARISON_ANISOTROPIC;
		}
		else if (minFilter == cr3d::Filter::Point && magFilter == cr3d::Filter::Point)
		{
			return mipFilter == cr3d::Filter::Point ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
		}
		else if (minFilter == cr3d::Filter::Linear && magFilter == cr3d::Filter::Point)
		{
			return mipFilter == cr3d::Filter::Point ? D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT : D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		}
		else if (minFilter == cr3d::Filter::Point && magFilter == cr3d::Filter::Linear)
		{
			return mipFilter == cr3d::Filter::Point ? D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT : D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
		}
		else
		{
			return mipFilter == cr3d::Filter::Point ? D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT : D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		}
	}
	else
	{
		if (anisotropic)
		{
			return D3D12_FILTER_ANISOTROPIC;
		}
		else if (minFilter == cr3d::Filter::Point && magFilter == cr3d::Filter::Point)
		{
			return mipFilter == cr3d::Filter::Point ? D3D12_FILTER_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		}
		else if (minFilter == cr3d::Filter::Linear && magFilter == cr3d::Filter::Point)
		{
			return mipFilter == cr3d::Filter::Point ? D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT : D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		}
		else if (minFilter == cr3d::Filter::Point && magFilter == cr3d::Filter::Linear)
		{
			return mipFilter == cr3d::Filter::Point ? D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT : D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		}
		else
		{
			return mipFilter == cr3d::Filter::Point ? D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		}
	}
}

D3D12_BLEND_OP crd3d::GetD3DBlendOp(cr3d::BlendOp blendOp)
{
	switch (blendOp)
	{
		case cr3d::BlendOp::Add:
			return D3D12_BLEND_OP_ADD;
		case cr3d::BlendOp::Subtract:
			return D3D12_BLEND_OP_SUBTRACT;
		case cr3d::BlendOp::ReverseSubtract:
			return D3D12_BLEND_OP_REV_SUBTRACT;
		case cr3d::BlendOp::Min:
			return D3D12_BLEND_OP_MIN;
		case cr3d::BlendOp::Max:
			return D3D12_BLEND_OP_MAX;
	}
	return D3D12_BLEND_OP_ADD;
}

D3D12_BLEND crd3d::GetD3DBlendFactor(cr3d::BlendFactor blendFactor)
{
	switch (blendFactor)
	{
		case cr3d::BlendFactor::Zero:
			return D3D12_BLEND_ZERO;
		case cr3d::BlendFactor::One:
			return D3D12_BLEND_ONE;
		case cr3d::BlendFactor::SrcColor:
			return D3D12_BLEND_SRC_COLOR;
		case cr3d::BlendFactor::OneMinusSrcColor:
			return D3D12_BLEND_INV_SRC_COLOR;
		case cr3d::BlendFactor::DstColor:
			return D3D12_BLEND_DEST_COLOR;
		case cr3d::BlendFactor::OneMinusDstColor:
			return D3D12_BLEND_INV_DEST_COLOR;
		case cr3d::BlendFactor::SrcAlpha:
			return D3D12_BLEND_SRC_ALPHA;
		case cr3d::BlendFactor::OneMinusSrcAlpha:
			return D3D12_BLEND_INV_SRC_ALPHA;
		case cr3d::BlendFactor::DstAlpha:
			return D3D12_BLEND_DEST_ALPHA;
		case cr3d::BlendFactor::OneMinusDstAlpha:
			return D3D12_BLEND_INV_DEST_ALPHA;
		case cr3d::BlendFactor::Constant:
			return D3D12_BLEND_BLEND_FACTOR;
		case cr3d::BlendFactor::OneMinusConstant:
			return D3D12_BLEND_INV_BLEND_FACTOR;
		case cr3d::BlendFactor::SrcAlphaSaturate:
			return D3D12_BLEND_SRC_ALPHA_SAT;
		case cr3d::BlendFactor::Src1Color:
			return D3D12_BLEND_SRC1_COLOR;
		case cr3d::BlendFactor::OneMinusSrc1Color:
			return D3D12_BLEND_INV_SRC1_COLOR;
		case cr3d::BlendFactor::Src1Alpha:
			return D3D12_BLEND_SRC1_ALPHA;
		case cr3d::BlendFactor::OneMinusSrc1Alpha:
			return D3D12_BLEND_INV_SRC1_ALPHA;
	}
	return D3D12_BLEND_ONE;
}

D3D12_COMPARISON_FUNC crd3d::GetD3DCompareOp(cr3d::CompareOp compareOp)
{
	switch (compareOp)
	{
		case cr3d::CompareOp::Never:
			return D3D12_COMPARISON_FUNC_NEVER;
		case cr3d::CompareOp::Less:
			return D3D12_COMPARISON_FUNC_LESS;
		case cr3d::CompareOp::Equal:
			return D3D12_COMPARISON_FUNC_EQUAL;
		case cr3d::CompareOp::LessOrEqual:
			return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case cr3d::CompareOp::Greater:
			return D3D12_COMPARISON_FUNC_GREATER;
		case cr3d::CompareOp::NotEqual:
			return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case cr3d::CompareOp::GreaterOrEqual:
			return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case cr3d::CompareOp::Always:
			return D3D12_COMPARISON_FUNC_ALWAYS;
	}
	return D3D12_COMPARISON_FUNC_ALWAYS;
}

D3D12_STENCIL_OP crd3d::GetD3DStencilOp(cr3d::StencilOp stencilOp)
{
	switch (stencilOp)
	{
		case cr3d::StencilOp::Keep:
			return D3D12_STENCIL_OP_KEEP;
		case cr3d::StencilOp::Zero:
			return D3D12_STENCIL_OP_ZERO;
		case cr3d::StencilOp::Replace:
			return D3D12_STENCIL_OP_REPLACE;
		case cr3d::StencilOp::IncrementSaturate:
			return D3D12_STENCIL_OP_INCR_SAT;
		case cr3d::StencilOp::DecrementSaturate:
			return D3D12_STENCIL_OP_DECR_SAT;
		case cr3d::StencilOp::Invert:
			return D3D12_STENCIL_OP_INVERT;
		case cr3d::StencilOp::IncrementAndWrap:
			return D3D12_STENCIL_OP_INCR;
		case cr3d::StencilOp::DecrementAndWrap:
			return D3D12_STENCIL_OP_DECR;
	}
	return D3D12_STENCIL_OP_KEEP;
}

uint32_t crd3d::GetD3D12SampleCount(cr3d::SampleCount sampleCount)
{
	switch (sampleCount)
	{
		case cr3d::SampleCount::S1:
			return 1;
		case cr3d::SampleCount::S2:
			return 2;
		case cr3d::SampleCount::S4:
			return 4;
		case cr3d::SampleCount::S8:
			return 8;
		case cr3d::SampleCount::S16:
			return 16;
		case cr3d::SampleCount::S32:
			return 32;
		case cr3d::SampleCount::S64:
			return 64;
	}
	return 1;
}

D3D_PRIMITIVE_TOPOLOGY crd3d::GetD3D12PrimitiveTopology(cr3d::PrimitiveTopology primitiveTopology)
{
	switch (primitiveTopology)
	{
		case cr3d::PrimitiveTopology::PointList:
			return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case cr3d::PrimitiveTopology::LineList:
			return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		case cr3d::PrimitiveTopology::LineStrip:
			return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case cr3d::PrimitiveTopology::TriangleList:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case cr3d::PrimitiveTopology::TriangleStrip:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		case cr3d::PrimitiveTopology::LineListAdjacency:
			return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
		case cr3d::PrimitiveTopology::LineStripAdjacency:
			return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
		case cr3d::PrimitiveTopology::TriangleListAdjacency:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
		case cr3d::PrimitiveTopology::TriangleStripAdjacency:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
		case cr3d::PrimitiveTopology::PatchList:
			return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST; // TODO This is incorrect
	}

	return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
}

D3D12_PRIMITIVE_TOPOLOGY_TYPE crd3d::GetD3D12PrimitiveTopologyType(cr3d::PrimitiveTopology primitiveTopology)
{
	switch (primitiveTopology)
	{
		case cr3d::PrimitiveTopology::PointList:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		case cr3d::PrimitiveTopology::LineList:
		case cr3d::PrimitiveTopology::LineStrip:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case cr3d::PrimitiveTopology::TriangleList:
		case cr3d::PrimitiveTopology::TriangleStrip:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		default:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
	}
}

D3D12_FILL_MODE crd3d::GetD3D12PolygonFillMode(cr3d::PolygonFillMode fillMode)
{
	switch (fillMode)
	{
		case cr3d::PolygonFillMode::Fill:
			return D3D12_FILL_MODE_SOLID;
		case cr3d::PolygonFillMode::Line:
			return D3D12_FILL_MODE_WIREFRAME;
		default:
			return D3D12_FILL_MODE_SOLID;
	}
}

D3D12_CULL_MODE crd3d::GetD3D12PolygonCullMode(cr3d::PolygonCullMode cullMode)
{
	switch (cullMode)
	{
		case cr3d::PolygonCullMode::Back:
			return D3D12_CULL_MODE_BACK;
		case cr3d::PolygonCullMode::Front:
			return D3D12_CULL_MODE_FRONT;
		case cr3d::PolygonCullMode::None:
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

D3D12_RESOURCE_STATES crd3d::GetD3D12LegacyResourceState(const cr3d::TextureState& textureState)
{
	switch (textureState.layout)
	{
		case cr3d::TextureLayout::Undefined:         return D3D12_RESOURCE_STATE_COMMON;
		case cr3d::TextureLayout::ShaderInput:
		{
			// Simplify upstream rendergraph code by always specifying both. We'll deprecate this eventually so it's not that important
			return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		}
		case cr3d::TextureLayout::RenderTarget:          return D3D12_RESOURCE_STATE_RENDER_TARGET;
		case cr3d::TextureLayout::RWTexture:             return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		case cr3d::TextureLayout::Present:               return D3D12_RESOURCE_STATE_PRESENT;
		case cr3d::TextureLayout::CopySource:            return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case cr3d::TextureLayout::CopyDestination:       return D3D12_RESOURCE_STATE_COPY_DEST;

		case cr3d::TextureLayout::DepthStencilReadWrite: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
		case cr3d::TextureLayout::DepthStencilWrite:     return D3D12_RESOURCE_STATE_DEPTH_WRITE;
		case cr3d::TextureLayout::StencilWriteDepthReadOnly: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
		case cr3d::TextureLayout::DepthWriteStencilReadOnly: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
		case cr3d::TextureLayout::DepthStencilReadOnly:      return D3D12_RESOURCE_STATE_DEPTH_READ;
		default:
			CrAssertMsg(false, "Unhandled texture layout");
			return D3D12_RESOURCE_STATE_COMMON;
	}
}

D3D12_BARRIER_SYNC GetD3D12BarrierShaderSyncFlags(cr3d::ShaderStageFlags::T stages)
{
	D3D12_BARRIER_SYNC barrierSync = D3D12_BARRIER_SYNC_NONE;

	if (stages & (cr3d::ShaderStageFlags::Vertex | cr3d::ShaderStageFlags::Geometry | cr3d::ShaderStageFlags::Hull | cr3d::ShaderStageFlags::Domain))
	{
		barrierSync |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
	}

	if (stages & cr3d::ShaderStageFlags::Pixel)
	{
		barrierSync |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
	}

	if (stages & cr3d::ShaderStageFlags::Compute)
	{
		barrierSync |= D3D12_BARRIER_SYNC_COMPUTE_SHADING;
	}

	return barrierSync;
}

crd3d::TextureBarrierInfoD3D12 crd3d::GetD3D12TextureBarrierInfo(const cr3d::TextureState& textureState)
{
	crd3d::TextureBarrierInfoD3D12 textureBarrierInfo;
	textureBarrierInfo.sync = D3D12_BARRIER_SYNC_NONE;
	textureBarrierInfo.access = D3D12_BARRIER_ACCESS_COMMON;

	switch (textureState.layout)
	{
		case cr3d::TextureLayout::RenderTarget:
		{
			textureBarrierInfo.sync = D3D12_BARRIER_SYNC_RENDER_TARGET;
			textureBarrierInfo.access = D3D12_BARRIER_ACCESS_RENDER_TARGET;
			break;
		}
		case cr3d::TextureLayout::DepthStencilReadWrite:
		case cr3d::TextureLayout::DepthStencilWrite:
		case cr3d::TextureLayout::StencilWriteDepthReadOnly:
		case cr3d::TextureLayout::DepthWriteStencilReadOnly:
		{
			textureBarrierInfo.sync = D3D12_BARRIER_SYNC_DEPTH_STENCIL;
			textureBarrierInfo.access = D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
			break;
		}
		case cr3d::TextureLayout::DepthStencilReadOnly:
		{
			textureBarrierInfo.sync = D3D12_BARRIER_SYNC_DEPTH_STENCIL;
			textureBarrierInfo.access = D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
			break;
		}
		case cr3d::TextureLayout::ShaderInput:
		{
			textureBarrierInfo.sync |= GetD3D12BarrierShaderSyncFlags(textureState.stages);
			textureBarrierInfo.access = D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
			break;
		}
		case cr3d::TextureLayout::RWTexture:
		{
			textureBarrierInfo.sync |= GetD3D12BarrierShaderSyncFlags(textureState.stages);
			textureBarrierInfo.access = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
			break;
		}
		case cr3d::TextureLayout::CopySource:
		{
			textureBarrierInfo.sync = D3D12_BARRIER_SYNC_COPY;
			textureBarrierInfo.access = D3D12_BARRIER_ACCESS_COPY_SOURCE;
			break;
		}
		case cr3d::TextureLayout::CopyDestination:
		{
			textureBarrierInfo.sync = D3D12_BARRIER_SYNC_COPY;
			textureBarrierInfo.access = D3D12_BARRIER_ACCESS_COPY_DEST;
			break;
		}
		case cr3d::TextureLayout::Present:
		{
			textureBarrierInfo.sync = D3D12_BARRIER_SYNC_ALL;
			textureBarrierInfo.access = D3D12_BARRIER_ACCESS_COMMON;
			break;
		}
		default:
			CrAssertMsg(false, "Unhandled texture state");
			break;
	}

	return textureBarrierInfo;
}

crd3d::BufferBarrierInfoD3D12 crd3d::GetD3D12BufferBarrierInfo(cr3d::BufferState::T bufferState, cr3d::ShaderStageFlags::T shaderStages)
{
	crd3d::BufferBarrierInfoD3D12 bufferBarrierInfo;

	switch (bufferState)
	{
		case cr3d::BufferState::ShaderInput:
			bufferBarrierInfo.sync = GetD3D12BarrierShaderSyncFlags(shaderStages);
			bufferBarrierInfo.access = D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
			break;
		case cr3d::BufferState::ReadWrite:
			bufferBarrierInfo.sync = GetD3D12BarrierShaderSyncFlags(shaderStages);
			bufferBarrierInfo.access = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
			break;
		case cr3d::BufferState::CopySource:
			bufferBarrierInfo.sync = D3D12_BARRIER_SYNC_COPY;
			bufferBarrierInfo.access = D3D12_BARRIER_ACCESS_COPY_SOURCE;
			break;
		case cr3d::BufferState::CopyDestination:
			bufferBarrierInfo.sync = D3D12_BARRIER_SYNC_COPY;
			bufferBarrierInfo.access = D3D12_BARRIER_ACCESS_COPY_DEST;
			break;
		case cr3d::BufferState::IndirectArgument:
			bufferBarrierInfo.sync = D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;
			bufferBarrierInfo.access = D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;
			break;
		case cr3d::BufferState::Undefined:
			bufferBarrierInfo.sync = D3D12_BARRIER_SYNC_NONE;
			bufferBarrierInfo.access = D3D12_BARRIER_ACCESS_NO_ACCESS;
			break;
		default:
			CrAssertMsg(false, "Unhandled buffer state");
			break;
	}

	return bufferBarrierInfo;
}

D3D12_BARRIER_LAYOUT crd3d::GetD3D12BarrierTextureLayout(const cr3d::TextureLayout::T textureLayout)
{
	switch (textureLayout)
	{
		case cr3d::TextureLayout::ShaderInput:
			return D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
		case cr3d::TextureLayout::RenderTarget:
			return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
		case cr3d::TextureLayout::RWTexture:
			return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
		case cr3d::TextureLayout::Present:
			return D3D12_BARRIER_LAYOUT_PRESENT;
		case cr3d::TextureLayout::CopySource:
			return D3D12_BARRIER_LAYOUT_COPY_SOURCE;
		case cr3d::TextureLayout::CopyDestination:
			return D3D12_BARRIER_LAYOUT_COPY_DEST;
		case cr3d::TextureLayout::DepthStencilReadWrite:
		case cr3d::TextureLayout::DepthStencilWrite:
			return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
		default:
			CrAssertMsg(false, "Unhandled texture layout");
			return D3D12_BARRIER_LAYOUT_COMMON;
	}
}
