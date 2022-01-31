#pragma once

#include <cstdint>

#include "Core/String/CrFixedString.h"
#include "Core/CrCoreForwardDeclarations.h"

#include "Rendering/CrRendering.h"
#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrGPUDeletable.h"

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
	const void* initialData; // TODO do better
	uint64_t initialDataSize; // TODO delete from here

	// This extra data exists to be able to pass platform-specific information
	// such as swapchain information, etc.
	uint32_t extraData;
	void* extraDataPtr;
	
	CrFixedString128 name;
};

class ICrTexture : public CrGPUDeletable
{
public:

	static const uint32_t MaxMipmaps = 14;

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

	uint32_t GetArraySize() const { return m_arraySize; }

	static uint32_t GetMipSliceOffset(cr3d::DataFormat::T format, uint32_t width, uint32_t height, uint32_t numMipmaps, bool isVolume, uint32_t mip, uint32_t slice);

	uint32_t GetMipSliceOffset(uint32_t mip, uint32_t slice) const;

	uint32_t GetUsedGPUMemory() const { return m_usedGPUMemory; }

	// TODO
	// How to do lock/unlock pairs for cubemaps, texture arrays, etc.
	// Lock(mip, face)
	// Unlock()

protected:

	// We don't allow the constructor externally
	ICrTexture(ICrRenderDevice* renderDevice, const CrTextureDescriptor& descriptor);

	ICrRenderDevice* m_renderDevice;

	cr3d::DataFormat::T m_format;

	cr3d::SampleCount m_sampleCount;

	cr3d::TextureType m_type;

	uint32_t m_width;

	uint32_t m_height;

	uint32_t m_depth;

	uint32_t m_arraySize;

	uint32_t m_mipmapCount;

	uint32_t m_usedGPUMemory;

	cr3d::TextureUsageFlags m_usage;
};


