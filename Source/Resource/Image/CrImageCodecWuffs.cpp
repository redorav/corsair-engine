#include "Resource/CrResource_pch.h"

#include "CrImageCodecWuffs.h"

#include "Core/FileSystem/ICrFile.h"

#include "Rendering/CrImage.h"
#include "Rendering/CrRendering.h"
#include "Rendering/CrDataFormats.h"

#pragma warning(push, 0)
#include <wuffs-unsupported-snapshot.c>
#pragma warning(pop)

cr3d::DataFormat::T WuffFormatToDataFormat(uint32_t wuffsFormat)
{
	switch (wuffsFormat)
	{
		case WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL:
			return cr3d::DataFormat::BGRA8_Unorm;
		default:
			CrAssertMsg(false, "Unhandled format");
			return cr3d::DataFormat::Invalid;
	}
}

CrImageHandle CrImageDecoderWuffs::Decode(const CrFileHandle& file) const
{
	// Read file into memory
	CrVector<unsigned char> fileData;
	fileData.resize_uninitialized(file->GetSize());
	file->Read(fileData.data(), fileData.size());

	CrImageHandle image = Decode(&fileData[0], file->GetSize());
	return image;
}

CrImageHandle CrImageDecoderWuffs::Decode(void* data, uint64_t dataSize) const
{
	// Set up source buffer
	wuffs_base__io_buffer wpngIOBuffer = wuffs_base__ptr_u8__reader((uint8_t*)data, dataSize, true);

	// Reserve memory for the decoder when we figure out which fourcc we have
	CrVector<uint8_t> wuffsDecoderMemory;
	int wuffsFourcc = wuffs_base__magic_number_guess_fourcc(wpngIOBuffer.reader_slice(), wpngIOBuffer.meta.closed); (void)wuffsFourcc;

	wuffs_base__image_decoder* wuffsImageDecoder = nullptr;
	wuffs_base__status initializeStatus;

	// Get the size of the decoder from the API. Using sizeof is incorrect if we link wuffs as a static library
	switch (wuffsFourcc)
	{
		case WUFFS_BASE__FOURCC__BMP:
		{
			wuffsDecoderMemory.resize(sizeof__wuffs_bmp__decoder());
			wuffs_bmp__decoder* wbmpImageDecoder = (wuffs_bmp__decoder*)wuffsDecoderMemory.data();
			initializeStatus = wuffs_bmp__decoder__initialize(wbmpImageDecoder, wuffsDecoderMemory.size(), WUFFS_VERSION, WUFFS_INITIALIZE__ALREADY_ZEROED);
			wuffsImageDecoder = (wuffs_base__image_decoder*)wbmpImageDecoder;
			break;
		}
		case WUFFS_BASE__FOURCC__GIF:
		{
			wuffsDecoderMemory.resize(sizeof__wuffs_gif__decoder());
			wuffs_gif__decoder* wgifImageDecoder = (wuffs_gif__decoder*)wuffsDecoderMemory.data();
			initializeStatus = wuffs_gif__decoder__initialize(wgifImageDecoder, wuffsDecoderMemory.size(), WUFFS_VERSION, WUFFS_INITIALIZE__ALREADY_ZEROED);
			wuffsImageDecoder = (wuffs_base__image_decoder*)wgifImageDecoder;
			break;
		}
		case WUFFS_BASE__FOURCC__NIE:
		{
			wuffsDecoderMemory.resize(sizeof__wuffs_nie__decoder());
			wuffs_nie__decoder* wnieImageDecoder = (wuffs_nie__decoder*)wuffsDecoderMemory.data();
			initializeStatus = wuffs_nie__decoder__initialize(wnieImageDecoder, wuffsDecoderMemory.size(), WUFFS_VERSION, WUFFS_INITIALIZE__ALREADY_ZEROED);
			wuffsImageDecoder = (wuffs_base__image_decoder*)wnieImageDecoder;
			break;
		}
		case WUFFS_BASE__FOURCC__PNG:
		{
			wuffsDecoderMemory.resize(sizeof__wuffs_png__decoder());
			wuffs_png__decoder* wpngImageDecoder = (wuffs_png__decoder*)wuffsDecoderMemory.data();
			initializeStatus = wuffs_png__decoder__initialize(wpngImageDecoder, wuffsDecoderMemory.size(), WUFFS_VERSION, WUFFS_INITIALIZE__ALREADY_ZEROED);
			wuffsImageDecoder = (wuffs_base__image_decoder*)wpngImageDecoder;
			break;
		}
		case WUFFS_BASE__FOURCC__TGA:
		{
			wuffsDecoderMemory.resize(sizeof__wuffs_tga__decoder());
			wuffs_tga__decoder* wtgaImageDecoder = (wuffs_tga__decoder*)wuffsDecoderMemory.data();
			initializeStatus = wuffs_tga__decoder__initialize(wtgaImageDecoder, wuffsDecoderMemory.size(), WUFFS_VERSION, WUFFS_INITIALIZE__ALREADY_ZEROED);
			wuffsImageDecoder = (wuffs_base__image_decoder*)wtgaImageDecoder;
			break;
		}
		case WUFFS_BASE__FOURCC__WBMP:
		{
			wuffsDecoderMemory.resize(sizeof__wuffs_wbmp__decoder());
			wuffs_wbmp__decoder* wwbmpImageDecoder = (wuffs_wbmp__decoder*)wuffsDecoderMemory.data();
			initializeStatus = wuffs_wbmp__decoder__initialize(wwbmpImageDecoder, wuffsDecoderMemory.size(), WUFFS_VERSION, WUFFS_INITIALIZE__ALREADY_ZEROED);
			wuffsImageDecoder = (wuffs_base__image_decoder*)wwbmpImageDecoder;
			break;
		}
		case WUFFS_BASE__FOURCC__JPEG:
		{
			wuffsDecoderMemory.resize(sizeof__wuffs_jpeg__decoder());
			wuffs_jpeg__decoder* wjpegImageDecoder = (wuffs_jpeg__decoder*)wuffsDecoderMemory.data();
			initializeStatus = wuffs_jpeg__decoder__initialize(wjpegImageDecoder, wuffsDecoderMemory.size(), WUFFS_VERSION, WUFFS_INITIALIZE__ALREADY_ZEROED);
			wuffsImageDecoder = (wuffs_base__image_decoder*)wjpegImageDecoder;
			break;
		}
	}

	if (!wuffsFourcc || !wuffs_base__status__is_ok(&initializeStatus))
	{
		CrAssertMsg(false, "Failed to initialize decoder");
		return nullptr;
	}

	// Decode the source image's configuration
	wuffs_base__image_config wpngSourceImageConfig {};
	wuffs_base__status imageConfigStatus = wuffsImageDecoder->decode_image_config(&wpngSourceImageConfig, &wpngIOBuffer);

	if (!wuffs_base__status__is_ok(&imageConfigStatus))
	{
		CrAssertMsg(false, "Failed to decode image config");
	}

	// Create an image config for the destination. This is something Wuff can handle for some cases, we may need to consider
	// an intermediate representation if we need more transformations
	wuffs_base__image_config wpngDestinationImageConfig = wpngSourceImageConfig;
	wpngDestinationImageConfig.pixcfg.set
	(
		WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL,
		WUFFS_BASE__PIXEL_FORMAT__BGRA_NONPREMUL,
		wpngSourceImageConfig.pixcfg.width(),
		wpngSourceImageConfig.pixcfg.height()
	);

	// Allocate memory for the destination texture
	CrImageHandle image(new CrImage());
	image->m_data.resize_uninitialized(wpngDestinationImageConfig.pixcfg.pixbuf_len());

	wuffs_base__slice_u8 wpngDestinationBuffer { image->m_data.data(), image->m_data.size() };
	wuffs_base__pixel_buffer wpngPixelBuffer {};
	wpngPixelBuffer.set_from_slice(&wpngDestinationImageConfig.pixcfg, wpngDestinationBuffer);

	wuffs_base__pixel_blend wpngPixelBlend = WUFFS_BASE__PIXEL_BLEND__SRC;
	wuffs_base__decode_frame_options wpngFrameOptions {};

	// Create temporary buffer depending on what the decoder needs
	CrVector<uint8_t> workBuffer; workBuffer.resize_uninitialized(wuffsImageDecoder->workbuf_len().max_incl);
	wuffs_base__slice_u8 wpngWorkBuffer { (uint8_t*)workBuffer.data(), wuffsImageDecoder->workbuf_len().max_incl};

	// Decode a frame. TODO Support multiple frames for gif
	wuffs_base__status decodeStatus = wuffsImageDecoder->decode_frame
	(
		&wpngPixelBuffer,
		&wpngIOBuffer,
		wpngPixelBlend,
		wpngWorkBuffer,
		&wpngFrameOptions
	);

	if (!wuffs_base__status__is_ok(&decodeStatus))
	{
		CrAssertMsg(false, "Failed to decode");
	}

	image->m_width = wpngDestinationImageConfig.pixcfg.width();
	image->m_height = wpngDestinationImageConfig.pixcfg.height();
	image->m_mipmapCount = 1;
	image->m_format = WuffFormatToDataFormat(wpngDestinationImageConfig.pixcfg.pixel_format().repr);
	image->m_type = cr3d::TextureType::Tex2D;

	return image;
}