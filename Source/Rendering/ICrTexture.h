#pragma once

#include "stdint.h"

#include "Core/String/CrFixedString.h"
#include "Core/CrCoreForwardDeclarations.h"

#include "Rendering/CrRendering.h"
#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrGPUDeletable.h"

#include "crstl/array.h"
#include "crstl/fixed_string.h"

struct CrTextureDescriptor
{
	CrTextureDescriptor();
	
	uint32_t width;
	uint32_t height;
	uint32_t depth; // Valid for volume textures
	uint32_t mipmapCount;
	uint32_t arraySize; // Valid for texture or cubemap arrays
	cr3d::DataFormat::T format;
	cr3d::SampleCount sampleCount;
	cr3d::TextureType type;
	cr3d::TextureUsageFlags usage;

	// Defaults to zero
	union
	{
		float colorClear[4] = {};
		struct
		{
			float depthClear;
			uint8_t stencilClear;
		};
	};

	const uint8_t* initialData; // TODO do better
	uint64_t initialDataSize; // TODO delete from here

	// This extra data exists to be able to pass platform-specific information
	// such as swapchain information, etc.
	uint32_t extraData;
	void* extraDataPtr;
	
	const char* name;
};

class ICrTexture : public CrGPUAutoDeletable
{
public:

	virtual ~ICrTexture();

	bool IsRenderTarget() const { return (m_usage & cr3d::TextureUsage::RenderTarget) != 0; }

	bool IsUnorderedAccess() const { return (m_usage & cr3d::TextureUsage::UnorderedAccess) != 0; }

	bool IsSwapchain() const { return (m_usage & cr3d::TextureUsage::SwapChain) != 0; }

	bool IsDepthStencil() const { return (m_usage & cr3d::TextureUsage::DepthStencil) != 0; }

	bool Is1DTexture() const { return m_type == cr3d::TextureType::Tex1D; }

	bool Is2DTexture() const { return m_type == cr3d::TextureType::Tex2D; }

	bool IsVolumeTexture() const { return m_type == cr3d::TextureType::Volume; }

	bool IsCubemap() const { return m_type == cr3d::TextureType::Cubemap; }

	cr3d::DataFormat::T GetFormat() const { return m_format; }

	cr3d::SampleCount GetSampleCount() const { return m_sampleCount; }

	uint32_t GetWidth() const { return m_width; }

	uint32_t GetHeight() const { return m_height; }

	uint32_t GetDepth() const { return m_depth; }

	uint32_t GetMipmapCount() const { return m_mipmapCount; }

	uint32_t GetSliceCount() const { return m_arraySize; }

	cr3d::TextureState GetDefaultState() const { return m_defaultState; }

	static cr3d::MipmapLayout GetDDSMipSliceLayout(cr3d::DataFormat::T format, uint32_t width, uint32_t height, uint32_t numMipmaps, bool isVolume, uint32_t mip, uint32_t slice);

	cr3d::MipmapLayout GetDDSMipSliceLayout(uint32_t mip, uint32_t slice) const;

	cr3d::MipmapLayout GetHardwareMipSliceLayout(uint32_t mip, uint32_t slice) const;

	uint32_t GetUsedGPUMemory() const { return m_usedGPUMemoryBytes; }

	void CopyIntoTextureMemory(uint8_t* destinationData, const uint8_t* sourceData, uint32_t mip, uint32_t slice);

	void CopyIntoTextureMemory
	(
		uint8_t* destinationData, const uint8_t* sourceData, 
		uint32_t startMip, uint32_t mipCount, uint32_t startSlice, uint32_t sliceCount
	);

#if !defined(CR_CONFIG_FINAL)

	const char* GetDebugName() const
	{
		return m_debugName.c_str();
	}

#endif

protected:

	// We don't allow the constructor externally
	ICrTexture(ICrRenderDevice* renderDevice, const CrTextureDescriptor& descriptor);

	cr3d::DataFormat::T m_format;

	cr3d::SampleCount m_sampleCount;

	cr3d::TextureType m_type;

	uint32_t m_width;

	uint32_t m_height;

	uint32_t m_depth;

	uint32_t m_arraySize;

	uint32_t m_mipmapCount;

	// Must be populated by the platform-specific implementation. This is the texture size as reported
	// by the hardware and takes into account padding, etc Used for reporting
	uint32_t m_usedGPUMemoryBytes;

	cr3d::TextureState m_defaultState;

	cr3d::TextureUsageFlags m_usage;

	// Mipmap layout that is platform-dependent
	crstl::array<cr3d::MipmapLayout, cr3d::MaxMipmaps> m_hardwareMipmapLayouts;

	// Distance between two consecutive slices
	uint32_t m_slicePitchBytes;

#if !defined(CR_CONFIG_FINAL)

	crstl::fixed_string128 m_debugName;

#endif
};
