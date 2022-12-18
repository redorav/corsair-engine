#include "CrRendering_pch.h"

#define VMA_IMPLEMENTATION
#include "CrVMA.h"

#include "CrVulkan.h"

#include "Core/Logging/ICrDebug.h"

#include "Rendering/CrRendering.h"

VkFormat crvk::GetVkFormat(cr3d::DataFormat::T format)
{
	switch (format)
	{
		//-------------
		// Uncompressed
		//-------------

		// 8-bit formats
		case cr3d::DataFormat::R8_Unorm:          return VK_FORMAT_R8_UNORM;
		case cr3d::DataFormat::R8_Snorm:          return VK_FORMAT_R8_SNORM;
		case cr3d::DataFormat::R8_Uint:           return VK_FORMAT_R8_UINT;
		case cr3d::DataFormat::R8_Sint:           return VK_FORMAT_R8_SINT;

		case cr3d::DataFormat::RG8_Unorm:         return VK_FORMAT_R8G8_UNORM;
		case cr3d::DataFormat::RG8_Snorm:         return VK_FORMAT_R8G8_SNORM;
		case cr3d::DataFormat::RG8_Uint:          return VK_FORMAT_R8G8_UINT;
		case cr3d::DataFormat::RG8_Sint:          return VK_FORMAT_R8G8_SINT;

		case cr3d::DataFormat::RGBA8_Unorm:       return VK_FORMAT_R8G8B8A8_UNORM;
		case cr3d::DataFormat::RGBA8_Snorm:       return VK_FORMAT_R8G8B8A8_SNORM;
		case cr3d::DataFormat::RGBA8_Uint:        return VK_FORMAT_R8G8B8A8_UINT;
		case cr3d::DataFormat::RGBA8_Sint:        return VK_FORMAT_R8G8B8A8_SINT;
		case cr3d::DataFormat::RGBA8_SRGB:        return VK_FORMAT_R8G8B8A8_SRGB;

		case cr3d::DataFormat::BGRA8_Unorm:       return VK_FORMAT_B8G8R8A8_UNORM;
		case cr3d::DataFormat::BGRA8_SRGB:        return VK_FORMAT_B8G8R8A8_SRGB;

		// 16-bit integer formats
		case cr3d::DataFormat::R16_Unorm:         return VK_FORMAT_R16_UNORM;
		case cr3d::DataFormat::R16_Snorm:         return VK_FORMAT_R16_SNORM;
		case cr3d::DataFormat::R16_Uint:          return VK_FORMAT_R16_UINT;
		case cr3d::DataFormat::R16_Sint:          return VK_FORMAT_R16_SINT;

		case cr3d::DataFormat::RG16_Unorm:        return VK_FORMAT_R16G16_UNORM;
		case cr3d::DataFormat::RG16_Snorm:        return VK_FORMAT_R16G16_SNORM;
		case cr3d::DataFormat::RG16_Uint:         return VK_FORMAT_R16G16_UINT;
		case cr3d::DataFormat::RG16_Sint:         return VK_FORMAT_R16G16_SINT;

		case cr3d::DataFormat::RGBA16_Unorm:      return VK_FORMAT_R16G16B16A16_UNORM;
		case cr3d::DataFormat::RGBA16_Snorm:      return VK_FORMAT_R16G16B16A16_SNORM;
		case cr3d::DataFormat::RGBA16_Uint:       return VK_FORMAT_R16G16B16A16_UINT;
		case cr3d::DataFormat::RGBA16_Sint:       return VK_FORMAT_R16G16B16A16_SINT;

		// 16-bit float formats
		case cr3d::DataFormat::R16_Float:         return VK_FORMAT_R16_SFLOAT;
		case cr3d::DataFormat::RG16_Float:        return VK_FORMAT_R16G16_SFLOAT;
		case cr3d::DataFormat::RGBA16_Float:      return VK_FORMAT_R16G16B16A16_SFLOAT;

		case cr3d::DataFormat::R32_Uint:          return VK_FORMAT_R32_UINT;
		case cr3d::DataFormat::R32_Sint:          return VK_FORMAT_R32_SINT;
		case cr3d::DataFormat::RG32_Uint:         return VK_FORMAT_R32G32_UINT;
		case cr3d::DataFormat::RG32_Sint:         return VK_FORMAT_R32G32_SINT;
		case cr3d::DataFormat::RGB32_Uint:        return VK_FORMAT_R32G32B32_UINT;
		case cr3d::DataFormat::RGB32_Sint:        return VK_FORMAT_R32G32B32_SINT;
		case cr3d::DataFormat::RGBA32_Uint:       return VK_FORMAT_R32G32B32A32_UINT;
		case cr3d::DataFormat::RGBA32_Sint:       return VK_FORMAT_R32G32B32A32_SINT;

		// 32-bit float formats
		case cr3d::DataFormat::R32_Float:         return VK_FORMAT_R32_SFLOAT;
		case cr3d::DataFormat::RG32_Float:        return VK_FORMAT_R32G32_SFLOAT;
		case cr3d::DataFormat::RGB32_Float:       return VK_FORMAT_R32G32B32_SFLOAT;
		case cr3d::DataFormat::RGBA32_Float:      return VK_FORMAT_R32G32B32A32_SFLOAT;

		// Compressed formats
		case cr3d::DataFormat::BC1_RGB_Unorm:     return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
		case cr3d::DataFormat::BC1_RGB_SRGB:      return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
		case cr3d::DataFormat::BC1_RGBA_Unorm:    return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
		case cr3d::DataFormat::BC1_RGBA_SRGB:     return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;

		case cr3d::DataFormat::BC2_Unorm:         return VK_FORMAT_BC2_UNORM_BLOCK;
		case cr3d::DataFormat::BC2_SRGB:          return VK_FORMAT_BC2_SRGB_BLOCK;

		case cr3d::DataFormat::BC3_Unorm:         return VK_FORMAT_BC3_UNORM_BLOCK;
		case cr3d::DataFormat::BC3_SRGB:          return VK_FORMAT_BC3_SRGB_BLOCK;

		case cr3d::DataFormat::BC4_Unorm:         return VK_FORMAT_BC4_UNORM_BLOCK;
		case cr3d::DataFormat::BC4_Snorm:         return VK_FORMAT_BC4_SNORM_BLOCK;

		case cr3d::DataFormat::BC5_Unorm:         return VK_FORMAT_BC5_UNORM_BLOCK;
		case cr3d::DataFormat::BC5_Snorm:         return VK_FORMAT_BC5_SNORM_BLOCK;

		case cr3d::DataFormat::BC6H_UFloat:       return VK_FORMAT_BC6H_UFLOAT_BLOCK;
		case cr3d::DataFormat::BC6H_SFloat:       return VK_FORMAT_BC6H_SFLOAT_BLOCK;

		case cr3d::DataFormat::BC7_Unorm:         return VK_FORMAT_BC7_UNORM_BLOCK;
		case cr3d::DataFormat::BC7_SRGB:          return VK_FORMAT_BC7_SRGB_BLOCK;

		case cr3d::DataFormat::ETC2_RGB8_Unorm:   return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
		case cr3d::DataFormat::ETC2_RGB8_SRGB:    return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;

		case cr3d::DataFormat::ETC2_RGB8A1_Unorm: return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
		case cr3d::DataFormat::ETC2_RGB8A1_SRGB:  return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;

		case cr3d::DataFormat::ETC2_RGBA8_Unorm:  return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
		case cr3d::DataFormat::ETC2_RGBA8_SRGB:   return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;

		case cr3d::DataFormat::EAC_R11_Unorm:     return VK_FORMAT_EAC_R11_UNORM_BLOCK;
		case cr3d::DataFormat::EAC_R11_Snorm:     return VK_FORMAT_EAC_R11_SNORM_BLOCK;
		case cr3d::DataFormat::EAC_R11G11_Unorm:  return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
		case cr3d::DataFormat::EAC_R11G11_Snorm:  return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;

		// ASTC
		case cr3d::DataFormat::ASTC_4x4_Unorm:    return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
		case cr3d::DataFormat::ASTC_4x4_SRGB:     return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
		case cr3d::DataFormat::ASTC_5x4_Unorm:    return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
		case cr3d::DataFormat::ASTC_5x4_SRGB:     return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
		case cr3d::DataFormat::ASTC_5x5_Unorm:    return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
		case cr3d::DataFormat::ASTC_5x5_SRGB:     return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
		case cr3d::DataFormat::ASTC_6x5_Unorm:    return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
		case cr3d::DataFormat::ASTC_6x5_SRGB:     return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
		case cr3d::DataFormat::ASTC_6x6_Unorm:    return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
		case cr3d::DataFormat::ASTC_6x6_SRGB:     return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
		case cr3d::DataFormat::ASTC_8x5_Unorm:    return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
		case cr3d::DataFormat::ASTC_8x5_SRGB:     return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
		case cr3d::DataFormat::ASTC_8x6_Unorm:    return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
		case cr3d::DataFormat::ASTC_8x6_SRGB:     return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
		case cr3d::DataFormat::ASTC_8x8_Unorm:    return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
		case cr3d::DataFormat::ASTC_8x8_SRGB:     return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
		case cr3d::DataFormat::ASTC_10x5_Unorm:   return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
		case cr3d::DataFormat::ASTC_10x5_SRGB:    return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
		case cr3d::DataFormat::ASTC_10x6_Unorm:   return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
		case cr3d::DataFormat::ASTC_10x6_SRGB:    return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
		case cr3d::DataFormat::ASTC_10x8_Unorm:   return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
		case cr3d::DataFormat::ASTC_10x8_SRGB:    return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
		case cr3d::DataFormat::ASTC_10x10_Unorm:  return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
		case cr3d::DataFormat::ASTC_10x10_SRGB:   return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
		case cr3d::DataFormat::ASTC_12x10_Unorm:  return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
		case cr3d::DataFormat::ASTC_12x10_SRGB:   return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
		case cr3d::DataFormat::ASTC_12x12_Unorm:  return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
		case cr3d::DataFormat::ASTC_12x12_SRGB:   return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;

		// PVRTC
		case cr3d::DataFormat::PVRTC1_2BPP_Unorm: return VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;
		case cr3d::DataFormat::PVRTC1_2BPP_SRGB:  return VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG;
		case cr3d::DataFormat::PVRTC1_4BPP_Unorm: return VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;
		case cr3d::DataFormat::PVRTC1_4BPP_SRGB:  return VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG;
		case cr3d::DataFormat::PVRTC2_2BPP_Unorm: return VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG;
		case cr3d::DataFormat::PVRTC2_2BPP_SRGB:  return VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG;
		case cr3d::DataFormat::PVRTC2_4BPP_Unorm: return VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG;
		case cr3d::DataFormat::PVRTC2_4BPP_SRGB:  return VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG;

		// Depth-stencil formats
		case cr3d::DataFormat::D16_Unorm:         return VK_FORMAT_D16_UNORM;
		case cr3d::DataFormat::D24_Unorm_S8_Uint: return VK_FORMAT_D24_UNORM_S8_UINT;
		case cr3d::DataFormat::D24_Unorm_X8:      return VK_FORMAT_X8_D24_UNORM_PACK32;
		case cr3d::DataFormat::D32_Float:         return VK_FORMAT_D32_SFLOAT;
		case cr3d::DataFormat::D32_Float_S8_Uint: return VK_FORMAT_D32_SFLOAT_S8_UINT;

		// Packed Formats
		case cr3d::DataFormat::RG11B10_Float:  return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
		case cr3d::DataFormat::RGB10A2_Unorm:  return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		case cr3d::DataFormat::RGB10A2_Uint:   return VK_FORMAT_A2B10G10R10_UINT_PACK32;
		case cr3d::DataFormat::B5G6R5_Unorm:   return VK_FORMAT_B5G6R5_UNORM_PACK16;
		case cr3d::DataFormat::B5G5R5A1_Unorm: return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
		case cr3d::DataFormat::BGRA4_Unorm:    return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
		case cr3d::DataFormat::RGB9E5_Float:   return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;

		default:
			CrAssertMsg(false, "Format not found");
			return VK_FORMAT_UNDEFINED;
	}
}

cr3d::DataFormat::T crvk::GetDataFormat(VkFormat vkFormat)
{
	switch (vkFormat)
	{
		//-------------
		// Uncompressed
		//-------------

		// 8-bit formats
		case VK_FORMAT_R8_UNORM:          return cr3d::DataFormat::R8_Unorm;
		case VK_FORMAT_R8_SNORM:          return cr3d::DataFormat::R8_Snorm;
		case VK_FORMAT_R8_UINT:           return cr3d::DataFormat::R8_Uint;
		case VK_FORMAT_R8_SINT:           return cr3d::DataFormat::R8_Sint;

		case VK_FORMAT_R8G8_UNORM:        return cr3d::DataFormat::RG8_Unorm;
		case VK_FORMAT_R8G8_SNORM:        return cr3d::DataFormat::RG8_Snorm;
		case VK_FORMAT_R8G8_UINT:         return cr3d::DataFormat::RG8_Uint;
		case VK_FORMAT_R8G8_SINT:         return cr3d::DataFormat::RG8_Sint;

		case VK_FORMAT_R8G8B8A8_UNORM:    return cr3d::DataFormat::RGBA8_Unorm;
		case VK_FORMAT_R8G8B8A8_SNORM:    return cr3d::DataFormat::RGBA8_Snorm;
		case VK_FORMAT_R8G8B8A8_UINT:     return cr3d::DataFormat::RGBA8_Uint;
		case VK_FORMAT_R8G8B8A8_SINT:     return cr3d::DataFormat::RGBA8_Sint;
		case VK_FORMAT_R8G8B8A8_SRGB:     return cr3d::DataFormat::RGBA8_SRGB;

		case VK_FORMAT_B8G8R8A8_UNORM:    return cr3d::DataFormat::BGRA8_Unorm;
		case VK_FORMAT_B8G8R8A8_SRGB:     return cr3d::DataFormat::BGRA8_SRGB;

		// 16-bit integer formats
		case VK_FORMAT_R16_UNORM:         return cr3d::DataFormat::R16_Unorm;
		case VK_FORMAT_R16_SNORM:         return cr3d::DataFormat::R16_Snorm;
		case VK_FORMAT_R16_UINT:          return cr3d::DataFormat::R16_Uint;
		case VK_FORMAT_R16_SINT:          return cr3d::DataFormat::R16_Sint;

		case VK_FORMAT_R16G16_UNORM:      return cr3d::DataFormat::RG16_Unorm;
		case VK_FORMAT_R16G16_SNORM:      return cr3d::DataFormat::RG16_Snorm;
		case VK_FORMAT_R16G16_UINT:       return cr3d::DataFormat::RG16_Uint;
		case VK_FORMAT_R16G16_SINT:       return cr3d::DataFormat::RG16_Sint;

		case VK_FORMAT_R16G16B16A16_UNORM: return cr3d::DataFormat::RGBA16_Unorm;
		case VK_FORMAT_R16G16B16A16_SNORM: return cr3d::DataFormat::RGBA16_Snorm;
		case VK_FORMAT_R16G16B16A16_UINT:  return cr3d::DataFormat::RGBA16_Uint;
		case VK_FORMAT_R16G16B16A16_SINT:  return cr3d::DataFormat::RGBA16_Sint;

		// 16-bit float formats
		case VK_FORMAT_R16_SFLOAT:          return cr3d::DataFormat::R16_Float;
		case VK_FORMAT_R16G16_SFLOAT:       return cr3d::DataFormat::RG16_Float;
		case VK_FORMAT_R16G16B16A16_SFLOAT: return cr3d::DataFormat::RGBA16_Float;

		case VK_FORMAT_R32_UINT:            return cr3d::DataFormat::R32_Uint;
		case VK_FORMAT_R32_SINT:            return cr3d::DataFormat::R32_Sint;
		case VK_FORMAT_R32G32_UINT:         return cr3d::DataFormat::RG32_Uint;
		case VK_FORMAT_R32G32_SINT:         return cr3d::DataFormat::RG32_Sint;
		case VK_FORMAT_R32G32B32_UINT:      return cr3d::DataFormat::RGB32_Uint;
		case VK_FORMAT_R32G32B32_SINT:      return cr3d::DataFormat::RGB32_Sint;
		case VK_FORMAT_R32G32B32A32_UINT:   return cr3d::DataFormat::RGBA32_Uint;
		case VK_FORMAT_R32G32B32A32_SINT:   return cr3d::DataFormat::RGBA32_Sint;

		// 32-bit float formats
		case VK_FORMAT_R32_SFLOAT:          return cr3d::DataFormat::R32_Float;
		case VK_FORMAT_R32G32_SFLOAT:       return cr3d::DataFormat::RG32_Float;
		case VK_FORMAT_R32G32B32_SFLOAT:    return cr3d::DataFormat::RGB32_Float;
		case VK_FORMAT_R32G32B32A32_SFLOAT: return cr3d::DataFormat::RGBA32_Float;

		// Depth-stencil formats
		case VK_FORMAT_D16_UNORM:           return cr3d::DataFormat::D16_Unorm;
		case VK_FORMAT_D24_UNORM_S8_UINT:   return cr3d::DataFormat::D24_Unorm_S8_Uint;
		case VK_FORMAT_X8_D24_UNORM_PACK32: return cr3d::DataFormat::D24_Unorm_X8;
		case VK_FORMAT_D32_SFLOAT:          return cr3d::DataFormat::D32_Float;
		case VK_FORMAT_D32_SFLOAT_S8_UINT:  return cr3d::DataFormat::D32_Float_S8_Uint;

		// Packed Formats
		case VK_FORMAT_B10G11R11_UFLOAT_PACK32:   return cr3d::DataFormat::RG11B10_Float;
		case VK_FORMAT_A2B10G10R10_UNORM_PACK32:  return cr3d::DataFormat::RGB10A2_Unorm;
		case VK_FORMAT_A2B10G10R10_UINT_PACK32:   return cr3d::DataFormat::RGB10A2_Uint;
		case VK_FORMAT_B5G6R5_UNORM_PACK16:       return cr3d::DataFormat::B5G6R5_Unorm;
		case VK_FORMAT_B5G5R5A1_UNORM_PACK16:     return cr3d::DataFormat::B5G5R5A1_Unorm;
		case VK_FORMAT_B4G4R4A4_UNORM_PACK16:     return cr3d::DataFormat::BGRA4_Unorm;
		case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:    return cr3d::DataFormat::RGB9E5_Float;

		default:
			CrAssertMsg(false, "Format not found");
			return cr3d::DataFormat::Invalid;
	}
}

VkSamplerAddressMode crvk::GetVkAddressMode(cr3d::AddressMode addressMode)
{
	switch (addressMode)
	{
		case cr3d::AddressMode::ClampToEdge:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case cr3d::AddressMode::ClampToBorder:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case cr3d::AddressMode::Wrap:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case cr3d::AddressMode::Mirror:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case cr3d::AddressMode::MirrorOnce:
			return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
	}

	return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
}

VkFilter crvk::GetVkFilter(cr3d::Filter filter)
{
	switch (filter)
	{
		case cr3d::Filter::Point:
			return VK_FILTER_NEAREST;
		case cr3d::Filter::Linear:
			return VK_FILTER_LINEAR;
	}

	return VK_FILTER_MAX_ENUM;
}

VkSamplerMipmapMode crvk::GetVkMipmapMode(cr3d::Filter filter)
{
	switch (filter)
	{
		case cr3d::Filter::Point:
			return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case cr3d::Filter::Linear:
			return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}

	return VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
}

VkBorderColor crvk::GetVkBorderColor(cr3d::BorderColor borderColor)
{
	switch (borderColor)
	{
		case cr3d::BorderColor::OpaqueBlack:
			return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		case cr3d::BorderColor::TransparentBlack:
			return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		case cr3d::BorderColor::OpaqueWhite:
			return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	}

	return VK_BORDER_COLOR_MAX_ENUM;
}

VkBlendOp crvk::GetVkBlendOp(cr3d::BlendOp blendOp)
{
	switch (blendOp)
	{
		case cr3d::BlendOp::Add:
			return VK_BLEND_OP_ADD;
		case cr3d::BlendOp::Subtract:
			return VK_BLEND_OP_SUBTRACT;
		case cr3d::BlendOp::ReverseSubtract:
			return VK_BLEND_OP_REVERSE_SUBTRACT;
		case cr3d::BlendOp::Min:
			return VK_BLEND_OP_MIN;
		case cr3d::BlendOp::Max:
			return VK_BLEND_OP_MAX;
	}
	return VK_BLEND_OP_MAX_ENUM;
}

VkBlendFactor crvk::GetVkBlendFactor(cr3d::BlendFactor blendFactor)
{
	switch (blendFactor)
	{
		case cr3d::BlendFactor::Zero:
			return VK_BLEND_FACTOR_ZERO;
		case cr3d::BlendFactor::One:
			return VK_BLEND_FACTOR_ONE;
		case cr3d::BlendFactor::SrcColor:
			return VK_BLEND_FACTOR_SRC_COLOR;
		case cr3d::BlendFactor::OneMinusSrcColor:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case cr3d::BlendFactor::DstColor:
			return VK_BLEND_FACTOR_DST_COLOR;
		case cr3d::BlendFactor::OneMinusDstColor:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case cr3d::BlendFactor::SrcAlpha:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case cr3d::BlendFactor::OneMinusSrcAlpha:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case cr3d::BlendFactor::DstAlpha:
			return VK_BLEND_FACTOR_DST_ALPHA;
		case cr3d::BlendFactor::OneMinusDstAlpha:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case cr3d::BlendFactor::Constant:
			return VK_BLEND_FACTOR_CONSTANT_COLOR;
		case cr3d::BlendFactor::OneMinusConstant:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		case cr3d::BlendFactor::SrcAlphaSaturate:
			return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		case cr3d::BlendFactor::Src1Color:
			return VK_BLEND_FACTOR_SRC1_COLOR;
		case cr3d::BlendFactor::OneMinusSrc1Color:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		case cr3d::BlendFactor::Src1Alpha:
			return VK_BLEND_FACTOR_SRC1_ALPHA;
		case cr3d::BlendFactor::OneMinusSrc1Alpha:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
	}
	return VK_BLEND_FACTOR_MAX_ENUM;
}

VkCompareOp crvk::GetVkCompareOp(cr3d::CompareOp compareOp)
{
	switch (compareOp)
	{
		case cr3d::CompareOp::Never:
			return VK_COMPARE_OP_NEVER;
		case cr3d::CompareOp::Less:
			return VK_COMPARE_OP_LESS;
		case cr3d::CompareOp::Equal:
			return VK_COMPARE_OP_EQUAL;
		case cr3d::CompareOp::LessOrEqual:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case cr3d::CompareOp::Greater:
			return VK_COMPARE_OP_GREATER;
		case cr3d::CompareOp::NotEqual:
			return VK_COMPARE_OP_NOT_EQUAL;
		case cr3d::CompareOp::GreaterOrEqual:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case cr3d::CompareOp::Always:
			return VK_COMPARE_OP_ALWAYS;
	}
	return VK_COMPARE_OP_MAX_ENUM;
}

VkStencilOp crvk::GetVkStencilOp(cr3d::StencilOp stencilOp)
{
	switch (stencilOp)
	{
		case cr3d::StencilOp::Keep:
			return VK_STENCIL_OP_KEEP;
		case cr3d::StencilOp::Zero:
			return VK_STENCIL_OP_ZERO;
		case cr3d::StencilOp::Replace:
			return VK_STENCIL_OP_REPLACE;
		case cr3d::StencilOp::IncrementSaturate:
			return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		case cr3d::StencilOp::DecrementSaturate:
			return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		case cr3d::StencilOp::Invert:
			return VK_STENCIL_OP_INVERT;
		case cr3d::StencilOp::IncrementAndWrap:
			return VK_STENCIL_OP_INCREMENT_AND_WRAP;
		case cr3d::StencilOp::DecrementAndWrap:
			return VK_STENCIL_OP_DECREMENT_AND_WRAP;
	}
	return VK_STENCIL_OP_MAX_ENUM;
}

VkSampleCountFlagBits crvk::GetVkSampleCount(cr3d::SampleCount sampleCount)
{
	switch (sampleCount)
	{
		case cr3d::SampleCount::S1:
			return VK_SAMPLE_COUNT_1_BIT;
		case cr3d::SampleCount::S2:
			return VK_SAMPLE_COUNT_2_BIT;
		case cr3d::SampleCount::S4:
			return VK_SAMPLE_COUNT_4_BIT;
		case cr3d::SampleCount::S8:
			return VK_SAMPLE_COUNT_8_BIT;
		case cr3d::SampleCount::S16:
			return VK_SAMPLE_COUNT_16_BIT;
		case cr3d::SampleCount::S32:
			return VK_SAMPLE_COUNT_32_BIT;
		case cr3d::SampleCount::S64:
			return VK_SAMPLE_COUNT_64_BIT;
	}
	return VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
}

VkShaderStageFlagBits crvk::GetVkShaderStage(cr3d::ShaderStage::T shaderStage)
{
	switch (shaderStage)
	{
		case cr3d::ShaderStage::Vertex:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case cr3d::ShaderStage::Geometry:
			return VK_SHADER_STAGE_GEOMETRY_BIT;
		case cr3d::ShaderStage::Hull:
			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case cr3d::ShaderStage::Domain:
			return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case cr3d::ShaderStage::Pixel:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case cr3d::ShaderStage::Compute:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		default:
			return VK_SHADER_STAGE_ALL;
	}
}

VkPrimitiveTopology crvk::GetVkPrimitiveTopology(cr3d::PrimitiveTopology primitiveTopology)
{
	switch (primitiveTopology)
	{
		case cr3d::PrimitiveTopology::PointList:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case cr3d::PrimitiveTopology::LineList:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case cr3d::PrimitiveTopology::LineStrip:
			return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case cr3d::PrimitiveTopology::TriangleList:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case cr3d::PrimitiveTopology::TriangleStrip:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		case cr3d::PrimitiveTopology::LineListAdjacency:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
		case cr3d::PrimitiveTopology::LineStripAdjacency:
			return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
		case cr3d::PrimitiveTopology::TriangleListAdjacency:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
		case cr3d::PrimitiveTopology::TriangleStripAdjacency:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
		case cr3d::PrimitiveTopology::PatchList:
			return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
	}

	return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}

VkVertexInputRate crvk::GetVkVertexInputRate(cr3d::VertexInputRate vertexInputRate)
{
	switch (vertexInputRate)
	{
		case cr3d::VertexInputRate::Vertex: return VK_VERTEX_INPUT_RATE_VERTEX;
		case cr3d::VertexInputRate::Instance: return VK_VERTEX_INPUT_RATE_INSTANCE;
	}

	return VK_VERTEX_INPUT_RATE_MAX_ENUM;
}

VkPolygonMode crvk::GetVkPolygonFillMode(cr3d::PolygonFillMode fillMode)
{
	switch (fillMode)
	{
		case cr3d::PolygonFillMode::Fill:
			return VK_POLYGON_MODE_FILL;
		case cr3d::PolygonFillMode::Line:
			return VK_POLYGON_MODE_LINE;
		default:
			return VK_POLYGON_MODE_MAX_ENUM;
	}
}

VkCullModeFlags crvk::GetVkPolygonCullMode(cr3d::PolygonCullMode cullMode)
{
	switch (cullMode)
	{
		case cr3d::PolygonCullMode::Back:
			return VK_CULL_MODE_BACK_BIT;
		case cr3d::PolygonCullMode::Front:
			return VK_CULL_MODE_FRONT_BIT;
		case cr3d::PolygonCullMode::None:
			return VK_CULL_MODE_NONE;
		default:
			return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
	}
}

VkFrontFace crvk::GetVkFrontFace(cr3d::FrontFace frontFace)
{
	switch (frontFace)
	{
		case cr3d::FrontFace::Clockwise:
			return VK_FRONT_FACE_CLOCKWISE;
		case cr3d::FrontFace::CounterClockwise:
			return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		default:
			return VK_FRONT_FACE_MAX_ENUM;
	}
}

VkDescriptorType crvk::GetVkDescriptorType(cr3d::ShaderResourceType::T resourceType)
{
	switch (resourceType)
	{
		case cr3d::ShaderResourceType::ConstantBuffer:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC; // TODO Fix this, we need to be able to tell whether dynamic or not
		case cr3d::ShaderResourceType::Texture:
			return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case cr3d::ShaderResourceType::RWTexture:
			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case cr3d::ShaderResourceType::Sampler:
			return VK_DESCRIPTOR_TYPE_SAMPLER;
		case cr3d::ShaderResourceType::StorageBuffer:
		case cr3d::ShaderResourceType::RWStorageBuffer:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case cr3d::ShaderResourceType::DataBuffer:
			return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		case cr3d::ShaderResourceType::RWDataBuffer:
			return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		default:
			return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}

	//VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER = 1,
	//VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER = 6,
	//VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC = 9,
	//VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT = 10,
}

VkAttachmentLoadOp crvk::GetVkAttachmentLoadOp(CrRenderTargetLoadOp loadOp)
{
	switch (loadOp)
	{
		case CrRenderTargetLoadOp::Clear:
			return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case CrRenderTargetLoadOp::DontCare:
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		case CrRenderTargetLoadOp::Load:
			return VK_ATTACHMENT_LOAD_OP_LOAD;
		default:
			return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
	}
}

VkAttachmentStoreOp crvk::GetVkAttachmentStoreOp(CrRenderTargetStoreOp storeOp)
{
	switch (storeOp)
	{
		case CrRenderTargetStoreOp::DontCare:
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		case CrRenderTargetStoreOp::Store:
			return VK_ATTACHMENT_STORE_OP_STORE;
		default:
			return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
	}
}

VkPipelineStageFlags crvk::GetVkPipelineStageFlagsFromShaderStages(cr3d::ShaderStageFlags::T shaderStages)
{
	VkPipelineStageFlags pipelineFlags = 0;

	if (shaderStages & cr3d::ShaderStageFlags::Vertex)
	{
		pipelineFlags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	}

	if (shaderStages & cr3d::ShaderStageFlags::Pixel)
	{
		pipelineFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	if (shaderStages & cr3d::ShaderStageFlags::Hull)
	{
		pipelineFlags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
	}

	if (shaderStages & cr3d::ShaderStageFlags::Domain)
	{
		pipelineFlags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
	}

	if (shaderStages & cr3d::ShaderStageFlags::Geometry)
	{
		pipelineFlags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	}

	if (shaderStages & cr3d::ShaderStageFlags::Compute)
	{
		pipelineFlags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	}

	return pipelineFlags;
}

VkBufferCreateInfo crvk::CreateVkBufferCreateInfo
(
	VkBufferCreateFlags flags, VkDeviceSize size, VkBufferUsageFlags usage,
	VkSharingMode sharingMode, uint32_t queueFamilyIndexCount, uint32_t* pQueueFamilyIndices
)
{
	return { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, flags, size, usage, sharingMode, queueFamilyIndexCount, pQueueFamilyIndices };
}

VkMemoryAllocateInfo crvk::CreateVkMemoryAllocateInfo(VkDeviceSize allocationSize, uint32_t memoryTypeIndex, void* extension /*= nullptr*/)
{
	return { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, extension, allocationSize, memoryTypeIndex };
}

VkWriteDescriptorSet crvk::CreateVkWriteDescriptorSet
(
	VkDescriptorSet descriptorSet, uint32_t binding, uint32_t arrayElement, 
	uint32_t descriptorCount, VkDescriptorType descriptorType, 
	const VkDescriptorImageInfo* imageInfo, const VkDescriptorBufferInfo* bufferInfo, const VkBufferView* texelBufferView
)
{
	VkWriteDescriptorSet writeDescriptorSet =
	{
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, 
		nullptr, descriptorSet, binding, arrayElement, 
		descriptorCount, descriptorType, imageInfo, 
		bufferInfo, texelBufferView
	};

	return writeDescriptorSet;
}
