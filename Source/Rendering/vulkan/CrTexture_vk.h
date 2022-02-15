#pragma once

#include "Rendering/ICrTexture.h"

#include "Core/Containers/CrArray.h"
#include "Core/Containers/CrVector.h"

#include <vulkan/vulkan.h>

class ICrRenderDevice;

// TODO Create platform-independent so sync between platforms
// Perhaps by storing void* as the view?
struct AdditionalTextureViews
{
	CrArray<CrVector<VkImageView>, ICrTexture::MaxMipmaps>	m_vkImageSingleMipSlice; // Each mipmap can have a variable amount of slices.

	CrArray<VkImageView, ICrTexture::MaxMipmaps>			m_vkImageViewSingleMipAllSlices; // Each mipmap can see all slices
};

class CrTextureVulkan final : public ICrTexture
{
public:

	CrTextureVulkan(ICrRenderDevice* renderDevice, const CrTextureDescriptor& descriptor);

	~CrTextureVulkan();

	VkImage GetVkImage() const;

	VkImageView GetVkImageViewAllMipsSlices() const;

	VkImageView GetVkImageViewSingleMipSlice(uint32_t mip, uint32_t slice) const;

	VkImageView GetVkImageViewSingleMipAllSlices(uint32_t mip) const;

	VkImageAspectFlags GetVkImageAspectMask() const;

private:

	VkImage								m_vkImage;

	// Main view, can access all mips and slices
	VkImageView							m_vkImageView;

	// This is optional as only render targets and RW textures need them, but can take up
	// some memory per texture (almost 512 bytes)
	CrUniquePtr<AdditionalTextureViews>	m_additionalTextureViews;

	VmaAllocation						m_vmaAllocation;

	VkImageAspectFlags					m_vkAspectMask; // Bits that specify color, depth, stencil or sparse texture
};
