#include "Rendering/CrRendering_pch.h"

#include "CrCommandBuffer_vk.h"
#include "CrTexture_vk.h"
#include "CrRenderDevice_vk.h"
#include "CrVulkan.h"

#include "Core/Logging/ICrDebug.h"

CrTextureVulkan::CrTextureVulkan(ICrRenderDevice* renderDevice, const CrTextureDescriptor& descriptor)
	: ICrTexture(renderDevice, descriptor)
	, m_vkImage(nullptr)
	, m_vkImageViewAllMipsAllSlices(nullptr)
{
	CrRenderDeviceVulkan* vulkanRenderDevice = static_cast<CrRenderDeviceVulkan*>(renderDevice);
	VkDevice vkDevice = vulkanRenderDevice->GetVkDevice();

	VkResult vkResult;
	VkFormat vkFormat = crvk::GetVkFormat(m_format);

	//-----------------
	// Usage properties
	//-----------------

	VkImageUsageFlags vkImageUsageFlags = 0;

	if (IsDepthStencil())
	{
		vkImageUsageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}

	if (IsRenderTarget())
	{
		vkImageUsageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	if (IsUnorderedAccess())
	{
		vkImageUsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
	}

	// All textures can be copied from the GPU to the CPU
	vkImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	// TODO revise this usage
	vkImageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	// TODO Validate that the image format supports the usages and provide an alternative

	vkImageUsageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT; // All images can be sampled

	m_sampleCount = descriptor.sampleCount;
	VkSampleCountFlagBits vkSamples = crvk::GetVkSampleCount(m_sampleCount);

	//----------------------
	// Image type properties
	//----------------------

	VkImageType vkImageType;
	VkImageViewType vkImageViewType;
	VkImageCreateFlags vkCreateFlags = 0;
	uint32_t arrayLayers = descriptor.arraySize;

	if (IsCubemap())
	{
		vkImageType = VK_IMAGE_TYPE_2D;
		if (arrayLayers > 1)
		{
			vkImageViewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
		}
		else
		{
			vkImageViewType = VK_IMAGE_VIEW_TYPE_CUBE;
		}
		vkCreateFlags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}
	else if (IsVolumeTexture())
	{
		vkImageType = VK_IMAGE_TYPE_3D;
		vkImageViewType = VK_IMAGE_VIEW_TYPE_3D;
	}
	else if (Is1DTexture())
	{
		vkImageType = VK_IMAGE_TYPE_1D;

		if (arrayLayers > 1)
		{
			vkImageViewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
		}
		else
		{
			vkImageViewType = VK_IMAGE_VIEW_TYPE_1D;
		}
	}
	else
	{
		vkImageType = VK_IMAGE_TYPE_2D;

		if (arrayLayers > 1)
		{
			vkImageViewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		}
		else
		{
			vkImageViewType = VK_IMAGE_VIEW_TYPE_2D;
		}
	}

	//---------------------------------
	// Create image and allocate memory
	//---------------------------------

	VkMemoryRequirements imageMemoryRequirements = {};
	VmaAllocationInfo vmaAllocationInfo = {};

	if (IsSwapchain())
	{
		m_vkImage = (VkImage)descriptor.extraDataPtr;
		vkGetImageMemoryRequirements(vkDevice, m_vkImage, &imageMemoryRequirements);
		m_usedGPUMemoryBytes = (uint32_t)imageMemoryRequirements.size;
	}
	else
	{
		VkImageCreateInfo imageCreateInfo;
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.format = vkFormat;
		imageCreateInfo.extent = { m_width, m_height, m_depth };
		imageCreateInfo.mipLevels = m_mipmapCount;
		imageCreateInfo.arrayLayers = arrayLayers;
		imageCreateInfo.samples = vkSamples;
		imageCreateInfo.usage = vkImageUsageFlags;
		imageCreateInfo.flags = vkCreateFlags;
		imageCreateInfo.imageType = vkImageType;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.pQueueFamilyIndices = nullptr;
		imageCreateInfo.queueFamilyIndexCount = 0;

		VmaAllocationCreateInfo vmaAllocationCreateInfo = {};

		if (descriptor.usage & cr3d::TextureUsage::CPUReadable)
		{
			imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED; // TODO Condition on initialData

			// TODO This isn't right. Introduce memory access to the texture descriptor and remove usage flag from TextureUsage
			vmaAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
		}
		else
		{
			imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			vmaAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		}

		// http://gpuopen.com/vulkan-device-memory/
		vkResult = vmaCreateImage(vulkanRenderDevice->GetVmaAllocator(), &imageCreateInfo, &vmaAllocationCreateInfo, &m_vkImage, &m_vmaAllocation, &vmaAllocationInfo);
		CrAssert(vkResult == VK_SUCCESS);

		m_usedGPUMemoryBytes = (uint32_t)vmaAllocationInfo.size; // Take note of GPU memory usage
	}

	vulkanRenderDevice->SetVkObjectName((uint64_t)m_vkImage, VK_OBJECT_TYPE_IMAGE, descriptor.name);

	//-----------------------
	// Create the image views
	//-----------------------

	// TODO This needs reworking. Only create if mips or slices are > 1
	if (IsRenderTarget() || IsDepthStencil() || IsUnorderedAccess() || IsSwapchain())
	{
		m_additionalViews = CrUniquePtr<CrVkAdditionalTextureViews>(new CrVkAdditionalTextureViews());
	}

	// There are two parts to the aspect mask it seems:
	// 
	// 1) Transitions and barriers will make use of them and express how many aspects a texture refers to and transition
	// the entirety of the aspects of the resource to a single state, as far as I can tell. For example, setting only
	// the depth aspect of a depth stencil texture when doing a transition to depth only will not transition it properly
	// 
	// 2) The view aspect mask, on the other hand, captures the idea that the shader can only see one part of the texture.
	// For example, the depth texture uses the standard view to be viewed in a shader as depth, and an additional view to 
	// be viewed as stencil (when applicable). The rest of the textures use the color mask

	VkImageAspectFlags viewAspectMask;

	if (cr3d::IsDepthFormat(m_format))
	{
		viewAspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	else
	{
		viewAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	// Shader input image view
	// This image view can see all mips and slices.
	{
		VkImageViewCreateInfo imageViewInfo;
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.pNext = nullptr;
		imageViewInfo.format = vkFormat;
		imageViewInfo.flags = vkCreateFlags;
		imageViewInfo.components = {};
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = m_mipmapCount;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = arrayLayers;
		imageViewInfo.viewType = vkImageViewType;
		imageViewInfo.image = m_vkImage;
		imageViewInfo.subresourceRange.aspectMask = viewAspectMask;

		vkResult = vkCreateImageView(vkDevice, &imageViewInfo, nullptr, &m_vkImageViewAllMipsAllSlices);
		CrAssert(vkResult == VK_SUCCESS);
	}

	// Create views that can only see a single mip or slice. We can use this to either bind a single
	// mip/slice as a texture, or to bind texture as a render target.
	if (m_additionalViews)
	{
		VkImageViewCreateInfo imageViewInfo;
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.pNext = nullptr;
		imageViewInfo.format = vkFormat;
		imageViewInfo.flags = vkCreateFlags;
		imageViewInfo.components = {};
		imageViewInfo.viewType = vkImageViewType;
		imageViewInfo.image = m_vkImage;
		imageViewInfo.subresourceRange.aspectMask = viewAspectMask;

		for (uint32_t mip = 0; mip < m_mipmapCount; ++mip)
		{
			imageViewInfo.subresourceRange.baseMipLevel = mip;
			imageViewInfo.subresourceRange.levelCount = 1;
			imageViewInfo.subresourceRange.baseArrayLayer = 0;
			imageViewInfo.subresourceRange.layerCount = m_depth;
			vkResult = vkCreateImageView(vkDevice, &imageViewInfo, nullptr, &m_additionalViews->m_vkImageViewSingleMipAllSlices[mip]);
			CrAssertMsg(vkResult == VK_SUCCESS, "Failed creating VkImageView");

			m_additionalViews->m_vkImageSingleMipSlice[mip].resize(m_arraySize);

			for (uint32_t slice = 0; slice < m_depth; ++slice)
			{
				imageViewInfo.subresourceRange.baseMipLevel = mip;
				imageViewInfo.subresourceRange.levelCount = 1;
				imageViewInfo.subresourceRange.baseArrayLayer = slice;
				imageViewInfo.subresourceRange.layerCount = 1;
				vkResult = vkCreateImageView(vkDevice, &imageViewInfo, nullptr, &m_additionalViews->m_vkImageSingleMipSlice[mip][slice]);
				CrAssertMsg(vkResult == VK_SUCCESS, "Failed creating VkImageView");
			}
		}

		m_additionalViews->m_vkImageViewStencil = nullptr;

		if (IsDepthStencil() && cr3d::IsDepthStencilFormat(m_format))
		{
			VkImageViewCreateInfo stencilImageViewInfo;
			stencilImageViewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			stencilImageViewInfo.pNext                           = nullptr;
			stencilImageViewInfo.format                          = vkFormat;
			stencilImageViewInfo.flags                           = vkCreateFlags;
			stencilImageViewInfo.components = {};
			stencilImageViewInfo.subresourceRange.baseMipLevel   = 0;
			stencilImageViewInfo.subresourceRange.levelCount     = m_mipmapCount;
			stencilImageViewInfo.subresourceRange.baseArrayLayer = 0;
			stencilImageViewInfo.subresourceRange.layerCount     = arrayLayers;
			stencilImageViewInfo.viewType                        = vkImageViewType;
			stencilImageViewInfo.image                           = m_vkImage;
			stencilImageViewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_STENCIL_BIT;
			vkResult = vkCreateImageView(vkDevice, &stencilImageViewInfo, nullptr, &m_additionalViews->m_vkImageViewStencil);
			CrAssert(vkResult == VK_SUCCESS);
		}
	}

	vulkanRenderDevice->TransitionVkTextureToInitialLayout(this, m_defaultState);

	for (uint32_t mip = 0; mip < m_mipmapCount; ++mip)
	{
		cr3d::MipmapLayout genericMipLayout = GetGenericMipSliceLayout(mip, 0);
		cr3d::MipmapLayout& mipmapLayout  = m_hardwareMipmapLayouts[mip];
		mipmapLayout.rowPitchBytes        = genericMipLayout.rowPitchBytes;
		mipmapLayout.offsetBytes          = genericMipLayout.offsetBytes;
		mipmapLayout.heightInPixelsBlocks = genericMipLayout.heightInPixelsBlocks;
	}

	if (m_arraySize > 1)
	{
		m_slicePitchBytes = GetGenericMipSliceLayout(m_mipmapCount, 0).offsetBytes;
	}

	// If we have initial data, copy it here
	if (descriptor.initialData)
	{
		if (m_usage & cr3d::TextureUsage::Default)
		{
			uint8_t* textureData = m_renderDevice->BeginTextureUpload(this);
			{
				CopyIntoTextureMemory(textureData, descriptor.initialData, 0, m_mipmapCount, 0, m_arraySize);
			}
			m_renderDevice->EndTextureUpload(this);
		}
		else if (m_usage & cr3d::TextureUsage::CPUReadable)
		{
			void* data;
			vkResult = vmaMapMemory(vulkanRenderDevice->GetVmaAllocator(), m_vmaAllocation, &data);
			CrAssert(vkResult == VK_SUCCESS);
			memcpy(data, descriptor.initialData, imageMemoryRequirements.size);
			vmaUnmapMemory(vulkanRenderDevice->GetVmaAllocator(), m_vmaAllocation);
		}
	}
}

CrTextureVulkan::~CrTextureVulkan()
{
	CrRenderDeviceVulkan* vulkanRenderDevice = static_cast<CrRenderDeviceVulkan*>(m_renderDevice);
	VkDevice vkDevice = vulkanRenderDevice->GetVkDevice();

	CrAssert(m_vkImageViewAllMipsAllSlices);
	CrAssert(m_vkImage);

	// Don't destroy images we don't manage. The swapchain image and memory was handed to us by the OS
	if (!IsSwapchain())
	{
		vmaDestroyImage(vulkanRenderDevice->GetVmaAllocator(), m_vkImage, m_vmaAllocation);
	}

	vkDestroyImageView(vkDevice, m_vkImageViewAllMipsAllSlices, nullptr);

	if (m_additionalViews)
	{
		for (uint32_t mip = 0; mip < m_additionalViews->m_vkImageViewSingleMipAllSlices.size(); ++mip)
		{
			vkDestroyImageView(vkDevice, m_additionalViews->m_vkImageViewSingleMipAllSlices[mip], nullptr);
		}

		for (uint32_t mip = 0; mip < m_additionalViews->m_vkImageSingleMipSlice.size(); ++mip)
		{
			for (uint32_t slice = 0; slice < m_additionalViews->m_vkImageSingleMipSlice[mip].size(); ++slice)
			{
				vkDestroyImageView(vkDevice, m_additionalViews->m_vkImageSingleMipSlice[mip][slice], nullptr);
			}
		}
	}
}
