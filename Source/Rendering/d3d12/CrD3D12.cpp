#include "CrRendering_pch.h"
#include "CrD3D12.h"

#include "Core/Logging/ICrDebug.h"

DXGI_FORMAT crd3d::GetD3DFormat(cr3d::DataFormat::T format)
{
	switch (format)
	{
		//-------------
		// Uncompressed
		//-------------

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
			CrAssertMsg(false, "Format not found!");
			return DXGI_FORMAT_UNKNOWN;
	}
}