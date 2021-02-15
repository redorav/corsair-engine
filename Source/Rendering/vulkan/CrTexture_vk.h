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

	CrTextureVulkan(ICrRenderDevice* renderDevice, const CrTextureCreateParams& params);

	~CrTextureVulkan();

	VkFormat GetVkFormat() const;

	VkSampleCountFlagBits GetVkSamples() const;

	VkImage GetVkImage() const;

	VkImageView GetVkImageViewAllMipsSlices() const;

	VkImageView GetVkImageViewSingleMipSlice(uint32_t mip, uint32_t slice) const;

	VkImageView GetVkImageViewSingleMipAllSlices(uint32_t mip) const;

	VkImageAspectFlags GetVkImageAspectFlags() const;

	VkAttachmentDescription GetVkAttachmentDescription() const;

private:

	VkDevice							m_vkDevice;

	VkFormat							m_vkFormat;

	VkSampleCountFlagBits				m_vkSamples;

	VkImage								m_vkImage;

	// Main view, can access all mips and slices
	VkImageView							m_vkImageView;

	// This is optional as only render targets and RW textures need them, but can take up
	// some memory per texture (almost 512 bytes)
	CrUniquePtr<AdditionalTextureViews>	m_additionalTextureViews;

	VkDeviceMemory						m_vkMemory;

	VkImageAspectFlags					m_vkAspectMask; // Bits that specify color, depth, stencil or sparse texture

	VkAttachmentDescription				m_vkAttachmentDescription;

	// These are filled in when used as a render target. These can only be used if this attachment is the only one for this combination of framebuffer/renderpass. 
	// E.g. If we use the depth buffer in a depth-prepass we can bind this one. If it's used as an attachment to the GBuffer or the Swapchain, we can't use this renderpass/framebuffer,
	// we need to use the one created specifically for that usage. We can still reuse the attachment description for that renderpass though.
	VkRenderPass						m_vkBaseRenderPass;

	VkFramebuffer						m_vkBaseFramebuffer;
};
