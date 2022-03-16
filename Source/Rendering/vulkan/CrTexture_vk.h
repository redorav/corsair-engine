#pragma once

#include "Rendering/ICrTexture.h"

#include "Core/Containers/CrArray.h"
#include "Core/Containers/CrVector.h"

#include <vulkan/vulkan.h>
#include "CrVMA.h"

class ICrRenderDevice;

// Sourced from https://github.com/Tobski/simple_vulkan_synchronization/blob/master/thsvs_simpler_vulkan_synchronization.h
// This shows how to make the combinations but it tries to tie them with the point in the pipeline at which they
// are used, whereas we decouple that and get fewer combinations
struct CrVkImageStateInfo
{
	VkImageLayout imageLayout = VK_IMAGE_LAYOUT_MAX_ENUM;
	VkAccessFlags accessMask = VK_ACCESS_FLAG_BITS_MAX_ENUM;
};

struct CrVkAdditionalTextureViews
{
	CrArray<CrVector<VkImageView>, ICrTexture::MaxMipmaps>	m_vkImageSingleMipSlice; // Each mipmap can have a variable amount of slices.
	CrArray<VkImageView, ICrTexture::MaxMipmaps>			m_vkImageViewSingleMipAllSlices; // Each mipmap can see all slices
};

class CrTextureVulkan final : public ICrTexture
{
public:

	CrTextureVulkan(ICrRenderDevice* renderDevice, const CrTextureDescriptor& descriptor);

	~CrTextureVulkan();

	static const CrVkImageStateInfo& GetVkImageStateInfo(cr3d::TextureState::T textureState);

	static VkPipelineStageFlags GetVkPipelineStageFlags(cr3d::TextureState::T textureState, cr3d::ShaderStageFlags::T shaderStages);

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
	CrUniquePtr<CrVkAdditionalTextureViews>	m_additionalViews;

	VmaAllocation						m_vmaAllocation;

	VkImageAspectFlags					m_vkAspectMask; // Bits that specify color, depth, stencil or sparse texture
};
