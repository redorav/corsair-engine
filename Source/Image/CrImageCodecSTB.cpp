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
		CrImageDescriptor imageDescriptor; // TODO
		(imageDescriptor);

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

void CrImageEncoderSTB::Encode(const CrImageHandle& image, const CrFileSharedHandle& file) const
{
	int channelCount = cr3d::DataFormats[image->GetFormat()].numComponents;

	int width = image->GetWidth();
	int height = image->GetHeight();
	int strideBytes = channelCount * width;

	int len;
	unsigned char* pngData = stbi_write_png_to_mem((const unsigned char*)image->GetData(), strideBytes, width, height, channelCount, &len);

	if (pngData)
	{
		file->Write(pngData, len);
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