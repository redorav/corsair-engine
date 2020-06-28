#pragma once

#include <cstdint>

#include "Core/String/CrString.h"
#include "Core/CrCoreForwardDeclarations.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

struct CrTextureCreateParams
{
	CrTextureCreateParams(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipmaps, uint32_t arraySize, 
						  cr3d::DataFormat::T format, cr3d::SampleCount sampleCount, cr3d::TextureType type, 
						  cr3d::TextureUsageFlags usage, const CrString& name, 
						  void* initialData, uint32_t extraData, void* extraDataPtr);

	CrTextureCreateParams();
	
	CrTextureCreateParams(uint32_t width, uint32_t height, cr3d::DataFormat::T format, cr3d::TextureUsageFlags usage, const CrString& name);

	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t numMipmaps;
	uint32_t arraySize;
	cr3d::DataFormat::T format;
	cr3d::SampleCount sampleCount;
	cr3d::TextureType type;
	cr3d::TextureUsageFlags usage;
	CrString name;
	const void* initialData; // TODO do better
	uint32_t initialDataSize;
	// This extra data exists to be able to pass platform-specific information
	// such as swapchain information, etc.
	uint32_t extraData;
	void* extraDataPtr;
};

class ICrTexture
{
public:

	~ICrTexture();

	bool IsCubemap() const;

	bool IsRenderTarget() const;

	bool IsUAV() const;

	bool IsVolumeTexture() const;

	bool IsDepth() const;

	cr3d::DataFormat::T GetFormat() const;

	cr3d::SampleCount GetSampleCount() const;

	uint32_t GetWidth() const;

	uint32_t GetHeight() const;

	uint32_t GetDepth() const;

	static uint32_t GetMipSliceOffset(cr3d::DataFormat::T format, uint32_t width, uint32_t height, uint32_t numMipmaps, bool isVolume, uint32_t mip, uint32_t slice);

	uint32_t GetMipSliceOffset(uint32_t mip, uint32_t slice) const;

	// TODO
	// How to do lock/unlock pairs for cubemaps, texture arrays, etc.
	// Lock(mip, face)
	// Unlock()

protected:

	// We don't allow the constructor externally
	ICrTexture(const CrTextureCreateParams& params);

	cr3d::DataFormat::T m_format;

	cr3d::SampleCount m_sampleCount;

	cr3d::TextureType m_type;

	uint32_t m_width;

	uint32_t m_height;

	uint32_t m_depth;

	uint32_t m_numMipmaps;

	uint32_t m_usedMemory;

	cr3d::TextureUsageFlags m_usage;
};
