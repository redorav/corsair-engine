#include "Resource/CrResource_pch.h"

#include "CrImageCodecDDS.h"

#include "Core/FileSystem/ICrFile.h"
#include "Core/Streams/CrMemoryStream.h"
#include "Core/Streams/CrFileStream.h"
#include "Core/CrMacros.h"

#include "Rendering/CrImage.h"
#include "Rendering/CrRendering.h"
#include "Rendering/CrDataFormats.h"

#include "ddspp.h"

static cr3d::DataFormat::T DXGItoDataFormat(ddspp::DXGIFormat format)
{
	switch (format)
	{
		case ddspp::BC1_UNORM: 
			return cr3d::DataFormat::BC1_RGBA_Unorm;
		case ddspp::BC3_UNORM: 
			return cr3d::DataFormat::BC3_Unorm;
		case ddspp::BC4_UNORM: 
			return cr3d::DataFormat::BC4_Unorm;
		case ddspp::BC7_UNORM: 
			return cr3d::DataFormat::BC7_Unorm;
		default:
			return cr3d::DataFormat::RGBA8_Unorm;
	}
}

static ddspp::DXGIFormat DataFormatToDdspp(cr3d::DataFormat::T format)
{
	switch (format)
	{
		// 8-bit formats
		case cr3d::DataFormat::R8_Unorm:          return ddspp::R8_UNORM;
		case cr3d::DataFormat::R8_Snorm:          return ddspp::R8_SNORM;
		case cr3d::DataFormat::R8_Uint:           return ddspp::R8_UINT;
		case cr3d::DataFormat::R8_Sint:           return ddspp::R8_SINT;

		case cr3d::DataFormat::RG8_Unorm:         return ddspp::R8G8_UNORM;
		case cr3d::DataFormat::RG8_Snorm:         return ddspp::R8G8_SNORM;
		case cr3d::DataFormat::RG8_Uint:          return ddspp::R8G8_UINT;
		case cr3d::DataFormat::RG8_Sint:          return ddspp::R8G8_SINT;

		case cr3d::DataFormat::RGBA8_Unorm:       return ddspp::R8G8B8A8_UNORM;
		case cr3d::DataFormat::RGBA8_Snorm:       return ddspp::R8G8B8A8_SNORM;
		case cr3d::DataFormat::RGBA8_Uint:        return ddspp::R8G8B8A8_UINT;
		case cr3d::DataFormat::RGBA8_Sint:        return ddspp::R8G8B8A8_SINT;
		case cr3d::DataFormat::RGBA8_SRGB:        return ddspp::R8G8B8A8_UNORM_SRGB;

		case cr3d::DataFormat::BGRA8_Unorm:       return ddspp::B8G8R8A8_UNORM;
		case cr3d::DataFormat::BGRA8_SRGB:        return ddspp::B8G8R8A8_UNORM_SRGB;

			// 16-bit integer formats
		case cr3d::DataFormat::R16_Unorm:         return ddspp::R16_UNORM;
		case cr3d::DataFormat::R16_Snorm:         return ddspp::R16_SNORM;
		case cr3d::DataFormat::R16_Uint:          return ddspp::R16_UINT;
		case cr3d::DataFormat::R16_Sint:          return ddspp::R16_SINT;

		case cr3d::DataFormat::RG16_Unorm:        return ddspp::R16G16_UNORM;
		case cr3d::DataFormat::RG16_Snorm:        return ddspp::R16G16_SNORM;
		case cr3d::DataFormat::RG16_Uint:         return ddspp::R16G16_UINT;
		case cr3d::DataFormat::RG16_Sint:         return ddspp::R16G16_SINT;

		case cr3d::DataFormat::RGBA16_Unorm:      return ddspp::R16G16B16A16_UNORM;
		case cr3d::DataFormat::RGBA16_Snorm:      return ddspp::R16G16B16A16_SNORM;
		case cr3d::DataFormat::RGBA16_Uint:       return ddspp::R16G16B16A16_UINT;
		case cr3d::DataFormat::RGBA16_Sint:       return ddspp::R16G16B16A16_SINT;

			// 16-bit float formats
		case cr3d::DataFormat::R16_Float:         return ddspp::R16_FLOAT;
		case cr3d::DataFormat::RG16_Float:        return ddspp::R16G16_FLOAT;
		case cr3d::DataFormat::RGBA16_Float:      return ddspp::R16G16B16A16_FLOAT;

		case cr3d::DataFormat::R32_Uint:          return ddspp::R32_UINT;
		case cr3d::DataFormat::R32_Sint:          return ddspp::R32_SINT;
		case cr3d::DataFormat::RG32_Uint:         return ddspp::R32G32_UINT;
		case cr3d::DataFormat::RG32_Sint:         return ddspp::R32G32_SINT;
		case cr3d::DataFormat::RGB32_Uint:        return ddspp::R32G32B32_UINT;
		case cr3d::DataFormat::RGB32_Sint:        return ddspp::R32G32B32_SINT;
		case cr3d::DataFormat::RGBA32_Uint:       return ddspp::R32G32B32A32_UINT;
		case cr3d::DataFormat::RGBA32_Sint:       return ddspp::R32G32B32A32_SINT;

			// 32-bit float formats
		case cr3d::DataFormat::R32_Float:         return ddspp::R32_FLOAT;
		case cr3d::DataFormat::RG32_Float:        return ddspp::R32G32_FLOAT;
		case cr3d::DataFormat::RGB32_Float:       return ddspp::R32G32B32_FLOAT;
		case cr3d::DataFormat::RGBA32_Float:      return ddspp::R32G32B32A32_FLOAT;

			// Compressed formats
		case cr3d::DataFormat::BC1_RGB_Unorm:     return ddspp::BC1_UNORM;
		case cr3d::DataFormat::BC1_RGB_SRGB:      return ddspp::BC1_UNORM_SRGB;
		case cr3d::DataFormat::BC1_RGBA_Unorm:    return ddspp::BC1_UNORM;
		case cr3d::DataFormat::BC1_RGBA_SRGB:     return ddspp::BC1_UNORM_SRGB;

		case cr3d::DataFormat::BC2_Unorm:         return ddspp::BC2_UNORM;
		case cr3d::DataFormat::BC2_SRGB:          return ddspp::BC2_UNORM_SRGB;

		case cr3d::DataFormat::BC3_Unorm:         return ddspp::BC3_UNORM;
		case cr3d::DataFormat::BC3_SRGB:          return ddspp::BC3_UNORM_SRGB;

		case cr3d::DataFormat::BC4_Unorm:         return ddspp::BC4_UNORM;
		case cr3d::DataFormat::BC4_Snorm:         return ddspp::BC4_SNORM;

		case cr3d::DataFormat::BC5_Unorm:         return ddspp::BC5_UNORM;
		case cr3d::DataFormat::BC5_Snorm:         return ddspp::BC5_SNORM;

		case cr3d::DataFormat::BC6H_UFloat:       return ddspp::BC6H_UF16;
		case cr3d::DataFormat::BC6H_SFloat:       return ddspp::BC6H_SF16;

		case cr3d::DataFormat::BC7_Unorm:         return ddspp::BC7_UNORM;
		case cr3d::DataFormat::BC7_SRGB:          return ddspp::BC7_UNORM_SRGB;

			// Depth-stencil formats
		case cr3d::DataFormat::D16_Unorm:         return ddspp::D16_UNORM;
		case cr3d::DataFormat::D24_Unorm_S8_Uint: return ddspp::D24_UNORM_S8_UINT;
		case cr3d::DataFormat::D24_Unorm_X8:      return ddspp::R24_UNORM_X8_TYPELESS;
		case cr3d::DataFormat::D32_Float:         return ddspp::D32_FLOAT;
		case cr3d::DataFormat::D32_Float_S8_Uint: return ddspp::D32_FLOAT_S8X24_UINT;

			// Packed Formats
		case cr3d::DataFormat::RG11B10_Float:  return ddspp::R11G11B10_FLOAT;
		case cr3d::DataFormat::RGB10A2_Unorm:  return ddspp::R10G10B10A2_UNORM;
		case cr3d::DataFormat::RGB10A2_Uint:   return ddspp::R10G10B10A2_UINT;
		case cr3d::DataFormat::B5G6R5_Unorm:   return ddspp::B5G6R5_UNORM;
		case cr3d::DataFormat::B5G5R5A1_Unorm: return ddspp::B5G5R5A1_UNORM;
		case cr3d::DataFormat::BGRA4_Unorm:    return ddspp::B4G4R4A4_UNORM;
		case cr3d::DataFormat::RGB9E5_Float:   return ddspp::R9G9B9E5_SHAREDEXP;
		default:
			return ddspp::R8G8B8A8_UNORM;
	}
}

static cr3d::TextureType DdsppToTextureType(ddspp::TextureType type)
{
	switch (type)
	{
		case ddspp::Texture1D:	return cr3d::TextureType::Tex1D;
		case ddspp::Texture2D:	return cr3d::TextureType::Tex2D;
		case ddspp::Texture3D:	return cr3d::TextureType::Volume;
		case ddspp::Cubemap:	return cr3d::TextureType::Cubemap;
		default:				return cr3d::TextureType::Tex2D;
	}
}

static ddspp::TextureType TextureTypeToDdspp(cr3d::TextureType type)
{
	switch (type)
	{
		case cr3d::TextureType::Tex1D: return ddspp::Texture1D;
		case cr3d::TextureType::Tex2D: return ddspp::Texture2D;
		case cr3d::TextureType::Volume: return ddspp::Texture3D;
		case cr3d::TextureType::Cubemap: return ddspp::Cubemap;
		default: return ddspp::Texture2D;
	}
}

static bool IsImageFormatSupported(cr3d::DataFormat::T format)
{
	// DDS Supports all uncompressed formats and all BC formats
	bool supported = false;
	supported |= format >= cr3d::DataFormat::FirstUncompressed && format <= cr3d::DataFormat::LastUncompressed;
	supported |= format >= cr3d::DataFormat::FirstBC && format <= cr3d::DataFormat::LastBC;
	return supported;
}

CrImageDecoderDDS::CrImageDecoderDDS()
{
	m_containerFormat = CrImageContainerFormat::DDS;
}

CrImageHandle CrImageDecoderDDS::Decode(const CrFileHandle& file) const
{
	// Read in the header
	unsigned char ddsHeaderData[ddspp::MAX_HEADER_SIZE];
	file->Read(ddsHeaderData, ddspp::MAX_HEADER_SIZE);

	// Decode the header
	ddspp::Descriptor desc;
	ddspp::Result result = ddspp::decode_header(ddsHeaderData, desc);

	if (result == ddspp::Success)
	{
		CrImageHandle image = CrImageHandle(new CrImage());

		// Read in actual data (without the header)
		uint64_t textureDataSize = file->GetSize() - desc.headerSize;
		image->m_data.resize_uninitialized(textureDataSize);
		file->Seek(SeekOrigin::Begin, desc.headerSize);
		file->Read(image->m_data.data(), textureDataSize);

		SetImageProperties(image, desc);

		return image;
	}
	else
	{
		return nullptr;
	}
}

CrImageHandle CrImageDecoderDDS::Decode(void* data, uint64_t dataSize) const
{
	// Read in the header
	unsigned char ddsHeaderData[ddspp::MAX_HEADER_SIZE];
	memcpy(ddsHeaderData, data, ddspp::MAX_HEADER_SIZE);

	// Decode the header
	ddspp::Descriptor desc;
	ddspp::Result result = ddspp::decode_header(ddsHeaderData, desc);

	if (result == ddspp::Success)
	{
		CrImageHandle image = CrImageHandle(new CrImage());

		// Read in actual data (without the header)
		data = (unsigned char*)data + desc.headerSize;
		uint64_t textureDataSize = dataSize - desc.headerSize;

		image->m_data.resize_uninitialized(textureDataSize);
		memcpy(image->m_data.data(), data, textureDataSize);

		SetImageProperties(image, desc);

		return image;
	}
	else
	{
		return nullptr;
	}
}

void CrImageDecoderDDS::SetImageProperties(CrImageHandle& image, const ddspp::Descriptor& desc) const
{
	image->m_width = desc.width;
	image->m_height = desc.height;
	image->m_depth = desc.depth;
	image->m_mipmapCount = desc.numMips;
	image->m_type = DdsppToTextureType(desc.type);
	image->m_format = DXGItoDataFormat(desc.format);
}

CrImageEncoderDDS::CrImageEncoderDDS()
{
	m_containerFormat = CrImageContainerFormat::DDS;
}

template<typename StreamT>
void CrImageEncoderDDS::Encode(const CrImageHandle& image, StreamT& stream) const
{
	const ddspp::DXGIFormat format = DataFormatToDdspp(image->GetFormat());
	const ddspp::TextureType type = TextureTypeToDdspp(image->GetType());

	ddspp::Header header; ddspp::HeaderDXT10 headerDX10;
	ddspp::encode_header(format, image->GetWidth(), image->GetHeight(), image->GetDepth(), type, image->GetMipmapCount(), 1, header, headerDX10);

	stream << ddspp::DDS_MAGIC;
	stream << header;
	if (ddspp::is_dxt10(header))
	{
		stream << headerDX10;
	}

	stream.Write(image->GetData(), image->GetDataSize());
}

void CrImageEncoderDDS::Encode(const CrImageHandle& image, const CrFileHandle& file) const
{
	CrWriteFileStream fileStream(file);
	Encode(image, fileStream);
}

void CrImageEncoderDDS::Encode(const CrImageHandle& image, void* data, uint64_t /*dataSize*/) const
{
	CrWriteMemoryStream memoryStream((uint8_t*)data);
	Encode(image, memoryStream);
}

bool CrImageEncoderDDS::IsImageFormatSupported(cr3d::DataFormat::T format) const
{
	return ::IsImageFormatSupported(format);
}