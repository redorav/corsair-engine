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
	// Read in the header
	unsigned char ddsHeaderData[ddspp::MAX_HEADER_SIZE];
	file->Read(ddsHeaderData, ddspp::MAX_HEADER_SIZE);

	// Decode the header
	ddspp::Descriptor desc;
	ddspp::Result result = ddspp::decode_header(ddsHeaderData, desc);

	if (result == ddspp::Success)
	{
		CrImageHandle image = CrImageHandle(new CrImage());

		// Seek to the actual data
		file->Seek(SeekOrigin::Begin, desc.headerSize);

		// Read in actual data
		uint64_t textureDataSize = file->GetSize() - desc.headerSize;
		image->m_data.resize(textureDataSize);
		file->Read(image->m_data.data(), image->m_data.size());

		image->m_format = DXGItoDataFormat(desc.format);
		image->m_width = desc.width;
		image->m_height = desc.height;
		image->m_depth = desc.depth;
		image->m_numMipmaps = desc.numMips;

		file->Rewind();

		return image;
	}
	else
	{
		return nullptr;
	}
}