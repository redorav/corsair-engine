#include "Resource/CrResource_pch.h"

#include "CrImageCodecSTB.h"

#include "Rendering/CrImage.h"
#include "Rendering/CrRendering.h"
#include "Rendering/CrDataFormats.h"

#pragma warning(push, 0)
#define STBI_NO_STDIO
#include <stb_image.h>

#define STBI_WRITE_NO_STDIO
#include <stb_image_write.h>
#pragma warning(pop)

CrImageHandle CrImageDecoderSTB::Decode(crstl::file& file) const
{
	// Read file into memory
	crstl::vector<unsigned char> fileData;
	fileData.resize_uninitialized(file.get_size());
	file.read(fileData.data(), fileData.size());

	CrImageHandle image = Decode(&fileData[0], file.get_size());
	return image;
}

CrImageHandle CrImageDecoderSTB::Decode(void* data, uint64_t dataSize) const
{
	int comp, w, h;
	unsigned char* dataPointer = stbi_load_from_memory((unsigned char*)data, (int)dataSize, &w, &h, &comp, STBI_rgb_alpha);

	if (dataPointer)
	{
		CrImageHandle image(new CrImage());

		uint32_t imageDataSize = w * h * STBI_rgb_alpha;

		// Copy stb data into image
		image->m_data.resize_uninitialized(imageDataSize);
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

static void WriteToFileSTB(void* context, void* imageData, int dataSize)
{
	CrWriteFileStream* fileStream = (CrWriteFileStream*)context;
	fileStream->Write(imageData, dataSize);
}

struct stb_memcpy
{
	void* data;
	uint64_t dataSize;
};

static void WriteToMemorySTB(void* context, void* imageData, int dataSize)
{
	stb_memcpy* memcpy_context = (stb_memcpy*)context;
	CrAssertMsg(memcpy_context->dataSize == (uint64_t)dataSize, "Sizes not equal");
	memcpy(memcpy_context, imageData, dataSize);
}

void CrImageEncoderSTB::Encode(const CrImageHandle& image, CrWriteFileStream& fileStream) const
{
	int channelCount = cr3d::DataFormats[image->GetFormat()].numComponents;

	int width = image->GetWidth();
	int height = image->GetHeight();
	int strideBytes = channelCount * width;

	if (m_containerFormat == CrImageContainerFormat::PNG)
	{
		/*int result = */stbi_write_png_to_func(WriteToFileSTB, (void*)&fileStream, width, height, channelCount, (const unsigned char*)image->GetData(), strideBytes);
	}
	else if (m_containerFormat == CrImageContainerFormat::JPG)
	{
		int quality = 100;
		/*int result = */stbi_write_jpg_to_func(WriteToFileSTB, (void*)&fileStream, width, height, channelCount, (const unsigned char*)image->GetData(), quality);
	}
	else if (m_containerFormat == CrImageContainerFormat::TGA)
	{
		/*int result = */stbi_write_tga_to_func(WriteToFileSTB, (void*)&fileStream, width, height, channelCount, (const unsigned char*)image->GetData());
	}
	else if (m_containerFormat == CrImageContainerFormat::BMP)
	{
		/*int result = */stbi_write_bmp_to_func(WriteToFileSTB, (void*)&fileStream, width, height, channelCount, (const unsigned char*)image->GetData());
	}
	else if (m_containerFormat == CrImageContainerFormat::HDR)
	{
		/*int result = */stbi_write_hdr_to_func(WriteToFileSTB, (void*)&fileStream, width, height, channelCount, (const float*)image->GetData());
	}
}

void CrImageEncoderSTB::Encode(const CrImageHandle& image, void* data, uint64_t dataSize) const
{
	int channelCount = cr3d::DataFormats[image->GetFormat()].numComponents;

	int width = image->GetWidth();
	int height = image->GetHeight();
	int strideBytes = channelCount * width;

	stb_memcpy context;
	context.data = data;
	context.dataSize = dataSize;

	/*int result = */stbi_write_png_to_func(WriteToMemorySTB, &context, width, height, channelCount, (const unsigned char*)image->GetData(), strideBytes);
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
