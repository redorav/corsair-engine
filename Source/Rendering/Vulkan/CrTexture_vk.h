#pragma once

#include "Rendering/ICrTexture.h"

#include "Core/Containers/CrVector.h"

#include <vulkan/vulkan.h>
#include "CrVMA.h"

#include "crstl/array.h"

class ICrRenderDevice;

struct CrVkAdditionalTextureViews
{
	crstl::array<CrVector<VkImageView>, cr3d::MaxMipmaps> m_vkImageSingleMipSlice; // Each mipmap can have a variable amount of slices.
	crstl::array<VkImageView, cr3d::MaxMipmaps> m_vkImageViewSingleMipAllSlices; // Each mipmap can see all slices

	VkImageView m_vkImageViewStencil;
};

class CrTextureVulkan final : public ICrTexture
{
public:

	CrTextureVulkan(ICrRenderDevice* renderDevice, const CrTextureDescriptor& descriptor);

	~CrTextureVulkan();

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