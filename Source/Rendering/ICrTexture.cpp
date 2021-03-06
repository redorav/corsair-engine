#include "CrRendering_pch.h"
#include "ICrTexture.h"

#include "Core/CrMacros.h"

#include "Core/SmartPointers/CrSharedPtr.h"

#include "Core/Logging/ICrDebug.h"

CrTextureDescriptor::CrTextureDescriptor(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipmaps, uint32_t arraySize, cr3d::DataFormat::T format, 
	cr3d::SampleCount sampleCount, cr3d::TextureType type, cr3d::TextureUsageFlags usage, const CrString& name, void* initialData, uint32_t extraData, void* extraDataPtr) 
	: width(width)
	, height(height)
	, depth(depth)
	, numMipmaps(numMipmaps)
	, arraySize(arraySize)
	, format(format)
	, sampleCount(sampleCount)
	, type(type)
	, usage(usage)
	, name(name)
	, initialData(initialData)
	, extraData(extraData)
	, extraDataPtr(extraDataPtr)
{

}

CrTextureDescriptor::CrTextureDescriptor(uint32_t width, uint32_t height, cr3d::DataFormat::T format, cr3d::TextureUsageFlags usage, const CrString& name)
	: CrTextureDescriptor(width, height, 1, 1, 1, format, cr3d::SampleCount::S1, cr3d::TextureType::Tex2D, usage, name, nullptr, 0, nullptr)
{

}

CrTextureDescriptor::CrTextureDescriptor()
	: CrTextureDescriptor(1, 1, 1, 1, 1, cr3d::DataFormat::RGBA8_Unorm, cr3d::SampleCount::S1, cr3d::TextureType::Tex2D, 
		cr3d::TextureUsage::Default, "", nullptr, 0, nullptr)
{

}

ICrTexture::ICrTexture(ICrRenderDevice* renderDevice, const CrTextureDescriptor& descriptor) : m_usedMemory(0)
{
	m_renderDevice = renderDevice;

	m_width = descriptor.width;
	m_height = descriptor.height;
	m_depth = CrMax(descriptor.depth, 1u);
	m_numMipmaps = CrMax(descriptor.numMipmaps, 1u);
	m_type = descriptor.type;
	m_sampleCount = descriptor.sampleCount;
	m_arraySize = descriptor.arraySize;

	switch (descriptor.type)
	{
		case cr3d::TextureType::Volume:
		{
			CrAssertMsg(m_depth > 1, "Depth must be > 1");
			CrAssertMsg(m_arraySize == 1, "Cannot create arrays of volumes");
			break;
		}
		case cr3d::TextureType::Cubemap:
		{
			CrAssertMsg(m_width == m_height, "Width and height must be the same");
			CrAssertMsg(m_depth == 1, "Depth must be 1");
			break;
		}
		case cr3d::TextureType::Tex2D:
		{
			CrAssertMsg(m_depth == 1, "Depth must be 1");
			break;
		}
		case cr3d::TextureType::Tex1D:
		{
			CrAssertMsg(m_height == 1 && m_depth == 1, "Height and depth must be 1");
			break;
		}
	}

	m_format = descriptor.format;
	m_usage = descriptor.usage;
}

uint32_t ICrTexture::GetMipSliceOffset(cr3d::DataFormat::T format, uint32_t width, uint32_t height, uint32_t numMipmaps, bool isVolume, uint32_t mip, uint32_t slice)
{
	// The mip/slice arrangement is different between texture arrays and volume textures
	//
	// Arrays
	//  __________  _____  __  __________  _____  __  __________  _____  __ 
	// |          ||     ||__||          ||     ||__||          ||     ||__|
	// |          ||_____|    |          ||_____|    |          ||_____|
	// |          |           |          |           |          |
	// |__________|           |__________|           |__________|
	//
	// Volume
	//  __________  __________  __________  _____  _____  _____  __  __  __ 
	// |          ||          ||          ||     ||     ||     ||__||__||__|
	// |          ||          ||          ||_____||_____||_____|
	// |          ||          ||          |
	// |__________||__________||__________|
	//

	uint32_t blockWidth, blockHeight;
	cr3d::GetFormatBlockWidthHeight(format, blockWidth, blockHeight);
	uint32_t bitsPerPixelOrBlock = cr3d::GetFormatBitsPerPixelOrBlock(format);

	uint32_t rowPitch = width * bitsPerPixelOrBlock / (8 * blockWidth);
	uint32_t depthPitch = rowPitch * height / blockHeight;

	uint64_t offset = 0;
	uint64_t mip0Size = depthPitch * 8; // Work in bits

	if (isVolume)
	{
		for (unsigned int m = 0; m < mip; ++m)
		{
			unsigned long long mipSize = mip0Size >> 2 * m;
			offset += mipSize * numMipmaps;
		}

		unsigned long long lastMip = mip0Size >> 2 * mip;

		offset += lastMip * slice;
	}
	else
	{
		unsigned long long mipChainSize = 0;

		for (unsigned int m = 0; m < numMipmaps; ++m)
		{
			unsigned long long mipSize = mip0Size >> 2 * m; // Divide by 2 in width and height
			mipChainSize += mipSize > bitsPerPixelOrBlock ? mipSize : bitsPerPixelOrBlock;
		}

		offset += mipChainSize * slice;

		for (unsigned int m = 0; m < mip; ++m)
		{
			unsigned long long mipSize = mip0Size >> 2 * m; // Divide by 2 in width and height
			offset += mipSize > bitsPerPixelOrBlock ? mipSize : bitsPerPixelOrBlock;
		}
	}

	offset /= 8; // Back to bytes

	return (uint32_t)offset;
}

uint32_t ICrTexture::GetMipSliceOffset(uint32_t mip, uint32_t slice) const
{
	return GetMipSliceOffset(m_format, m_width, m_height, m_numMipmaps, IsVolumeTexture(), mip, slice);
}

ICrTexture::~ICrTexture()
{
	// TODO Add to a list that'll eventually delete it, to avoid destroying something currently in use
	// Do this via the shared_ptr deleter
	//DestroyPS();
}