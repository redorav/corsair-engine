#include "CrRendering_pch.h"

#include "CrCommandQueue_vk.h"
#include "CrCommandBuffer_vk.h"
#include "CrTexture_vk.h"
#include "CrRenderDevice_vk.h"
#include "CrVulkan.h"

#include "Core/Logging/ICrDebug.h"

CrTextureVulkan::CrTextureVulkan(ICrRenderDevice* renderDevice, const CrTextureDescriptor& descriptor)
	: ICrTexture(renderDevice, descriptor)
	, m_vkImage(nullptr)
	, m_vkImageView(nullptr)
{
	CrRenderDeviceVulkan* vulkanRenderDevice = static_cast<CrRenderDeviceVulkan*>(renderDevice);
	VkDevice vkDevice = vulkanRenderDevice->GetVkDevice();

	VkResult vkResult;
	VkFormat vkFormat = crvk::GetVkFormat(m_format);

	//-----------------
	// Usage properties
	//-----------------

	VkImageUsageFlags usageFlags = 0;

	if (IsDepthStencil())
	{
		usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}

	if (IsRenderTarget())
	{
		usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	if (IsUnorderedAccess())
	{
		usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
	}

	// All textures can be copied from the GPU to the CPU
	usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	// TODO Validate that the image format supports the usages and provide an alternative

	usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT; // All images can be sampled

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
			vkCreateFlags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR;
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
			vkCreateFlags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR;
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
		m_usedGPUMemory = (uint32_t)imageMemoryRequirements.size;
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
		imageCreateInfo.usage = usageFlags | VK_IMAGE_USAGE_TRANSFER_DST_BIT; // TODO revise this usage
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

		m_usedGPUMemory = (uint32_t)vmaAllocationInfo.size; // Take note of GPU memory usage
	}

	vulkanRenderDevice->SetVkObjectName((uint64_t)m_vkImage, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, descriptor.name);

	//-----------------------
	// Create the image views
	//-----------------------

	// TODO This needs reworking. Only create if mips or slices are > 1
	if (IsRenderTarget() || IsDepthStencil() || IsUnorderedAccess() || IsSwapchain())
	{
		m_additionalTextureViews = CrUniquePtr<AdditionalTextureViews>(new AdditionalTextureViews());
	}

	if (IsDepthStencil()) // TODO sparse textures
	{
		m_vkAspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else
	{
		m_vkAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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
		imageViewInfo.subresourceRange.aspectMask = m_vkAspectMask;

		vkResult = vkCreateImageView(vkDevice, &imageViewInfo, nullptr, &m_vkImageView);
		CrAssert(vkResult == VK_SUCCESS);
	}

	// Create views that can only see a single mip or slice. We can use this to either bind a single
	// mip/slice as a texture, or to bind texture as a render target.
	if (m_additionalTextureViews)
	{
		VkImageViewCreateInfo imageViewInfo;
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.pNext = nullptr;
		imageViewInfo.format = vkFormat;
		imageViewInfo.flags = vkCreateFlags;
		imageViewInfo.components = {};
		imageViewInfo.viewType = vkImageViewType;
		imageViewInfo.image = m_vkImage;
		imageViewInfo.subresourceRange.aspectMask = m_vkAspectMask;

		for (uint32_t mip = 0; mip < m_mipmapCount; ++mip)
		{
			imageViewInfo.subresourceRange.baseMipLevel = mip;
			imageViewInfo.subresourceRange.levelCount = 1;
			imageViewInfo.subresourceRange.baseArrayLayer = 0;
			imageViewInfo.subresourceRange.layerCount = m_depth;
			vkResult = vkCreateImageView(vkDevice, &imageViewInfo, nullptr, &m_additionalTextureViews->m_vkImageViewSingleMipAllSlices[mip]);
			CrAssertMsg(vkResult == VK_SUCCESS, "Failed creating VkImageView");

			m_additionalTextureViews->m_vkImageSingleMipSlice[mip].resize(m_depth);

			for (uint32_t slice = 0; slice < m_depth; ++slice)
			{
				imageViewInfo.subresourceRange.baseMipLevel = mip;
				imageViewInfo.subresourceRange.levelCount = 1;
				imageViewInfo.subresourceRange.baseArrayLayer = slice;
				imageViewInfo.subresourceRange.layerCount = 1;
				vkResult = vkCreateImageView(vkDevice, &imageViewInfo, nullptr, &m_additionalTextureViews->m_vkImageSingleMipSlice[mip][slice]);
				CrAssertMsg(vkResult == VK_SUCCESS, "Failed creating VkImageView");
			}
		}
	}

	// If we have initial data, copy it here via a staging buffer
	// TODO An optimization to this is to use the Lock()/Unlock() pattern to get a pointer
	// That way we can load the data directly into the buffer, avoiding one of the copies
	if (descriptor.initialData)
	{
		if (m_usage & cr3d::TextureUsage::Default)
		{
			CrHardwareGPUBufferDescriptor stagingBufferDescriptor(cr3d::BufferUsage::TransferSrc, cr3d::MemoryAccess::Staging, (uint32_t)m_usedGPUMemory);
			CrSharedPtr<ICrHardwareGPUBuffer> stagingBuffer = CrSharedPtr<ICrHardwareGPUBuffer>(vulkanRenderDevice->CreateHardwareGPUBuffer(stagingBufferDescriptor));
			CrHardwareGPUBufferVulkan* vulkanStagingBuffer = static_cast<CrHardwareGPUBufferVulkan*>(stagingBuffer.get());

			void* data = stagingBuffer->Lock();
			memcpy(data, descriptor.initialData, descriptor.initialDataSize);
			stagingBuffer->Unlock();

			// Setup buffer copy regions for each mip level
			CrVector<VkBufferImageCopy> bufferCopyRegions;

			for (uint32_t mip = 0; mip < m_mipmapCount; mip++)
			{
				VkBufferImageCopy bufferCopyRegion;
				bufferCopyRegion.imageSubresource.aspectMask = m_vkAspectMask;
				bufferCopyRegion.imageSubresource.mipLevel = mip;
				bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
				bufferCopyRegion.imageSubresource.layerCount = m_arraySize;
				bufferCopyRegion.imageExtent = { CrMax(m_width >> mip, 1u), CrMax(m_height >> mip, 1u), CrMax(m_depth >> mip, 1u) };
				bufferCopyRegion.imageOffset = { 0, 0, 0 };

				// TODO Fix this is dds-specific
				bufferCopyRegion.bufferOffset = GetMipSliceOffset(mip, 0);
				bufferCopyRegion.bufferRowLength = 0;
				bufferCopyRegion.bufferImageHeight = 0;
				bufferCopyRegions.push_back(bufferCopyRegion);
			}

			VkImageSubresourceRange subresourceRange;
			subresourceRange.aspectMask = m_vkAspectMask;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = m_mipmapCount;
			subresourceRange.baseArrayLayer = 0;
			subresourceRange.layerCount = m_arraySize; // TODO

			// TODO Rework how this all works. We shouldn't be stalling here or creating new command buffers.
			// However, changing this requires more framework to be in place
			CrCommandBufferSharedHandle cmdBuffer = renderDevice->GetAuxiliaryCommandBuffer();
			CrCommandBufferVulkan* vulkanCmdBuffer = static_cast<CrCommandBufferVulkan*>(cmdBuffer.get());
			vulkanCmdBuffer->Begin();
			{
				// Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
				VkImageMemoryBarrier imageMemoryBarrier;
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.pNext = nullptr;
				imageMemoryBarrier.image = m_vkImage;
				imageMemoryBarrier.subresourceRange = subresourceRange;
				imageMemoryBarrier.srcAccessMask = 0;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

				// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition 
				// Source pipeline stage is host write/read execution (VK_PIPELINE_STAGE_HOST_BIT)
				// Destination pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
				vkCmdPipelineBarrier(vulkanCmdBuffer->GetVkCommandBuffer(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

				// Copy mip levels from staging buffer
				vkCmdCopyBufferToImage(vulkanCmdBuffer->GetVkCommandBuffer(), vulkanStagingBuffer->GetVkBuffer(), m_vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());

				// Once the data has been uploaded we transfer to the texture image to the shader read layout, so it can be sampled from
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

				// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition 
				// Source pipeline stage stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
				// Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
				vkCmdPipelineBarrier(vulkanCmdBuffer->GetVkCommandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
			}
			vulkanCmdBuffer->End();
			vulkanCmdBuffer->Submit();
			renderDevice->GetMainCommandQueue()->WaitIdle();
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

	CrAssert(m_vkImageView);
	CrAssert(m_vkImage);

	// Don't destroy images we don't manage. The swapchain image and memory was handed to us by the OS
	if (m_usage != cr3d::TextureUsage::SwapChain)
	{
		vmaDestroyImage(vulkanRenderDevice->GetVmaAllocator(), m_vkImage, m_vmaAllocation);
	}

	vkDestroyImageView(vkDevice, m_vkImageView, nullptr);

	if (m_additionalTextureViews)
	{
		for (uint32_t mip = 0; mip < m_additionalTextureViews->m_vkImageViewSingleMipAllSlices.size(); ++mip)
		{
			vkDestroyImageView(vkDevice, m_additionalTextureViews->m_vkImageViewSingleMipAllSlices[mip], nullptr);
		}

		for (uint32_t mip = 0; mip < m_additionalTextureViews->m_vkImageSingleMipSlice.size(); ++mip)
		{
			for (uint32_t slice = 0; slice < m_additionalTextureViews->m_vkImageSingleMipSlice[mip].size(); ++slice)
			{
				vkDestroyImageView(vkDevice, m_additionalTextureViews->m_vkImageSingleMipSlice[mip][slice], nullptr);
			}
		}
	}
}

VkImage CrTextureVulkan::GetVkImage() const
{
	return m_vkImage;
}

VkImageView CrTextureVulkan::GetVkImageViewAllMipsSlices() const
{
	return m_vkImageView;
}

VkImageView CrTextureVulkan::GetVkImageViewSingleMipSlice(uint32_t mip, uint32_t slice) const
{
	return m_additionalTextureViews->m_vkImageSingleMipSlice[mip][slice];
}

VkImageView CrTextureVulkan::GetVkImageViewSingleMipAllSlices(uint32_t mip) const
{
	return m_additionalTextureViews->m_vkImageViewSingleMipAllSlices[mip];
}

VkImageAspectFlags CrTextureVulkan::GetVkImageAspectMask() const
{
	return m_vkAspectMask;
}

CrArray<CrVkImageStateInfo, cr3d::TextureState::Count> CrVkImageResourceStateTable;

static bool PopulateVkImageResourceTable()
{
	CrVkImageResourceStateTable[cr3d::TextureState::Undefined]         = { VK_IMAGE_LAYOUT_UNDEFINED,                        VK_ACCESS_NONE_KHR };
	CrVkImageResourceStateTable[cr3d::TextureState::ShaderInput]       = { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         VK_ACCESS_SHADER_READ_BIT };
	CrVkImageResourceStateTable[cr3d::TextureState::RenderTarget]      = { VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,         VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT };
	CrVkImageResourceStateTable[cr3d::TextureState::RWTexture]         = { VK_IMAGE_LAYOUT_GENERAL,                          VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT };
	CrVkImageResourceStateTable[cr3d::TextureState::Present]           = { VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,                  0 };
	CrVkImageResourceStateTable[cr3d::TextureState::DepthStencilRead]  = { VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  VK_ACCESS_SHADER_READ_BIT };
	CrVkImageResourceStateTable[cr3d::TextureState::DepthStencilWrite] = { VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT };
	CrVkImageResourceStateTable[cr3d::TextureState::CopySource]        = { VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,             VK_ACCESS_TRANSFER_READ_BIT };
	CrVkImageResourceStateTable[cr3d::TextureState::CopyDestination]   = { VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,             VK_ACCESS_TRANSFER_WRITE_BIT };
	CrVkImageResourceStateTable[cr3d::TextureState::PreInitialized]    = { VK_IMAGE_LAYOUT_PREINITIALIZED,                   VK_ACCESS_HOST_WRITE_BIT };

	// Validate the entries on boot
	for (const CrVkImageStateInfo& resourceInfo : CrVkImageResourceStateTable)
	{
		CrAssertMsg((resourceInfo.imageLayout != VK_IMAGE_LAYOUT_MAX_ENUM) && (resourceInfo.accessMask != VK_ACCESS_FLAG_BITS_MAX_ENUM), "Resource info entry is invalid");
	}

	return true;
};

static const bool DummyPopulateVkImageTable = PopulateVkImageResourceTable();

const CrVkImageStateInfo& CrTextureVulkan::GetVkImageStateInfo(cr3d::TextureState::T textureState)
{
	return CrVkImageResourceStateTable[textureState];
}
