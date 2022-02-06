#include "CrImageDecoderDDS.h"

#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/FileSystem/ICrFile.h"
#include "Core/Streams/CrMemoryStream.h"
#include "Core/Streams/CrFileStream.h"
#include "Core/CrMacros.h"

#include "Rendering/CrImage.h"
#include "Rendering/CrRendering.h"

#include "ddspp.h"

// TODO Complete and extract to a more useful header
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

static ddspp::DXGIFormat DataFormatToDXGI(cr3d::DataFormat::T format)
{
	switch (format)
	{
		case cr3d::DataFormat::RGBA8_Unorm:
			return ddspp::R8G8B8A8_UNORM;
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

CrImageHandle CrImageDecoderDDS::Decode(const CrFileSharedHandle& file) const
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
		image->m_data.resize(textureDataSize);
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

		image->m_data.resize(textureDataSize);
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

template<typename StreamT>
void CrImageEncoderDDS::Encode(const CrImageHandle& image, StreamT& stream) const
{
	const ddspp::DXGIFormat format = DataFormatToDXGI(image->GetFormat());
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

void CrImageEncoderDDS::Encode(const CrImageHandle& image, const CrFileSharedHandle& file) const
{
	CrWriteFileStream fileStream(file);

	const ddspp::DXGIFormat format = DataFormatToDXGI(image->GetFormat());
	const ddspp::TextureType type = TextureTypeToDdspp(image->GetType());

	ddspp::Header header; ddspp::HeaderDXT10 headerDX10;
	ddspp::encode_header(format, image->GetWidth(), image->GetHeight(), image->GetDepth(), type, image->GetMipmapCount(), 1, header, headerDX10);

	file->Write((void*)&ddspp::DDS_MAGIC, sizeof(ddspp::DDS_MAGIC));
	file->Write(&header, sizeof(header));
	if (ddspp::is_dxt10(header))
	{
		file->Write(&headerDX10, sizeof(headerDX10));
	}

	file->Write((void*)image->GetData(), image->GetDataSize());
}

void CrImageEncoderDDS::Encode(const CrImageHandle& image, void* data, uint64_t dataSize) const
{
	const ddspp::DXGIFormat format = DataFormatToDXGI(image->GetFormat());
	const ddspp::TextureType type = TextureTypeToDdspp(image->GetType());

	ddspp::Header header; ddspp::HeaderDXT10 headerDX10;
	ddspp::encode_header(format, image->GetWidth(), image->GetHeight(), image->GetDepth(), type, image->GetMipmapCount(), 1, header, headerDX10);

	CrWriteMemoryStream memoryStream((uint8_t*)data);

	memoryStream << ddspp::DDS_MAGIC;
	memoryStream << header;
	if (ddspp::is_dxt10(header))
	{
		memoryStream << headerDX10;
	}
	
	memoryStream.Write(data, dataSize);
}