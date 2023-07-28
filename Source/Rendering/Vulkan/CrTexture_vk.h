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
	CrArray<CrVector<VkImageView>, cr3d::MaxMipmaps> m_vkImageSingleMipSlice; // Each mipmap can have a variable amount of slices.
	CrArray<VkImageView, cr3d::MaxMipmaps> m_vkImageViewSingleMipAllSlices; // Each mipmap can see all slices

	VkImageView m_vkImageViewStencil;
};

class CrTextureVulkan final : public ICrTexture
{
public:

	CrTextureVulkan(ICrRenderDevice* renderDevice, const CrTextureDescriptor& descriptor);

	~CrTextureVulkan();

	static const CrVkImageStateInfo& GetVkImageStateInfo(cr3d::TextureLayout::T textureLayout);

	static VkPipelineStageFlags GetVkPipelineStageFlags(const cr3d::TextureState& textureState);

	VkImage GetVkImage() const { return m_vkImage; }

	VkImageView GetVkImageViewShaderAllMipsAllSlices() const { return m_vkImageViewAllMipsAllSlices; }

	// Used for depth-stencil images when we want to bind the stencil
	// The base image view should be assumed to be depth
	VkImageView GetVkImageViewStencil() const { return m_additionalViews->m_vkImageViewStencil; }

	VkImageView GetVkImageViewSingleMipSlice(uint32_t mip, uint32_t slice) const { return m_additionalViews->m_vkImageSingleMipSlice[mip][slice]; }

	VkImageView GetVkImageViewSingleMipAllSlices(uint32_t mip) const { return m_additionalViews->m_vkImageViewSingleMipAllSlices[mip]; }

private:

	VkImage								m_vkImage;

	// Main shader view, can access all mips and slices
	VkImageView							m_vkImageViewAllMipsAllSlices;

	// This is optional as only render targets and RW textures need them, but can take up
	// some memory per texture (almost 512 bytes)
	CrUniquePtr<CrVkAdditionalTextureViews>	m_additionalViews;

	// Allocation handle through the VMA allocator
	VmaAllocation						m_vmaAllocation;
};