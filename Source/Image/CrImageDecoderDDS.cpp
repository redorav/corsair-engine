#include "CrImageDecoderDDS.h"

#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/FileSystem/ICrFile.h"

#include "Rendering/CrImage.h" // TODO Move to Image folder
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

CrImageHandle CrImageDecoderDDS::Decode(const CrFileSharedHandle& file)
{
	std::vector<unsigned char*> fileData;
	uint64_t fileSize = file->GetSize();
	fileData.resize(fileSize);

	file->Read(fileData.data(), fileSize);
	
	return Decode(fileData.data(), fileSize);
}

CrImageHandle CrImageDecoderDDS::Decode(void* data, uint64_t dataSize)
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
		image->m_format = DXGItoDataFormat(desc.format);
		image->m_width = desc.width;
		image->m_height = desc.height;
		image->m_depth = desc.depth;
		image->m_numMipmaps = desc.numMips;

		return image;
	}
	else
	{
		return nullptr;
	}
}