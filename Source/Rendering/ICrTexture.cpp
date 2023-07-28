#include "Rendering/CrRendering_pch.h"
#include "ICrTexture.h"

#include "Core/CrMacros.h"
#include "Core/Logging/ICrDebug.h"

#include "Math/CrMath.h"

CrTextureDescriptor::CrTextureDescriptor()
	: width(1)
	, height(1)
	, depth(1)
	, mipmapCount(1)
	, arraySize(1)
	, format(cr3d::DataFormat::RGBA8_Unorm)
	, sampleCount(cr3d::SampleCount::S1)
	, type(cr3d::TextureType::Tex2D)
	, usage(cr3d::TextureUsage::Default)
	, initialData(nullptr)
	, initialDataSize(0)
	, extraData(0)
	, extraDataPtr(nullptr)
	, name(nullptr)
{

}

ICrTexture::ICrTexture(ICrRenderDevice* renderDevice, const CrTextureDescriptor& descriptor) : CrGPUDeletable(renderDevice)
	, m_usedGPUMemoryBytes(0)
{
	m_width = descriptor.width;
	m_height = descriptor.height;
	m_depth = CrMax(descriptor.depth, 1u);
	m_mipmapCount = CrMax(descriptor.mipmapCount, 1u);
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

	// This is the state we expect the texture to be in by default, when not being used by
	// a command buffer or render pass. It is the state the texture decays to after copy
	// operations if not explicitly declared
	if (IsRenderTarget())
	{
		m_defaultState = { cr3d::TextureLayout::RenderTarget, cr3d::ShaderStageFlags::Pixel };
	}
	else if (IsDepthStencil())
	{
		if (cr3d::IsDepthStencilFormat(descriptor.format))
		{
			m_defaultState = { cr3d::TextureLayout::DepthStencilReadWrite, cr3d::ShaderStageFlags::Pixel };
		}
		else if (cr3d::IsDepthFormat(descriptor.format))
		{
			m_defaultState = { cr3d::TextureLayout::DepthReadWrite, cr3d::ShaderStageFlags::Pixel };
		}
		else
		{
			CrAssertMsg(false, "Format not compatible with depth stencil capabilities");
		}
	}
	else if (IsUnorderedAccess())
	{
		m_defaultState = { cr3d::TextureLayout::RWTexture, cr3d::ShaderStageFlags::Compute };
	}
	else if (IsSwapchain())
	{
		m_defaultState = { cr3d::TextureLayout::Present, cr3d::ShaderStageFlags::Present };
	}
	else
	{
		// If none of the states above, assume we want to sample this texture
		m_defaultState = { cr3d::TextureLayout::ShaderInput, cr3d::ShaderStageFlags::Pixel };
	}

	m_hardwareMipmapLayouts = {};
}

cr3d::MipmapLayout ICrTexture::GetGenericMipSliceLayout(cr3d::DataFormat::T format, uint32_t width, uint32_t height, uint32_t numMipmaps, bool isVolume, uint32_t mip, uint32_t slice)
{
	cr3d::MipmapLayout mipmapLayout;

	// The mip/slice arrangement is different between texture arrays and volume textures
	//
	// Arrays
	// 
	//      0         1    2       0         1    2       0         1    2
	//  __________  _____  __  __________  _____  __  __________  _____  __ 
	// |          ||     ||__||          ||     ||__||          ||     ||__|
	// |          ||_____|    |          ||_____|    |          ||_____|
	// |          |           |          |           |          |
	// |__________|           |__________|           |__________|
	//
	// Volume
	// 
	//      0                                 1                  2
	//  __________  __________  __________  _____  _____  _____  __  __  __ 
	// |          ||          ||          ||     ||     ||     ||__||__||__|
	// |          ||          ||          ||_____||_____||_____|
	// |          ||          ||          |
	// |__________||__________||__________|
	//

	uint32_t blockWidth, blockHeight;
	cr3d::GetFormatBlockWidthHeight(format, blockWidth, blockHeight);
	uint32_t bitsPerPixelOrBlock = cr3d::GetFormatBitsPerPixelOrBlock(format);

	uint64_t rowPitch = width * bitsPerPixelOrBlock / (8 * blockWidth);
	uint64_t heightInPixelsBlocks = height / blockHeight;
	uint64_t depthPitch = rowPitch * heightInPixelsBlocks;

	uint64_t offset = 0;
	uint64_t mip0Size = depthPitch * 8; // Work in bits

	if (isVolume)
	{
		// There are no slices in volume textures, we ignore the parameter
		for (uint32_t m = 0; m < mip; ++m)
		{
			offset += (mip0Size >> 2 * m);
		}

		offset *= numMipmaps;
	}
	else
	{
		uint64_t mipChainSize = 0;

		for (uint32_t m = 0; m < numMipmaps; ++m)
		{
			uint64_t mipSize = mip0Size >> 2 * m; // Divide by 2 in width and height
			mipChainSize += mipSize > bitsPerPixelOrBlock ? mipSize : bitsPerPixelOrBlock;
		}

		offset += mipChainSize * slice;

		for (uint32_t m = 0; m < mip; ++m)
		{
			uint64_t mipSize = mip0Size >> 2 * m; // Divide by 2 in width and height
			offset += mipSize > bitsPerPixelOrBlock ? mipSize : bitsPerPixelOrBlock;
		}
	}

	offset /= 8; // Back to bytes

	mipmapLayout.offsetBytes          = (uint32_t)offset;
	mipmapLayout.rowPitchBytes        = (uint32_t)rowPitch >> mip;
	mipmapLayout.heightInPixelsBlocks = (uint32_t)heightInPixelsBlocks >> mip;

	return mipmapLayout;
}

cr3d::MipmapLayout ICrTexture::GetGenericMipSliceLayout(uint32_t mip, uint32_t slice) const
{
	return GetGenericMipSliceLayout(m_format, m_width, m_height, m_mipmapCount, IsVolumeTexture(), mip, slice);
}

cr3d::MipmapLayout ICrTexture::GetHardwareMipSliceLayout(uint32_t mip, uint32_t slice) const
{
	cr3d::MipmapLayout mipmapLayout = m_hardwareMipmapLayouts[mip];
	mipmapLayout.offsetBytes += m_slicePitchBytes * slice;
	return mipmapLayout;
}

void ICrTexture::CopyIntoTextureMemory(uint8_t* destinationData, const uint8_t* sourceData, uint32_t mip, uint32_t slice)
{
	cr3d::MipmapLayout sourceMipLayout = GetGenericMipSliceLayout(mip, slice);
	cr3d::MipmapLayout destinationMipLayout = GetHardwareMipSliceLayout(mip, slice);

	uint32_t depth = GetDepth();

	// Mipmaps are considered to include depth. Slice in this context only refers to arrays of textures
	// If the mip size is equal, we can copy the entire mipmap in one memcpy
	if (destinationMipLayout.GetMipSize() == sourceMipLayout.GetMipSize())
	{
		memcpy
		(
			destinationData + destinationMipLayout.offsetBytes,
			sourceData + sourceMipLayout.offsetBytes,
			sourceMipLayout.GetMipSize() * depth
		);
	}
	else
	{
		// If the depth pitch is not equal, we process every depth slice
		for (uint32_t z = 0; z < depth; ++z)
		{
			if (sourceMipLayout.rowPitchBytes == destinationMipLayout.rowPitchBytes)
			{
				memcpy
				(
					destinationData + destinationMipLayout.offsetBytes + destinationMipLayout.GetMipSize() * z,
					sourceData + sourceMipLayout.offsetBytes + sourceMipLayout.GetMipSize() * z, 
					sourceMipLayout.GetMipSize()
				);
			}
			else
			{
				// If the row pitch is not equal, we copy row by row. This typically happens at the lowest mips
				for (uint32_t row = 0; row < sourceMipLayout.heightInPixelsBlocks; ++row)
				{
					memcpy
					(
						destinationData + destinationMipLayout.offsetBytes + destinationMipLayout.GetMipSize() * z + row * destinationMipLayout.rowPitchBytes,
						sourceData + sourceMipLayout.offsetBytes + sourceMipLayout.GetMipSize() * z + row * sourceMipLayout.rowPitchBytes,
						sourceMipLayout.rowPitchBytes // Always copy unpadded size as it's smaller
					);
				}
			}
		}
	}
}

void ICrTexture::CopyIntoTextureMemory
(
	uint8_t* destinationData, const uint8_t* sourceData, 
	uint32_t startMip, uint32_t mipCount, uint32_t startSlice, uint32_t sliceCount
)
{
	for (uint32_t slice = startSlice; slice < startSlice + sliceCount; ++slice)
	{
		for (uint32_t mip = startMip; mip < startMip + mipCount; ++mip)
		{
			CopyIntoTextureMemory(destinationData, sourceData, mip, slice);
		}
	}
}

ICrTexture::~ICrTexture()
{
	
}