#include "Graphics/CrRendering_pch.h"
#include "ICrTexture.h"

#include "Core/CrMacros.h"
#include "Core/Logging/ICrDebug.h"

#include "Math/CrMath.h"

#include "ddspp.h"

CrTextureDescriptor::CrTextureDescriptor()
	: width(1)
	, height(1)
	, depth(1)
	, mipmapCount(1)
	, arraySize(1)
	, format(crgfx::DataFormat::RGBA8_Unorm)
	, sampleCount(crgfx::SampleCount::S1)
	, type(crgfx::TextureType::Tex2D)
	, usage(crgfx::TextureUsage::Default)
	, initialData(nullptr)
	, initialDataSize(0)
	, extraData(0)
	, extraDataPtr(nullptr)
	, name(nullptr)
{

}

ICrTexture::ICrTexture(ICrRenderDevice* renderDevice, const CrTextureDescriptor& descriptor) : CrGPUAutoDeletable(renderDevice)
	, m_usedGPUMemoryBytes(0)
{
	m_width = descriptor.width;
	m_height = descriptor.height;
	m_depth = CrMax(descriptor.depth, 1u);
	m_mipmapCount = CrMax(descriptor.mipmapCount, 1u);
	m_type = descriptor.type;
	m_sampleCount = descriptor.sampleCount;
	m_arraySize = descriptor.arraySize;

#if !defined(CR_CONFIG_FINAL)
	if (descriptor.name)
	{
		m_debugName = descriptor.name;
	}
#endif
	
	switch (descriptor.type)
	{
		case crgfx::TextureType::Volume:
		{
			CrAssertMsg(m_depth > 1, "Depth must be > 1");
			CrAssertMsg(m_arraySize == 1, "Cannot create arrays of volumes");
			break;
		}
		case crgfx::TextureType::Cubemap:
		{
			CrAssertMsg(m_width == m_height, "Width and height must be the same");
			CrAssertMsg(m_depth == 1, "Depth must be 1");
			break;
		}
		case crgfx::TextureType::Tex2D:
		{
			CrAssertMsg(m_depth == 1, "Depth must be 1");
			break;
		}
		case crgfx::TextureType::Tex1D:
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
		m_defaultState = { crgfx::TextureLayout::RenderTarget, crgfx::ShaderStageFlags::Unused };
	}
	else if (IsDepthStencil())
	{
		m_defaultState = { crgfx::TextureLayout::DepthStencilReadWrite, crgfx::ShaderStageFlags::Unused };
	}
	else if (IsUnorderedAccess())
	{
		m_defaultState = { crgfx::TextureLayout::RWTexture, crgfx::ShaderStageFlags::Compute };
	}
	else if (IsSwapchain())
	{
		m_defaultState = { crgfx::TextureLayout::Present, crgfx::ShaderStageFlags::Unused };
	}
	else
	{
		// If none of the states above, assume we want to sample this texture
		m_defaultState = { crgfx::TextureLayout::ShaderInput, crgfx::ShaderStageFlags::Pixel };
	}

	m_hardwareMipmapLayouts = {};
}

enum DXGI_FORMAT;

namespace crd3d
{
	DXGI_FORMAT GetDXGIFormat(crgfx::DataFormat::T format);
}

crgfx::MipmapLayout ICrTexture::GetDDSMipSliceLayout(crgfx::DataFormat::T format, uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipmaps, bool isVolume, uint32_t mip, uint32_t slice)
{
	crgfx::MipmapLayout mipmapLayout;

	// TODO Move GetDXGIFormat to an actually generic place
	ddspp::DXGIFormat dxgiFormat = (ddspp::DXGIFormat)crd3d::GetDXGIFormat(format);

	ddspp::Descriptor ddsDescriptor = {};
	ddsDescriptor.format = dxgiFormat;
	ddsDescriptor.width = width;
	ddsDescriptor.height = height;
	ddsDescriptor.depth = depth;
	ddsDescriptor.numMips = numMipmaps;
	ddsDescriptor.type = isVolume ? ddspp::Texture3D : ddspp::Texture2D;
	ddsDescriptor.bitsPerPixelOrBlock = ddspp::get_bits_per_pixel_or_block(ddsDescriptor.format);
	ddspp::get_block_size(ddsDescriptor.format, ddsDescriptor.blockWidth, ddsDescriptor.blockHeight);

	mipmapLayout.offsetBytes = (uint32_t)ddspp::get_offset(ddsDescriptor, mip, slice);
	mipmapLayout.rowPitchBytes = (uint32_t)ddspp::get_row_pitch(ddsDescriptor, mip);
	mipmapLayout.heightInPixelsBlocks = ddspp::get_height_pixels_blocks(ddsDescriptor, mip);

	return mipmapLayout;
}

crgfx::MipmapLayout ICrTexture::GetDDSMipSliceLayout(uint32_t mip, uint32_t slice) const
{
	return GetDDSMipSliceLayout(m_format, m_width, m_height, m_depth, m_mipmapCount, IsVolumeTexture(), mip, slice);
}

crgfx::MipmapLayout ICrTexture::GetHardwareMipSliceLayout(uint32_t mip, uint32_t slice) const
{
	crgfx::MipmapLayout mipmapLayout = m_hardwareMipmapLayouts[mip];
	mipmapLayout.offsetBytes += m_slicePitchBytes * slice;
	return mipmapLayout;
}

void ICrTexture::CopyIntoTextureMemory(uint8_t* destinationData, const uint8_t* sourceData, uint32_t mip, uint32_t slice)
{
	crgfx::MipmapLayout sourceMipLayout = GetDDSMipSliceLayout(mip, slice);
	crgfx::MipmapLayout destinationMipLayout = GetHardwareMipSliceLayout(mip, slice);

	uint32_t mipDepth = CrMax(1u, GetDepth() >> mip);

	// Mipmaps are considered to include depth. Slice in this context only refers to arrays of textures
	// If the mip size is equal, we can copy the entire mipmap in one memcpy
	if (destinationMipLayout.GetMipSize() == sourceMipLayout.GetMipSize())
	{
		memcpy
		(
			destinationData + destinationMipLayout.offsetBytes,
			sourceData + sourceMipLayout.offsetBytes,
			sourceMipLayout.GetMipSize() * mipDepth
		);
	}
	else
	{
		// If the depth pitch is not equal, we process every depth slice
		for (uint32_t d = 0; d < mipDepth; ++d)
		{
			if (sourceMipLayout.rowPitchBytes == destinationMipLayout.rowPitchBytes)
			{
				memcpy
				(
					destinationData + destinationMipLayout.offsetBytes + destinationMipLayout.GetMipSize() * d,
					sourceData + sourceMipLayout.offsetBytes + sourceMipLayout.GetMipSize() * d, 
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
						destinationData + destinationMipLayout.offsetBytes + destinationMipLayout.GetMipSize() * d + row * destinationMipLayout.rowPitchBytes,
						sourceData + sourceMipLayout.offsetBytes + sourceMipLayout.GetMipSize() * d + row * sourceMipLayout.rowPitchBytes,
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