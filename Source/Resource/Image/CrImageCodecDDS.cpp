#include "Resource/CrResource_pch.h"

#include "CrImageCodecDDS.h"

#include "Core/Streams/CrMemoryStream.h"
#include "Core/Streams/CrFileStream.h"
#include "Core/CrMacros.h"

#include "Graphics/CrImage.h"
#include "Graphics/CrRendering.h"
#include "Graphics/CrDataFormats.h"

#include "ddspp.h"

static crgfx::DataFormat::T DXGItoDataFormat(ddspp::DXGIFormat format)
{
	switch (format)
	{
		case ddspp::BC1_UNORM: 
			return crgfx::DataFormat::BC1_RGBA_Unorm;
		case ddspp::BC3_UNORM: 
			return crgfx::DataFormat::BC3_Unorm;
		case ddspp::BC4_UNORM: 
			return crgfx::DataFormat::BC4_Unorm;
		case ddspp::BC5_UNORM:
			return crgfx::DataFormat::BC5_Unorm;
		case ddspp::BC7_UNORM: 
			return crgfx::DataFormat::BC7_Unorm;
		default:
			return crgfx::DataFormat::RGBA8_Unorm;
	}
}

static ddspp::DXGIFormat DataFormatToDdspp(crgfx::DataFormat::T format)
{
	switch (format)
	{
		// 8-bit formats
		case crgfx::DataFormat::R8_Unorm:          return ddspp::R8_UNORM;
		case crgfx::DataFormat::R8_Snorm:          return ddspp::R8_SNORM;
		case crgfx::DataFormat::R8_Uint:           return ddspp::R8_UINT;
		case crgfx::DataFormat::R8_Sint:           return ddspp::R8_SINT;

		case crgfx::DataFormat::RG8_Unorm:         return ddspp::R8G8_UNORM;
		case crgfx::DataFormat::RG8_Snorm:         return ddspp::R8G8_SNORM;
		case crgfx::DataFormat::RG8_Uint:          return ddspp::R8G8_UINT;
		case crgfx::DataFormat::RG8_Sint:          return ddspp::R8G8_SINT;

		case crgfx::DataFormat::RGBA8_Unorm:       return ddspp::R8G8B8A8_UNORM;
		case crgfx::DataFormat::RGBA8_Snorm:       return ddspp::R8G8B8A8_SNORM;
		case crgfx::DataFormat::RGBA8_Uint:        return ddspp::R8G8B8A8_UINT;
		case crgfx::DataFormat::RGBA8_Sint:        return ddspp::R8G8B8A8_SINT;
		case crgfx::DataFormat::RGBA8_SRGB:        return ddspp::R8G8B8A8_UNORM_SRGB;

		case crgfx::DataFormat::BGRA8_Unorm:       return ddspp::B8G8R8A8_UNORM;
		case crgfx::DataFormat::BGRA8_SRGB:        return ddspp::B8G8R8A8_UNORM_SRGB;

			// 16-bit integer formats
		case crgfx::DataFormat::R16_Unorm:         return ddspp::R16_UNORM;
		case crgfx::DataFormat::R16_Snorm:         return ddspp::R16_SNORM;
		case crgfx::DataFormat::R16_Uint:          return ddspp::R16_UINT;
		case crgfx::DataFormat::R16_Sint:          return ddspp::R16_SINT;

		case crgfx::DataFormat::RG16_Unorm:        return ddspp::R16G16_UNORM;
		case crgfx::DataFormat::RG16_Snorm:        return ddspp::R16G16_SNORM;
		case crgfx::DataFormat::RG16_Uint:         return ddspp::R16G16_UINT;
		case crgfx::DataFormat::RG16_Sint:         return ddspp::R16G16_SINT;

		case crgfx::DataFormat::RGBA16_Unorm:      return ddspp::R16G16B16A16_UNORM;
		case crgfx::DataFormat::RGBA16_Snorm:      return ddspp::R16G16B16A16_SNORM;
		case crgfx::DataFormat::RGBA16_Uint:       return ddspp::R16G16B16A16_UINT;
		case crgfx::DataFormat::RGBA16_Sint:       return ddspp::R16G16B16A16_SINT;

			// 16-bit float formats
		case crgfx::DataFormat::R16_Float:         return ddspp::R16_FLOAT;
		case crgfx::DataFormat::RG16_Float:        return ddspp::R16G16_FLOAT;
		case crgfx::DataFormat::RGBA16_Float:      return ddspp::R16G16B16A16_FLOAT;

		case crgfx::DataFormat::R32_Uint:          return ddspp::R32_UINT;
		case crgfx::DataFormat::R32_Sint:          return ddspp::R32_SINT;
		case crgfx::DataFormat::RG32_Uint:         return ddspp::R32G32_UINT;
		case crgfx::DataFormat::RG32_Sint:         return ddspp::R32G32_SINT;
		case crgfx::DataFormat::RGB32_Uint:        return ddspp::R32G32B32_UINT;
		case crgfx::DataFormat::RGB32_Sint:        return ddspp::R32G32B32_SINT;
		case crgfx::DataFormat::RGBA32_Uint:       return ddspp::R32G32B32A32_UINT;
		case crgfx::DataFormat::RGBA32_Sint:       return ddspp::R32G32B32A32_SINT;

			// 32-bit float formats
		case crgfx::DataFormat::R32_Float:         return ddspp::R32_FLOAT;
		case crgfx::DataFormat::RG32_Float:        return ddspp::R32G32_FLOAT;
		case crgfx::DataFormat::RGB32_Float:       return ddspp::R32G32B32_FLOAT;
		case crgfx::DataFormat::RGBA32_Float:      return ddspp::R32G32B32A32_FLOAT;

			// Compressed formats
		case crgfx::DataFormat::BC1_RGB_Unorm:     return ddspp::BC1_UNORM;
		case crgfx::DataFormat::BC1_RGB_SRGB:      return ddspp::BC1_UNORM_SRGB;
		case crgfx::DataFormat::BC1_RGBA_Unorm:    return ddspp::BC1_UNORM;
		case crgfx::DataFormat::BC1_RGBA_SRGB:     return ddspp::BC1_UNORM_SRGB;

		case crgfx::DataFormat::BC2_Unorm:         return ddspp::BC2_UNORM;
		case crgfx::DataFormat::BC2_SRGB:          return ddspp::BC2_UNORM_SRGB;

		case crgfx::DataFormat::BC3_Unorm:         return ddspp::BC3_UNORM;
		case crgfx::DataFormat::BC3_SRGB:          return ddspp::BC3_UNORM_SRGB;

		case crgfx::DataFormat::BC4_Unorm:         return ddspp::BC4_UNORM;
		case crgfx::DataFormat::BC4_Snorm:         return ddspp::BC4_SNORM;

		case crgfx::DataFormat::BC5_Unorm:         return ddspp::BC5_UNORM;
		case crgfx::DataFormat::BC5_Snorm:         return ddspp::BC5_SNORM;

		case crgfx::DataFormat::BC6H_UFloat:       return ddspp::BC6H_UF16;
		case crgfx::DataFormat::BC6H_SFloat:       return ddspp::BC6H_SF16;

		case crgfx::DataFormat::BC7_Unorm:         return ddspp::BC7_UNORM;
		case crgfx::DataFormat::BC7_SRGB:          return ddspp::BC7_UNORM_SRGB;

			// Depth-stencil formats
		case crgfx::DataFormat::D16_Unorm:         return ddspp::D16_UNORM;
		case crgfx::DataFormat::D24_Unorm_S8_Uint: return ddspp::D24_UNORM_S8_UINT;
		case crgfx::DataFormat::D24_Unorm_X8:      return ddspp::R24_UNORM_X8_TYPELESS;
		case crgfx::DataFormat::D32_Float:         return ddspp::D32_FLOAT;
		case crgfx::DataFormat::D32_Float_S8_Uint: return ddspp::D32_FLOAT_S8X24_UINT;

			// Packed Formats
		case crgfx::DataFormat::RG11B10_Float:  return ddspp::R11G11B10_FLOAT;
		case crgfx::DataFormat::RGB10A2_Unorm:  return ddspp::R10G10B10A2_UNORM;
		case crgfx::DataFormat::RGB10A2_Uint:   return ddspp::R10G10B10A2_UINT;
		case crgfx::DataFormat::B5G6R5_Unorm:   return ddspp::B5G6R5_UNORM;
		case crgfx::DataFormat::B5G5R5A1_Unorm: return ddspp::B5G5R5A1_UNORM;
		case crgfx::DataFormat::BGRA4_Unorm:    return ddspp::B4G4R4A4_UNORM;
		case crgfx::DataFormat::RGB9E5_Float:   return ddspp::R9G9B9E5_SHAREDEXP;
		default:
			return ddspp::R8G8B8A8_UNORM;
	}
}

static crgfx::TextureType DdsppToTextureType(ddspp::TextureType type)
{
	switch (type)
	{
		case ddspp::Texture1D:	return crgfx::TextureType::Tex1D;
		case ddspp::Texture2D:	return crgfx::TextureType::Tex2D;
		case ddspp::Texture3D:	return crgfx::TextureType::Volume;
		case ddspp::Cubemap:	return crgfx::TextureType::Cubemap;
		default:				return crgfx::TextureType::Tex2D;
	}
}

static ddspp::TextureType TextureTypeToDdspp(crgfx::TextureType type)
{
	switch (type)
	{
		case crgfx::TextureType::Tex1D: return ddspp::Texture1D;
		case crgfx::TextureType::Tex2D: return ddspp::Texture2D;
		case crgfx::TextureType::Volume: return ddspp::Texture3D;
		case crgfx::TextureType::Cubemap: return ddspp::Cubemap;
		default: return ddspp::Texture2D;
	}
}

static bool IsImageFormatSupported(crgfx::DataFormat::T format)
{
	// DDS Supports all uncompressed formats and all BC formats
	bool supported = false;
	supported |= format >= crgfx::DataFormat::FirstUncompressed && format <= crgfx::DataFormat::LastUncompressed;
	supported |= format >= crgfx::DataFormat::FirstBC && format <= crgfx::DataFormat::LastBC;
	return supported;
}

CrImageDecoderDDS::CrImageDecoderDDS()
{
	m_containerFormat = CrImageContainerFormat::DDS;
}

CrImageHandle CrImageDecoderDDS::Decode(crstl::file& file) const
{
	// Read in the header
	unsigned char ddsHeaderData[ddspp::MAX_HEADER_SIZE];
	file.read(ddsHeaderData, ddspp::MAX_HEADER_SIZE);

	// Decode the header
	ddspp::Descriptor desc;
	ddspp::Result result = ddspp::decode_header(ddsHeaderData, desc);

	if (result == ddspp::Success)
	{
		CrImageHandle image = CrImageHandle(new CrImage());

		// Read in actual data (without the header)
		uint64_t textureDataSize = file.get_size() - desc.headerSize;
		image->m_data.resize_uninitialized(textureDataSize);
		file.seek(crstl::file_seek_origin::begin, desc.headerSize);
		file.read(image->m_data.data(), textureDataSize);

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
void CrImageEncoderDDS::EncodeInternal(const CrImageHandle& image, StreamT& stream) const
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

void CrImageEncoderDDS::Encode(const CrImageHandle& image, CrWriteFileStream& fileStream) const
{
	EncodeInternal(image, fileStream);
}

void CrImageEncoderDDS::Encode(const CrImageHandle& image, void* data, uint64_t /*dataSize*/) const
{
	CrWriteMemoryStream memoryStream((uint8_t*)data);
	EncodeInternal(image, memoryStream);
}

bool CrImageEncoderDDS::IsImageFormatSupported(crgfx::DataFormat::T format) const
{
	return ::IsImageFormatSupported(format);
}