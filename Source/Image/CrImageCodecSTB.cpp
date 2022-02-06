#include "CrImageCodecSTB.h"

#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/FileSystem/ICrFile.h"

#include "Rendering/CrImage.h"
#include "Rendering/CrRendering.h"

#define STBI_NO_STDIO
#pragma warning(push, 0)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#pragma warning(pop)

CrImageHandle CrImageDecoderSTB::Decode(const CrFileSharedHandle& file) const
{
	// Read file into memory
	CrVector<unsigned char> fileData;
	fileData.resize(file->GetSize());
	file->Read(fileData.data(), fileData.size());

	CrImageHandle image = Decode(&fileData[0], file->GetSize());
	return image;
}

CrImageHandle CrImageDecoderSTB::Decode(void* data, uint64_t dataSize) const
{
	int comp, w, h;
	unsigned char* dataPointer = stbi_load_from_memory((unsigned char*)data, (int)dataSize, &w, &h, &comp, STBI_rgb_alpha);

	if (dataPointer)
	{
		CrImageHandle image = CrImageHandle(new CrImage());

		uint32_t imageDataSize = w * h * STBI_rgb_alpha;

		// Copy stb data into image
		image->m_data.resize(imageDataSize);
		memcpy(image->m_data.data(), dataPointer, image->m_data.size());

		image->m_width = w;
		image->m_height = h;
		image->m_mipmapCount = 1;

		// This format does not work for textures with 3 channels, so we tell stb to create an alpha channel
		// because stb can return data with different number of channels depending on the image
		image->m_format = cr3d::DataFormat::RGBA8_Unorm;
		image->m_type = cr3d::TextureType::Tex2D;

		// Free stb data if the allocation was successful
		stbi_image_free(dataPointer);

		return image;
	}
	else
	{
		return nullptr;
	}
}

CrImageEncoderSTB::CrImageEncoderSTB(CrImageContainerFormat::T containerFormat)
{
	CrAssertMsg
	(
		containerFormat == CrImageContainerFormat::PNG ||
		containerFormat == CrImageContainerFormat::JPG ||
		containerFormat == CrImageContainerFormat::TGA ||
		containerFormat == CrImageContainerFormat::BMP ||
		containerFormat == CrImageContainerFormat::HDR,
		"Format not allowed"
	);

	m_containerFormat = containerFormat;
}

static void WriteToFileSTB(void* context, void* data, int size)
{
	ICrFile* file = (ICrFile*)context;
	file->Write(data, size);
}

void CrImageEncoderSTB::Encode(const CrImageHandle& image, const CrFileSharedHandle& file) const
{
	int channelCount = cr3d::DataFormats[image->GetFormat()].numComponents;

	int width = image->GetWidth();
	int height = image->GetHeight();
	int strideBytes = channelCount * width;
	
	int len = 0;
	unsigned char* pngData = nullptr;
	stbi__write_context stbContext;
	stbContext.func = WriteToFileSTB;
	stbContext.context = file.get();

	if (m_containerFormat == CrImageContainerFormat::PNG)
	{
		pngData = stbi_write_png_to_mem((const unsigned char*)image->GetData(), strideBytes, width, height, channelCount, &len);

		if (pngData)
		{
			file->Write(pngData, len);
		}
	}
	else if (m_containerFormat == CrImageContainerFormat::JPG)
	{
		int quality = 100;
		/*int r = */stbi_write_jpg_core(&stbContext, width, height, channelCount, image->GetData(), quality);
	}
	else if (m_containerFormat == CrImageContainerFormat::TGA)
	{
		/*int r = */stbi_write_tga_core(&stbContext, width, height, channelCount, (void*)image->GetData());
	}
	else if (m_containerFormat == CrImageContainerFormat::BMP)
	{
		/*int r = */stbi_write_bmp_core(&stbContext, width, height, channelCount, image->GetData());
	}
	else if (m_containerFormat == CrImageContainerFormat::HDR)
	{
		/*int r = */stbi_write_hdr_core(&stbContext, width, height, channelCount, (float*)image->GetData());
	}
}

void CrImageEncoderSTB::Encode(const CrImageHandle& image, void* data, uint64_t dataSize) const
{
	int channelCount = cr3d::DataFormats[image->GetFormat()].numComponents;

	int width = image->GetWidth();
	int height = image->GetHeight();
	int strideBytes = channelCount * width;

	int len;
	unsigned char* pngData = stbi_write_png_to_mem((const unsigned char*)image->GetData(), strideBytes, width, height, channelCount, &len);

	if (pngData)
	{
		memcpy(data, pngData, dataSize);
		stbi_image_free(pngData);
	}
}

bool CrImageEncoderSTB::IsImageFormatSupported(cr3d::DataFormat::T format) const
{
	cr3d::DataFormatInfo formatInfo = cr3d::DataFormats[format];

	bool supported = false;

	if (m_containerFormat == CrImageContainerFormat::PNG ||
		m_containerFormat == CrImageContainerFormat::JPG ||
		m_containerFormat == CrImageContainerFormat::TGA ||
		m_containerFormat == CrImageContainerFormat::BMP)
	{
		// Only support 8-bit formats non-HDR formats
		supported |= (formatInfo.elementSizeR == 8 && !formatInfo.hdrFloat);
	}
	else if (m_containerFormat == CrImageContainerFormat::HDR)
	{
		// Only supports 32-bit float data
		supported |= (formatInfo.hdrFloat && formatInfo.dataOrBlockSize == 4);
	}

	// STB doesn't support any form of hardware compression format
	supported &= !formatInfo.compressed;

	return supported;
}
