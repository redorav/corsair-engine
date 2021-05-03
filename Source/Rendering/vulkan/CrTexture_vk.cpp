#include "CrRendering_pch.h"

#include "CrCommandQueue_vk.h"
#include "CrCommandBuffer_vk.h"
#include "CrTexture_vk.h"
#include "CrRenderDevice_vk.h"
#include "CrVulkan.h"

#include "Core/Logging/ICrDebug.h"

CrTextureVulkan::CrTextureVulkan(ICrRenderDevice* renderDevice, const CrTextureDescriptor& descriptor)
	: ICrTexture(renderDevice, descriptor)
	, m_vkBaseFramebuffer(nullptr)
	, m_vkBaseRenderPass(nullptr)
	, m_vkImage(nullptr)
	, m_vkImageView(nullptr)
	, m_vkMemory(nullptr)
{
	CrRenderDeviceVulkan* vulkanDevice = static_cast<CrRenderDeviceVulkan*>(renderDevice);

	m_vkDevice = vulkanDevice->GetVkDevice();

	VkResult result;
	m_vkFormat = crvk::GetVkFormat(m_format);

	//-----------------
	// Usage properties
	//-----------------

	VkImageUsageFlags usageFlags = 0;
	VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (IsSwapchain())
	{
		imageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}
	else
	{
		if (IsDepthStencil())
		{
			usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		if (IsRenderTarget())
		{
			usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		if (IsUnorderedAccess())
		{
			usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
		}
	}

	// TODO Validate that the image format supports the usages and provide an alternative

	usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT; // All images can be sampled

	m_sampleCount = descriptor.sampleCount;
	m_vkSamples = crvk::GetVkSampleCount(m_sampleCount);

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

	VkMemoryRequirements imageMemoryRequirements;

	if (IsSwapchain())
	{
		m_vkImage = (VkImage)descriptor.extraDataPtr;
		vkGetImageMemoryRequirements(m_vkDevice, m_vkImage, &imageMemoryRequirements);
	}
	else
	{
		VkImageCreateInfo imageCreateInfo;
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.format = m_vkFormat;
		imageCreateInfo.extent = { m_width, m_height, m_depth };
		imageCreateInfo.mipLevels = m_mipmapCount;
		imageCreateInfo.arrayLayers = arrayLayers;
		imageCreateInfo.samples = m_vkSamples;
		imageCreateInfo.usage = usageFlags | VK_IMAGE_USAGE_TRANSFER_DST_BIT; // TODO revise this usage
		imageCreateInfo.flags = vkCreateFlags;
		imageCreateInfo.imageType = vkImageType;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.pQueueFamilyIndices = nullptr;
		imageCreateInfo.queueFamilyIndexCount = 0;

		VkMemoryPropertyFlags memoryFlags = 0;

		if (descriptor.usage & cr3d::TextureUsage::CPUReadable)
		{
			imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED; // TODO Condition on initialData

			memoryFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
			memoryFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}
		else
		{
			imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			memoryFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}

		result = vkCreateImage(m_vkDevice, &imageCreateInfo, nullptr, &m_vkImage);
		CrAssert(result == VK_SUCCESS);
		
		vkGetImageMemoryRequirements(m_vkDevice, m_vkImage, &imageMemoryRequirements); // http://gpuopen.com/vulkan-device-memory/

		VkMemoryAllocateInfo memAlloc;
		memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAlloc.pNext = nullptr;
		memAlloc.memoryTypeIndex = 0;
		memAlloc.allocationSize = imageMemoryRequirements.size;

		memAlloc.memoryTypeIndex = vulkanDevice->GetVkMemoryType(imageMemoryRequirements.memoryTypeBits, memoryFlags);

		result = vkAllocateMemory(m_vkDevice, &memAlloc, nullptr, &m_vkMemory);
		CrAssert(result == VK_SUCCESS);

		result = vkBindImageMemory(m_vkDevice, m_vkImage, m_vkMemory, 0);
		CrAssert(result == VK_SUCCESS);
	}

	m_usedMemory = (uint32_t)imageMemoryRequirements.size; // Take note of GPU memory usage

	//-----------------------
	// Create the image views
	//-----------------------

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
		imageViewInfo.format = m_vkFormat;
		imageViewInfo.flags = vkCreateFlags;
		imageViewInfo.components = {};
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = m_mipmapCount;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = arrayLayers;
		imageViewInfo.viewType = vkImageViewType;
		imageViewInfo.image = m_vkImage;
		imageViewInfo.subresourceRange.aspectMask = m_vkAspectMask;

		result = vkCreateImageView(m_vkDevice, &imageViewInfo, nullptr, &m_vkImageView);
		CrAssert(result == VK_SUCCESS);
	}

	// Create views that can only see a single mip or slice. We can use this to either bind a single
	// mip/slice as a texture, or to bind texture as a render target.
	if (m_additionalTextureViews)
	{
		VkImageViewCreateInfo imageViewInfo;
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.pNext = nullptr;
		imageViewInfo.format = m_vkFormat;
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
			result = vkCreateImageView(m_vkDevice, &imageViewInfo, nullptr, &m_additionalTextureViews->m_vkImageViewSingleMipAllSlices[mip]);
			CrAssertMsg(result == VK_SUCCESS, "Failed creating VkImageView");

			m_additionalTextureViews->m_vkImageSingleMipSlice[mip].resize(m_depth);

			for (uint32_t slice = 0; slice < m_depth; ++slice)
			{
				imageViewInfo.subresourceRange.baseMipLevel = mip;
				imageViewInfo.subresourceRange.levelCount = 1;
				imageViewInfo.subresourceRange.baseArrayLayer = slice;
				imageViewInfo.subresourceRange.layerCount = 1;
				result = vkCreateImageView(m_vkDevice, &imageViewInfo, nullptr, &m_additionalTextureViews->m_vkImageSingleMipSlice[mip][slice]);
				CrAssertMsg(result == VK_SUCCESS, "Failed creating VkImageView");
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
			VkBuffer stagingBuffer;

			// Create a staging buffer
			VkBufferCreateInfo stagingBufferCreateInfo = crvk::CreateVkBufferCreateInfo
			(
				0, imageMemoryRequirements.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
				VK_SHARING_MODE_EXCLUSIVE, 0, nullptr
			);

			result = vkCreateBuffer(m_vkDevice, &stagingBufferCreateInfo, nullptr, &stagingBuffer);
			CrAssertMsg(result == VK_SUCCESS, "Failed creating VkBuffer");

			VkMemoryRequirements bufferMemoryRequirements;
			vkGetBufferMemoryRequirements(m_vkDevice, stagingBuffer, &bufferMemoryRequirements);
			CrAssert(descriptor.initialDataSize <= bufferMemoryRequirements.size);

			VkMemoryAllocateInfo memAllocInfo = crvk::CreateVkMemoryAllocateInfo(bufferMemoryRequirements.size, vulkanDevice->GetVkMemoryType(bufferMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

			VkDeviceMemory stagingMemory;
			result = vkAllocateMemory(m_vkDevice, &memAllocInfo, nullptr, &stagingMemory);
			CrAssert(result == VK_SUCCESS);

			result = vkBindBufferMemory(m_vkDevice, stagingBuffer, stagingMemory, 0);
			CrAssert(result == VK_SUCCESS);

			// Copy into buffer
			void* data;
			result = vkMapMemory(m_vkDevice, stagingMemory, 0, bufferMemoryRequirements.size, 0, &data);
			CrAssert(result == VK_SUCCESS);
			memcpy(data, descriptor.initialData, descriptor.initialDataSize);
			vkUnmapMemory(m_vkDevice, stagingMemory);

			// Setup buffer copy regions for each mip level
			CrVector<VkBufferImageCopy> bufferCopyRegions;

			for (uint32_t mip = 0; mip < m_mipmapCount; mip++)
			{
				VkBufferImageCopy bufferCopyRegion;
				bufferCopyRegion.imageSubresource.aspectMask = m_vkAspectMask;
				bufferCopyRegion.imageSubresource.mipLevel = mip;
				bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
				bufferCopyRegion.imageSubresource.layerCount = 1;
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
			subresourceRange.layerCount = 1; // TODO

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

			// TODO Rework how this all works. We shouldn't be stalling here or creating new command buffers.
			// However, changing this requires more framework to be in place
			CrCommandBufferSharedHandle cmdBuffer = renderDevice->GetMainCommandQueue()->CreateCommandBuffer();
			CrCommandBufferVulkan* vulkanCmdBuffer = static_cast<CrCommandBufferVulkan*>(cmdBuffer.get());
			cmdBuffer->Begin();

			// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition 
			// Source pipeline stage is host write/read execution (VK_PIPELINE_STAGE_HOST_BIT)
			// Destination pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
			vkCmdPipelineBarrier(vulkanCmdBuffer->GetVkCommandBuffer(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			// Copy mip levels from staging buffer
			vkCmdCopyBufferToImage(vulkanCmdBuffer->GetVkCommandBuffer(), stagingBuffer, m_vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());

			// Once the data has been uploaded we transfer to the texture image to the shader read layout, so it can be sampled from
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition 
			// Source pipeline stage stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
			// Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
			vkCmdPipelineBarrier(vulkanCmdBuffer->GetVkCommandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			cmdBuffer->End();
			cmdBuffer->Submit();
			renderDevice->GetMainCommandQueue()->WaitIdle();

			// Clean up staging resources
			vkFreeMemory(m_vkDevice, stagingMemory, nullptr);
			vkDestroyBuffer(m_vkDevice, stagingBuffer, nullptr);
		}
		else if (m_usage & cr3d::TextureUsage::CPUReadable)
		{
			void* data;
			result = vkMapMemory(m_vkDevice, m_vkMemory, 0, imageMemoryRequirements.size, 0, &data);
			CrAssert(result == VK_SUCCESS);
			memcpy(data, descriptor.initialData, imageMemoryRequirements.size);
			vkUnmapMemory(m_vkDevice, m_vkMemory);
		}
	}

	if (IsRenderTarget() || IsDepthStencil() || IsSwapchain())
	{
		// We create a dummy attachment description here that will be useful for creating
		// the framebuffer objects later. This isn't useful per se but can be used as a helper
		// for creating render passes and framebuffers. I think this needs to be deleted.
		m_vkAttachmentDescription = { 0, m_vkFormat, m_vkSamples, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, imageLayout };
	}

	// TODO Review all this once we've implemented renderpasses and framebuffers
	if (IsRenderTarget() || IsDepthStencil())
	{
		VkAttachmentReference attachmentReference = { 0, imageLayout }; // Bind to slot 0 always if this is the only render target

		// Setup a single subpass reference. We do this to create a subpass dependency and an obvious resource transition instead
		// of a vkCmdPipelineBarrier call somewhere later. If need be we could change this to be more explicit
		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		if (IsDepthStencil())
		{
			subpassDescription.colorAttachmentCount = 0;
			subpassDescription.pDepthStencilAttachment = &attachmentReference;
		}
		else
		{
			subpassDescription.colorAttachmentCount = 1;
			subpassDescription.pColorAttachments = &attachmentReference;
		}

		subpassDescription.inputAttachmentCount = 0; // Input attachments can be used to sample from contents of a previous subpass
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.preserveAttachmentCount = 0; // Preserved attachments can be used to loop (and preserve) attachments through subpasses
		subpassDescription.pPreserveAttachments = nullptr;

		if (m_sampleCount == cr3d::SampleCount::S1)
		{
			subpassDescription.pResolveAttachments = nullptr; // Resolve attachments are resolved at the end of a sub pass and can be used for e.g. multi sampling
		}
		else
		{
			// TODO Multisampling
		}		

		VkSubpassDependency dependencies[2];

		// First dependency at the start of the renderpass. Does the transition from final to initial layout
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL; // Source is all commands outside of the renderpass
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; // Signal at the beginning of the renderpass
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		
		if(IsDepthStencil())
		{
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // Signal at early depth stencil TODO correct?
			dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
		else
		{
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Signal at the blending stage
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Second dependency at the end the renderpass
		// Does the transition from the initial to the final layout
		dependencies[1].srcSubpass = 0; // The only subpass we have
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL; // Consumer are all commands outside of the renderpass
		
		if (IsDepthStencil())
		{
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // Signal at early depth stencil TODO correct?
			dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
		else
		{
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Signal at the blending stage
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}

		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &m_vkAttachmentDescription;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = 2;
		renderPassInfo.pDependencies = dependencies;

		result = vkCreateRenderPass(m_vkDevice, &renderPassInfo, nullptr, &m_vkBaseRenderPass);
		CrAssert(result == VK_SUCCESS);

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.pNext = nullptr;
		frameBufferCreateInfo.renderPass = m_vkBaseRenderPass;
		frameBufferCreateInfo.attachmentCount = 1;
		frameBufferCreateInfo.pAttachments = &m_vkImageView;
		frameBufferCreateInfo.width = m_width;
		frameBufferCreateInfo.height = m_height;
		frameBufferCreateInfo.layers = 1;

		result = vkCreateFramebuffer(m_vkDevice, &frameBufferCreateInfo, nullptr, &m_vkBaseFramebuffer);
	}
}

CrTextureVulkan::~CrTextureVulkan()
{
	CrAssert(m_vkImageView);
	CrAssert(m_vkImage);

	// Don't destroy images we don't manage. The swapchain image and memory was handed to us by the OS
	if (m_usage != cr3d::TextureUsage::SwapChain)
	{
		CrAssert(m_vkMemory);
		vkDestroyImage(m_vkDevice, m_vkImage, nullptr);
		vkFreeMemory(m_vkDevice, m_vkMemory, nullptr);
	}

	vkDestroyImageView(m_vkDevice, m_vkImageView, nullptr);

	if (m_additionalTextureViews)
	{
		for (uint32_t mip = 0; mip < m_additionalTextureViews->m_vkImageViewSingleMipAllSlices.size(); ++mip)
		{
			vkDestroyImageView(m_vkDevice, m_additionalTextureViews->m_vkImageViewSingleMipAllSlices[mip], nullptr);
		}

		for (uint32_t mip = 0; mip < m_additionalTextureViews->m_vkImageSingleMipSlice.size(); ++mip)
		{
			for (uint32_t slice = 0; slice < m_additionalTextureViews->m_vkImageSingleMipSlice[mip].size(); ++slice)
			{
				vkDestroyImageView(m_vkDevice, m_additionalTextureViews->m_vkImageSingleMipSlice[mip][slice], nullptr);
			}
		}
	}

	if (m_vkBaseRenderPass)
	{
		vkDestroyRenderPass(m_vkDevice, m_vkBaseRenderPass, nullptr);
		m_vkBaseRenderPass = nullptr;
	}

	if (m_vkBaseFramebuffer)
	{
		vkDestroyFramebuffer(m_vkDevice, m_vkBaseFramebuffer, nullptr);
		m_vkBaseFramebuffer = nullptr;
	}
}

VkFormat CrTextureVulkan::GetVkFormat() const
{
	return m_vkFormat;
}

VkSampleCountFlagBits CrTextureVulkan::GetVkSamples() const
{
	return m_vkSamples;
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

VkImageAspectFlags CrTextureVulkan::GetVkImageAspectFlags() const
{
	return m_vkAspectMask;
}

VkAttachmentDescription CrTextureVulkan::GetVkAttachmentDescription() const
{
	return m_vkAttachmentDescription;
}
