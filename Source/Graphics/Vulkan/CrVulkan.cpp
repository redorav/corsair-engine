#include "Graphics/CrRendering_pch.h"

#define VMA_IMPLEMENTATION
#include "CrVMA.h"

#include "CrVulkan.h"

#include "Core/Logging/ICrDebug.h"

#include "Graphics/CrRendering.h"

VkFormat crvk::GetVkFormat(crgfx::DataFormat::T format)
{
	switch (format)
	{
		//-------------
		// Uncompressed
		//-------------

		// 8-bit formats
		case crgfx::DataFormat::R8_Unorm:          return VK_FORMAT_R8_UNORM;
		case crgfx::DataFormat::R8_Snorm:          return VK_FORMAT_R8_SNORM;
		case crgfx::DataFormat::R8_Uint:           return VK_FORMAT_R8_UINT;
		case crgfx::DataFormat::R8_Sint:           return VK_FORMAT_R8_SINT;

		case crgfx::DataFormat::RG8_Unorm:         return VK_FORMAT_R8G8_UNORM;
		case crgfx::DataFormat::RG8_Snorm:         return VK_FORMAT_R8G8_SNORM;
		case crgfx::DataFormat::RG8_Uint:          return VK_FORMAT_R8G8_UINT;
		case crgfx::DataFormat::RG8_Sint:          return VK_FORMAT_R8G8_SINT;

		case crgfx::DataFormat::RGBA8_Unorm:       return VK_FORMAT_R8G8B8A8_UNORM;
		case crgfx::DataFormat::RGBA8_Snorm:       return VK_FORMAT_R8G8B8A8_SNORM;
		case crgfx::DataFormat::RGBA8_Uint:        return VK_FORMAT_R8G8B8A8_UINT;
		case crgfx::DataFormat::RGBA8_Sint:        return VK_FORMAT_R8G8B8A8_SINT;
		case crgfx::DataFormat::RGBA8_SRGB:        return VK_FORMAT_R8G8B8A8_SRGB;

		case crgfx::DataFormat::BGRA8_Unorm:       return VK_FORMAT_B8G8R8A8_UNORM;
		case crgfx::DataFormat::BGRA8_SRGB:        return VK_FORMAT_B8G8R8A8_SRGB;

		// 16-bit integer formats
		case crgfx::DataFormat::R16_Unorm:         return VK_FORMAT_R16_UNORM;
		case crgfx::DataFormat::R16_Snorm:         return VK_FORMAT_R16_SNORM;
		case crgfx::DataFormat::R16_Uint:          return VK_FORMAT_R16_UINT;
		case crgfx::DataFormat::R16_Sint:          return VK_FORMAT_R16_SINT;

		case crgfx::DataFormat::RG16_Unorm:        return VK_FORMAT_R16G16_UNORM;
		case crgfx::DataFormat::RG16_Snorm:        return VK_FORMAT_R16G16_SNORM;
		case crgfx::DataFormat::RG16_Uint:         return VK_FORMAT_R16G16_UINT;
		case crgfx::DataFormat::RG16_Sint:         return VK_FORMAT_R16G16_SINT;

		case crgfx::DataFormat::RGBA16_Unorm:      return VK_FORMAT_R16G16B16A16_UNORM;
		case crgfx::DataFormat::RGBA16_Snorm:      return VK_FORMAT_R16G16B16A16_SNORM;
		case crgfx::DataFormat::RGBA16_Uint:       return VK_FORMAT_R16G16B16A16_UINT;
		case crgfx::DataFormat::RGBA16_Sint:       return VK_FORMAT_R16G16B16A16_SINT;

		// 16-bit float formats
		case crgfx::DataFormat::R16_Float:         return VK_FORMAT_R16_SFLOAT;
		case crgfx::DataFormat::RG16_Float:        return VK_FORMAT_R16G16_SFLOAT;
		case crgfx::DataFormat::RGBA16_Float:      return VK_FORMAT_R16G16B16A16_SFLOAT;

		case crgfx::DataFormat::R32_Uint:          return VK_FORMAT_R32_UINT;
		case crgfx::DataFormat::R32_Sint:          return VK_FORMAT_R32_SINT;
		case crgfx::DataFormat::RG32_Uint:         return VK_FORMAT_R32G32_UINT;
		case crgfx::DataFormat::RG32_Sint:         return VK_FORMAT_R32G32_SINT;
		case crgfx::DataFormat::RGB32_Uint:        return VK_FORMAT_R32G32B32_UINT;
		case crgfx::DataFormat::RGB32_Sint:        return VK_FORMAT_R32G32B32_SINT;
		case crgfx::DataFormat::RGBA32_Uint:       return VK_FORMAT_R32G32B32A32_UINT;
		case crgfx::DataFormat::RGBA32_Sint:       return VK_FORMAT_R32G32B32A32_SINT;

		// 32-bit float formats
		case crgfx::DataFormat::R32_Float:         return VK_FORMAT_R32_SFLOAT;
		case crgfx::DataFormat::RG32_Float:        return VK_FORMAT_R32G32_SFLOAT;
		case crgfx::DataFormat::RGB32_Float:       return VK_FORMAT_R32G32B32_SFLOAT;
		case crgfx::DataFormat::RGBA32_Float:      return VK_FORMAT_R32G32B32A32_SFLOAT;

		// Compressed formats
		case crgfx::DataFormat::BC1_RGB_Unorm:     return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
		case crgfx::DataFormat::BC1_RGB_SRGB:      return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
		case crgfx::DataFormat::BC1_RGBA_Unorm:    return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
		case crgfx::DataFormat::BC1_RGBA_SRGB:     return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;

		case crgfx::DataFormat::BC2_Unorm:         return VK_FORMAT_BC2_UNORM_BLOCK;
		case crgfx::DataFormat::BC2_SRGB:          return VK_FORMAT_BC2_SRGB_BLOCK;

		case crgfx::DataFormat::BC3_Unorm:         return VK_FORMAT_BC3_UNORM_BLOCK;
		case crgfx::DataFormat::BC3_SRGB:          return VK_FORMAT_BC3_SRGB_BLOCK;

		case crgfx::DataFormat::BC4_Unorm:         return VK_FORMAT_BC4_UNORM_BLOCK;
		case crgfx::DataFormat::BC4_Snorm:         return VK_FORMAT_BC4_SNORM_BLOCK;

		case crgfx::DataFormat::BC5_Unorm:         return VK_FORMAT_BC5_UNORM_BLOCK;
		case crgfx::DataFormat::BC5_Snorm:         return VK_FORMAT_BC5_SNORM_BLOCK;

		case crgfx::DataFormat::BC6H_UFloat:       return VK_FORMAT_BC6H_UFLOAT_BLOCK;
		case crgfx::DataFormat::BC6H_SFloat:       return VK_FORMAT_BC6H_SFLOAT_BLOCK;

		case crgfx::DataFormat::BC7_Unorm:         return VK_FORMAT_BC7_UNORM_BLOCK;
		case crgfx::DataFormat::BC7_SRGB:          return VK_FORMAT_BC7_SRGB_BLOCK;

		case crgfx::DataFormat::ETC2_RGB8_Unorm:   return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
		case crgfx::DataFormat::ETC2_RGB8_SRGB:    return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;

		case crgfx::DataFormat::ETC2_RGB8A1_Unorm: return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
		case crgfx::DataFormat::ETC2_RGB8A1_SRGB:  return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;

		case crgfx::DataFormat::ETC2_RGBA8_Unorm:  return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
		case crgfx::DataFormat::ETC2_RGBA8_SRGB:   return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;

		case crgfx::DataFormat::EAC_R11_Unorm:     return VK_FORMAT_EAC_R11_UNORM_BLOCK;
		case crgfx::DataFormat::EAC_R11_Snorm:     return VK_FORMAT_EAC_R11_SNORM_BLOCK;
		case crgfx::DataFormat::EAC_R11G11_Unorm:  return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
		case crgfx::DataFormat::EAC_R11G11_Snorm:  return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;

		// ASTC
		case crgfx::DataFormat::ASTC_4x4_Unorm:    return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
		case crgfx::DataFormat::ASTC_4x4_SRGB:     return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
		case crgfx::DataFormat::ASTC_5x4_Unorm:    return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
		case crgfx::DataFormat::ASTC_5x4_SRGB:     return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
		case crgfx::DataFormat::ASTC_5x5_Unorm:    return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
		case crgfx::DataFormat::ASTC_5x5_SRGB:     return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
		case crgfx::DataFormat::ASTC_6x5_Unorm:    return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
		case crgfx::DataFormat::ASTC_6x5_SRGB:     return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
		case crgfx::DataFormat::ASTC_6x6_Unorm:    return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
		case crgfx::DataFormat::ASTC_6x6_SRGB:     return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
		case crgfx::DataFormat::ASTC_8x5_Unorm:    return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
		case crgfx::DataFormat::ASTC_8x5_SRGB:     return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
		case crgfx::DataFormat::ASTC_8x6_Unorm:    return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
		case crgfx::DataFormat::ASTC_8x6_SRGB:     return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
		case crgfx::DataFormat::ASTC_8x8_Unorm:    return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
		case crgfx::DataFormat::ASTC_8x8_SRGB:     return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
		case crgfx::DataFormat::ASTC_10x5_Unorm:   return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
		case crgfx::DataFormat::ASTC_10x5_SRGB:    return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
		case crgfx::DataFormat::ASTC_10x6_Unorm:   return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
		case crgfx::DataFormat::ASTC_10x6_SRGB:    return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
		case crgfx::DataFormat::ASTC_10x8_Unorm:   return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
		case crgfx::DataFormat::ASTC_10x8_SRGB:    return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
		case crgfx::DataFormat::ASTC_10x10_Unorm:  return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
		case crgfx::DataFormat::ASTC_10x10_SRGB:   return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
		case crgfx::DataFormat::ASTC_12x10_Unorm:  return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
		case crgfx::DataFormat::ASTC_12x10_SRGB:   return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
		case crgfx::DataFormat::ASTC_12x12_Unorm:  return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
		case crgfx::DataFormat::ASTC_12x12_SRGB:   return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;

		// PVRTC
		case crgfx::DataFormat::PVRTC1_2BPP_Unorm: return VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;
		case crgfx::DataFormat::PVRTC1_2BPP_SRGB:  return VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG;
		case crgfx::DataFormat::PVRTC1_4BPP_Unorm: return VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;
		case crgfx::DataFormat::PVRTC1_4BPP_SRGB:  return VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG;
		case crgfx::DataFormat::PVRTC2_2BPP_Unorm: return VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG;
		case crgfx::DataFormat::PVRTC2_2BPP_SRGB:  return VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG;
		case crgfx::DataFormat::PVRTC2_4BPP_Unorm: return VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG;
		case crgfx::DataFormat::PVRTC2_4BPP_SRGB:  return VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG;

		// Depth-stencil formats
		case crgfx::DataFormat::D16_Unorm:         return VK_FORMAT_D16_UNORM;
		case crgfx::DataFormat::D24_Unorm_S8_Uint: return VK_FORMAT_D24_UNORM_S8_UINT;
		case crgfx::DataFormat::D24_Unorm_X8:      return VK_FORMAT_X8_D24_UNORM_PACK32;
		case crgfx::DataFormat::D32_Float:         return VK_FORMAT_D32_SFLOAT;
		case crgfx::DataFormat::D32_Float_S8_Uint: return VK_FORMAT_D32_SFLOAT_S8_UINT;

		// Packed Formats
		case crgfx::DataFormat::RG11B10_Float:  return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
		case crgfx::DataFormat::RGB10A2_Unorm:  return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		case crgfx::DataFormat::RGB10A2_Uint:   return VK_FORMAT_A2B10G10R10_UINT_PACK32;
		case crgfx::DataFormat::B5G6R5_Unorm:   return VK_FORMAT_B5G6R5_UNORM_PACK16;
		case crgfx::DataFormat::B5G5R5A1_Unorm: return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
		case crgfx::DataFormat::BGRA4_Unorm:    return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
		case crgfx::DataFormat::RGB9E5_Float:   return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;

		default:
			CrAssertMsg(false, "Format not found");
			return VK_FORMAT_UNDEFINED;
	}
}

crgfx::DataFormat::T crvk::GetDataFormat(VkFormat vkFormat)
{
	switch (vkFormat)
	{
		//-------------
		// Uncompressed
		//-------------

		// 8-bit formats
		case VK_FORMAT_R8_UNORM:          return crgfx::DataFormat::R8_Unorm;
		case VK_FORMAT_R8_SNORM:          return crgfx::DataFormat::R8_Snorm;
		case VK_FORMAT_R8_UINT:           return crgfx::DataFormat::R8_Uint;
		case VK_FORMAT_R8_SINT:           return crgfx::DataFormat::R8_Sint;

		case VK_FORMAT_R8G8_UNORM:        return crgfx::DataFormat::RG8_Unorm;
		case VK_FORMAT_R8G8_SNORM:        return crgfx::DataFormat::RG8_Snorm;
		case VK_FORMAT_R8G8_UINT:         return crgfx::DataFormat::RG8_Uint;
		case VK_FORMAT_R8G8_SINT:         return crgfx::DataFormat::RG8_Sint;

		case VK_FORMAT_R8G8B8A8_UNORM:    return crgfx::DataFormat::RGBA8_Unorm;
		case VK_FORMAT_R8G8B8A8_SNORM:    return crgfx::DataFormat::RGBA8_Snorm;
		case VK_FORMAT_R8G8B8A8_UINT:     return crgfx::DataFormat::RGBA8_Uint;
		case VK_FORMAT_R8G8B8A8_SINT:     return crgfx::DataFormat::RGBA8_Sint;
		case VK_FORMAT_R8G8B8A8_SRGB:     return crgfx::DataFormat::RGBA8_SRGB;

		case VK_FORMAT_B8G8R8A8_UNORM:    return crgfx::DataFormat::BGRA8_Unorm;
		case VK_FORMAT_B8G8R8A8_SRGB:     return crgfx::DataFormat::BGRA8_SRGB;

		// 16-bit integer formats
		case VK_FORMAT_R16_UNORM:         return crgfx::DataFormat::R16_Unorm;
		case VK_FORMAT_R16_SNORM:         return crgfx::DataFormat::R16_Snorm;
		case VK_FORMAT_R16_UINT:          return crgfx::DataFormat::R16_Uint;
		case VK_FORMAT_R16_SINT:          return crgfx::DataFormat::R16_Sint;

		case VK_FORMAT_R16G16_UNORM:      return crgfx::DataFormat::RG16_Unorm;
		case VK_FORMAT_R16G16_SNORM:      return crgfx::DataFormat::RG16_Snorm;
		case VK_FORMAT_R16G16_UINT:       return crgfx::DataFormat::RG16_Uint;
		case VK_FORMAT_R16G16_SINT:       return crgfx::DataFormat::RG16_Sint;

		case VK_FORMAT_R16G16B16A16_UNORM: return crgfx::DataFormat::RGBA16_Unorm;
		case VK_FORMAT_R16G16B16A16_SNORM: return crgfx::DataFormat::RGBA16_Snorm;
		case VK_FORMAT_R16G16B16A16_UINT:  return crgfx::DataFormat::RGBA16_Uint;
		case VK_FORMAT_R16G16B16A16_SINT:  return crgfx::DataFormat::RGBA16_Sint;

		// 16-bit float formats
		case VK_FORMAT_R16_SFLOAT:          return crgfx::DataFormat::R16_Float;
		case VK_FORMAT_R16G16_SFLOAT:       return crgfx::DataFormat::RG16_Float;
		case VK_FORMAT_R16G16B16A16_SFLOAT: return crgfx::DataFormat::RGBA16_Float;

		case VK_FORMAT_R32_UINT:            return crgfx::DataFormat::R32_Uint;
		case VK_FORMAT_R32_SINT:            return crgfx::DataFormat::R32_Sint;
		case VK_FORMAT_R32G32_UINT:         return crgfx::DataFormat::RG32_Uint;
		case VK_FORMAT_R32G32_SINT:         return crgfx::DataFormat::RG32_Sint;
		case VK_FORMAT_R32G32B32_UINT:      return crgfx::DataFormat::RGB32_Uint;
		case VK_FORMAT_R32G32B32_SINT:      return crgfx::DataFormat::RGB32_Sint;
		case VK_FORMAT_R32G32B32A32_UINT:   return crgfx::DataFormat::RGBA32_Uint;
		case VK_FORMAT_R32G32B32A32_SINT:   return crgfx::DataFormat::RGBA32_Sint;

		// 32-bit float formats
		case VK_FORMAT_R32_SFLOAT:          return crgfx::DataFormat::R32_Float;
		case VK_FORMAT_R32G32_SFLOAT:       return crgfx::DataFormat::RG32_Float;
		case VK_FORMAT_R32G32B32_SFLOAT:    return crgfx::DataFormat::RGB32_Float;
		case VK_FORMAT_R32G32B32A32_SFLOAT: return crgfx::DataFormat::RGBA32_Float;

		// Depth-stencil formats
		case VK_FORMAT_D16_UNORM:           return crgfx::DataFormat::D16_Unorm;
		case VK_FORMAT_D24_UNORM_S8_UINT:   return crgfx::DataFormat::D24_Unorm_S8_Uint;
		case VK_FORMAT_X8_D24_UNORM_PACK32: return crgfx::DataFormat::D24_Unorm_X8;
		case VK_FORMAT_D32_SFLOAT:          return crgfx::DataFormat::D32_Float;
		case VK_FORMAT_D32_SFLOAT_S8_UINT:  return crgfx::DataFormat::D32_Float_S8_Uint;

		// Packed Formats
		case VK_FORMAT_B10G11R11_UFLOAT_PACK32:   return crgfx::DataFormat::RG11B10_Float;
		case VK_FORMAT_A2B10G10R10_UNORM_PACK32:  return crgfx::DataFormat::RGB10A2_Unorm;
		case VK_FORMAT_A2B10G10R10_UINT_PACK32:   return crgfx::DataFormat::RGB10A2_Uint;
		case VK_FORMAT_B5G6R5_UNORM_PACK16:       return crgfx::DataFormat::B5G6R5_Unorm;
		case VK_FORMAT_B5G5R5A1_UNORM_PACK16:     return crgfx::DataFormat::B5G5R5A1_Unorm;
		case VK_FORMAT_B4G4R4A4_UNORM_PACK16:     return crgfx::DataFormat::BGRA4_Unorm;
		case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:    return crgfx::DataFormat::RGB9E5_Float;

		default:
			CrAssertMsg(false, "Format not found");
			return crgfx::DataFormat::Invalid;
	}
}

VkSamplerAddressMode crvk::GetVkAddressMode(crgfx::AddressMode addressMode)
{
	switch (addressMode)
	{
		case crgfx::AddressMode::ClampToEdge:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case crgfx::AddressMode::ClampToBorder:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case crgfx::AddressMode::Wrap:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case crgfx::AddressMode::Mirror:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case crgfx::AddressMode::MirrorOnce:
			return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
	}

	return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
}

VkFilter crvk::GetVkFilter(crgfx::Filter filter)
{
	switch (filter)
	{
		case crgfx::Filter::Point:
			return VK_FILTER_NEAREST;
		case crgfx::Filter::Linear:
			return VK_FILTER_LINEAR;
	}

	return VK_FILTER_MAX_ENUM;
}

VkSamplerMipmapMode crvk::GetVkMipmapMode(crgfx::Filter filter)
{
	switch (filter)
	{
		case crgfx::Filter::Point:
			return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case crgfx::Filter::Linear:
			return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}

	return VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
}

VkBorderColor crvk::GetVkBorderColor(crgfx::BorderColor borderColor)
{
	switch (borderColor)
	{
		case crgfx::BorderColor::OpaqueBlack:
			return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		case crgfx::BorderColor::TransparentBlack:
			return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		case crgfx::BorderColor::OpaqueWhite:
			return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	}

	return VK_BORDER_COLOR_MAX_ENUM;
}

VkBlendOp crvk::GetVkBlendOp(crgfx::BlendOp blendOp)
{
	switch (blendOp)
	{
		case crgfx::BlendOp::Add:
			return VK_BLEND_OP_ADD;
		case crgfx::BlendOp::Subtract:
			return VK_BLEND_OP_SUBTRACT;
		case crgfx::BlendOp::ReverseSubtract:
			return VK_BLEND_OP_REVERSE_SUBTRACT;
		case crgfx::BlendOp::Min:
			return VK_BLEND_OP_MIN;
		case crgfx::BlendOp::Max:
			return VK_BLEND_OP_MAX;
	}
	return VK_BLEND_OP_MAX_ENUM;
}

VkBlendFactor crvk::GetVkBlendFactor(crgfx::BlendFactor blendFactor)
{
	switch (blendFactor)
	{
		case crgfx::BlendFactor::Zero:
			return VK_BLEND_FACTOR_ZERO;
		case crgfx::BlendFactor::One:
			return VK_BLEND_FACTOR_ONE;
		case crgfx::BlendFactor::SrcColor:
			return VK_BLEND_FACTOR_SRC_COLOR;
		case crgfx::BlendFactor::OneMinusSrcColor:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case crgfx::BlendFactor::DstColor:
			return VK_BLEND_FACTOR_DST_COLOR;
		case crgfx::BlendFactor::OneMinusDstColor:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case crgfx::BlendFactor::SrcAlpha:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case crgfx::BlendFactor::OneMinusSrcAlpha:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case crgfx::BlendFactor::DstAlpha:
			return VK_BLEND_FACTOR_DST_ALPHA;
		case crgfx::BlendFactor::OneMinusDstAlpha:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case crgfx::BlendFactor::Constant:
			return VK_BLEND_FACTOR_CONSTANT_COLOR;
		case crgfx::BlendFactor::OneMinusConstant:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		case crgfx::BlendFactor::SrcAlphaSaturate:
			return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		case crgfx::BlendFactor::Src1Color:
			return VK_BLEND_FACTOR_SRC1_COLOR;
		case crgfx::BlendFactor::OneMinusSrc1Color:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		case crgfx::BlendFactor::Src1Alpha:
			return VK_BLEND_FACTOR_SRC1_ALPHA;
		case crgfx::BlendFactor::OneMinusSrc1Alpha:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
	}
	return VK_BLEND_FACTOR_MAX_ENUM;
}

VkCompareOp crvk::GetVkCompareOp(crgfx::CompareOp compareOp)
{
	switch (compareOp)
	{
		case crgfx::CompareOp::Never:
			return VK_COMPARE_OP_NEVER;
		case crgfx::CompareOp::Less:
			return VK_COMPARE_OP_LESS;
		case crgfx::CompareOp::Equal:
			return VK_COMPARE_OP_EQUAL;
		case crgfx::CompareOp::LessOrEqual:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case crgfx::CompareOp::Greater:
			return VK_COMPARE_OP_GREATER;
		case crgfx::CompareOp::NotEqual:
			return VK_COMPARE_OP_NOT_EQUAL;
		case crgfx::CompareOp::GreaterOrEqual:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case crgfx::CompareOp::Always:
			return VK_COMPARE_OP_ALWAYS;
	}
	return VK_COMPARE_OP_MAX_ENUM;
}

VkStencilOp crvk::GetVkStencilOp(crgfx::StencilOp stencilOp)
{
	switch (stencilOp)
	{
		case crgfx::StencilOp::Keep:
			return VK_STENCIL_OP_KEEP;
		case crgfx::StencilOp::Zero:
			return VK_STENCIL_OP_ZERO;
		case crgfx::StencilOp::Replace:
			return VK_STENCIL_OP_REPLACE;
		case crgfx::StencilOp::IncrementSaturate:
			return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		case crgfx::StencilOp::DecrementSaturate:
			return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		case crgfx::StencilOp::Invert:
			return VK_STENCIL_OP_INVERT;
		case crgfx::StencilOp::IncrementAndWrap:
			return VK_STENCIL_OP_INCREMENT_AND_WRAP;
		case crgfx::StencilOp::DecrementAndWrap:
			return VK_STENCIL_OP_DECREMENT_AND_WRAP;
	}
	return VK_STENCIL_OP_MAX_ENUM;
}

VkSampleCountFlagBits crvk::GetVkSampleCount(crgfx::SampleCount sampleCount)
{
	switch (sampleCount)
	{
		case crgfx::SampleCount::S1:
			return VK_SAMPLE_COUNT_1_BIT;
		case crgfx::SampleCount::S2:
			return VK_SAMPLE_COUNT_2_BIT;
		case crgfx::SampleCount::S4:
			return VK_SAMPLE_COUNT_4_BIT;
		case crgfx::SampleCount::S8:
			return VK_SAMPLE_COUNT_8_BIT;
		case crgfx::SampleCount::S16:
			return VK_SAMPLE_COUNT_16_BIT;
		case crgfx::SampleCount::S32:
			return VK_SAMPLE_COUNT_32_BIT;
		case crgfx::SampleCount::S64:
			return VK_SAMPLE_COUNT_64_BIT;
	}
	return VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
}

VkShaderStageFlagBits crvk::GetVkShaderStage(crgfx::ShaderStage::T shaderStage)
{
	switch (shaderStage)
	{
		case crgfx::ShaderStage::Vertex:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case crgfx::ShaderStage::Geometry:
			return VK_SHADER_STAGE_GEOMETRY_BIT;
		case crgfx::ShaderStage::Hull:
			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case crgfx::ShaderStage::Domain:
			return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case crgfx::ShaderStage::Pixel:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case crgfx::ShaderStage::Compute:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		default:
			return VK_SHADER_STAGE_ALL;
	}
}

VkPrimitiveTopology crvk::GetVkPrimitiveTopology(crgfx::PrimitiveTopology primitiveTopology)
{
	switch (primitiveTopology)
	{
		case crgfx::PrimitiveTopology::PointList:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case crgfx::PrimitiveTopology::LineList:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case crgfx::PrimitiveTopology::LineStrip:
			return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case crgfx::PrimitiveTopology::TriangleList:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case crgfx::PrimitiveTopology::TriangleStrip:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		case crgfx::PrimitiveTopology::LineListAdjacency:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
		case crgfx::PrimitiveTopology::LineStripAdjacency:
			return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
		case crgfx::PrimitiveTopology::TriangleListAdjacency:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
		case crgfx::PrimitiveTopology::TriangleStripAdjacency:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
		case crgfx::PrimitiveTopology::PatchList:
			return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
	}

	return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}

VkVertexInputRate crvk::GetVkVertexInputRate(crgfx::VertexInputRate vertexInputRate)
{
	switch (vertexInputRate)
	{
		case crgfx::VertexInputRate::Vertex: return VK_VERTEX_INPUT_RATE_VERTEX;
		case crgfx::VertexInputRate::Instance: return VK_VERTEX_INPUT_RATE_INSTANCE;
	}

	return VK_VERTEX_INPUT_RATE_MAX_ENUM;
}

VkPolygonMode crvk::GetVkPolygonFillMode(crgfx::PolygonFillMode fillMode)
{
	switch (fillMode)
	{
		case crgfx::PolygonFillMode::Fill:
			return VK_POLYGON_MODE_FILL;
		case crgfx::PolygonFillMode::Line:
			return VK_POLYGON_MODE_LINE;
		default:
			return VK_POLYGON_MODE_MAX_ENUM;
	}
}

VkCullModeFlags crvk::GetVkPolygonCullMode(crgfx::PolygonCullMode cullMode)
{
	switch (cullMode)
	{
		case crgfx::PolygonCullMode::Back:
			return VK_CULL_MODE_BACK_BIT;
		case crgfx::PolygonCullMode::Front:
			return VK_CULL_MODE_FRONT_BIT;
		case crgfx::PolygonCullMode::None:
			return VK_CULL_MODE_NONE;
		default:
			return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
	}
}

VkFrontFace crvk::GetVkFrontFace(crgfx::FrontFace frontFace)
{
	switch (frontFace)
	{
		case crgfx::FrontFace::Clockwise:
			return VK_FRONT_FACE_CLOCKWISE;
		case crgfx::FrontFace::CounterClockwise:
			return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		default:
			return VK_FRONT_FACE_MAX_ENUM;
	}
}

VkDescriptorType crvk::GetVkDescriptorType(crgfx::ShaderResourceType::T resourceType)
{
	switch (resourceType)
	{
		case crgfx::ShaderResourceType::ConstantBuffer:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC; // TODO Fix this, we need to be able to tell whether dynamic or not
		case crgfx::ShaderResourceType::Texture:
			return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case crgfx::ShaderResourceType::RWTexture:
			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case crgfx::ShaderResourceType::Sampler:
			return VK_DESCRIPTOR_TYPE_SAMPLER;
		case crgfx::ShaderResourceType::StorageBuffer:
		case crgfx::ShaderResourceType::RWStorageBuffer:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case crgfx::ShaderResourceType::TypedBuffer:
			return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		case crgfx::ShaderResourceType::RWTypedBuffer:
			return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		default:
			return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}

	//VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER = 1,
	//VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER = 6,
	//VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC = 9,
	//VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT = 10,
}

VkAttachmentLoadOp crvk::GetVkAttachmentLoadOp(crgfx::RenderTargetLoadOp loadOp)
{
	switch (loadOp)
	{
		case crgfx::RenderTargetLoadOp::Clear:
			return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case crgfx::RenderTargetLoadOp::DontCare:
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		case crgfx::RenderTargetLoadOp::Load:
			return VK_ATTACHMENT_LOAD_OP_LOAD;
		default:
			return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
	}
}

VkAttachmentStoreOp crvk::GetVkAttachmentStoreOp(crgfx::RenderTargetStoreOp storeOp)
{
	switch (storeOp)
	{
		case crgfx::RenderTargetStoreOp::DontCare:
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		case crgfx::RenderTargetStoreOp::Store:
			return VK_ATTACHMENT_STORE_OP_STORE;
		default:
			return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
	}
}

VkPipelineStageFlags crvk::GetVkPipelineStageFlagsFromShaderStages(crgfx::ShaderStageFlags::T shaderStages)
{
	VkPipelineStageFlags pipelineFlags = 0;

	if (shaderStages & crgfx::ShaderStageFlags::Vertex)
	{
		pipelineFlags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	}

	if (shaderStages & crgfx::ShaderStageFlags::Pixel)
	{
		pipelineFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	if (shaderStages & crgfx::ShaderStageFlags::Hull)
	{
		pipelineFlags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
	}

	if (shaderStages & crgfx::ShaderStageFlags::Domain)
	{
		pipelineFlags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
	}

	if (shaderStages & crgfx::ShaderStageFlags::Geometry)
	{
		pipelineFlags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	}

	if (shaderStages & crgfx::ShaderStageFlags::Compute)
	{
		pipelineFlags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	}

	return pipelineFlags;
}

VkImageAspectFlags crvk::GetVkImageAspectFlags(crgfx::DataFormat::T textureFormat)
{
	VkImageAspectFlags aspectFlags = 0;

	if (crgfx::IsDepthStencilFormat(textureFormat))
	{
		aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else if (crgfx::IsDepthOnlyFormat(textureFormat))
	{
		aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else
	{
		aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	CrAssertMsg(aspectFlags != 0, "Aspect flags cannot be 0");

	return aspectFlags;
}

crvk::VkImageTransitionInfo crvk::GetVkImageStateInfo(crgfx::DataFormat::T textureFormat, crgfx::TextureLayout::T textureLayout)
{
	bool depthOnlyFormat = crgfx::IsDepthOnlyFormat(textureFormat);

	switch (textureLayout)
	{
		case crgfx::TextureLayout::Undefined:       return { VK_IMAGE_LAYOUT_UNDEFINED,                VK_ACCESS_NONE_KHR };
		case crgfx::TextureLayout::ShaderInput:     return { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT };
		case crgfx::TextureLayout::RenderTarget:    return { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT };
		case crgfx::TextureLayout::RWTexture:       return { VK_IMAGE_LAYOUT_GENERAL,                  VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT };
		case crgfx::TextureLayout::Present:         return { VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,          VK_ACCESS_NONE_KHR };
		case crgfx::TextureLayout::CopySource:      return { VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,     VK_ACCESS_TRANSFER_READ_BIT };
		case crgfx::TextureLayout::CopyDestination: return { VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,     VK_ACCESS_TRANSFER_WRITE_BIT };
		case crgfx::TextureLayout::DepthStencilReadWrite:
		{
			return { depthOnlyFormat ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT };
		}
		case crgfx::TextureLayout::DepthStencilWrite:
		{
			return { depthOnlyFormat ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT };
		}
		case crgfx::TextureLayout::StencilWriteDepthReadOnly:
		{
			if (depthOnlyFormat)
			{
				return { VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT };
			}
			else
			{
				return { VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT };
			}
		}
		case crgfx::TextureLayout::DepthWriteStencilReadOnly:
		{
			if (depthOnlyFormat)
			{
				return { VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT };
			}
			else
			{
				return { VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT };
			}
		}
		case crgfx::TextureLayout::DepthStencilReadOnly:
		{
			if (depthOnlyFormat)
			{
				return { VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT };
			}
			else
			{
				return { VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT };
			}
		}
		case crgfx::TextureLayout::DepthStencilReadOnlyShader:
		{
			if (depthOnlyFormat)
			{
				return { VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT };
			}
			else
			{
				return { VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT };
			}
		}
		default:
			CrAssertMsg(false, "Unhandled layout");
			return { VK_IMAGE_LAYOUT_MAX_ENUM, VK_ACCESS_NONE };
	}
}

VkPipelineStageFlags crvk::GetVkPipelineStageFlags(const crgfx::TextureState& textureState)
{
	VkPipelineStageFlags pipelineFlags = 0;

	switch (textureState.layout)
	{
		case crgfx::TextureLayout::RenderTarget:
		{
			pipelineFlags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		}
		case crgfx::TextureLayout::DepthStencilReadWrite:
		case crgfx::TextureLayout::DepthStencilWrite:
		case crgfx::TextureLayout::StencilWriteDepthReadOnly:
		case crgfx::TextureLayout::DepthWriteStencilReadOnly:
		case crgfx::TextureLayout::DepthStencilReadOnly:
		{
			pipelineFlags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			break;
		}
		case crgfx::TextureLayout::ShaderInput:
		case crgfx::TextureLayout::RWTexture:
		{
			pipelineFlags |= crvk::GetVkPipelineStageFlagsFromShaderStages(textureState.stages);
			break;
		}
		case crgfx::TextureLayout::Present:
		case crgfx::TextureLayout::Undefined:
			break;
		default:
			CrAssertMsg(false, "Unhandled case");
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
