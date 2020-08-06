#include "CrRendering_pch.h"

#include "ICrShader.h" // TODO remove
#include "CrCommandQueue_vk.h"
#include "CrCommandBuffer_vk.h"
#include "CrRenderDevice_vk.h"
#include "CrTexture_vk.h"
#include "CrSampler_vk.h"
#include "CrShaderManager_vk.h"
#include "CrRenderPass_vk.h"
#include "CrFramebuffer_vk.h"

#include "Core/Containers/CrArray.h"

#include "Core/Logging/ICrDebug.h"

CrCommandBufferVulkan::CrCommandBufferVulkan(ICrCommandQueue* commandQueue)
	: ICrCommandBuffer(commandQueue), m_vkCommandBuffer(nullptr)
{
	// Command buffer device same as command queue device
	CrCommandQueueVulkan* commandQueueVulkan = static_cast<CrCommandQueueVulkan*>(commandQueue);
	m_vkDevice = commandQueueVulkan->GetVkDevice();

	// We need to tell the API the number of max. requested descriptors per type
	CrArray<VkDescriptorPoolSize, 5> typeCounts;

	typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	typeCounts[0].descriptorCount = 10000;

	typeCounts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	typeCounts[1].descriptorCount = 10000;

	typeCounts[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	typeCounts[2].descriptorCount = 10000;

	typeCounts[3].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	typeCounts[3].descriptorCount = 10000;

	typeCounts[4].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	typeCounts[4].descriptorCount = 10000;

	// For additional types you need to add new entries in the type count list
	// typeCounts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	// typeCounts[1].descriptorCount = 2;

	// Create the global descriptor pool
	// All descriptors used in this example are allocated from this pool
	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.pNext = nullptr;
	descriptorPoolInfo.poolSizeCount = (uint32_t)typeCounts.size();
	descriptorPoolInfo.pPoolSizes = typeCounts.data();
	//descriptorPoolInfo.maxSets Affects amount of CPU memory Vulkan Fast Paths, Timothy Lottes

	// Set the max. number of sets that can be requested
	// Requesting descriptors beyond maxSets will result in an error
	descriptorPoolInfo.maxSets = 10000; // TODO as part of constructor with sensible defaults

	VkResult result = VK_SUCCESS;
		
	result = vkCreateDescriptorPool(m_vkDevice, &descriptorPoolInfo, nullptr, &m_vkDescriptorPool);
	CrAssert(result == VK_SUCCESS);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandQueueVulkan->GetVkCommandBufferPool();
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;

	result = vkAllocateCommandBuffers(m_vkDevice, &commandBufferAllocateInfo, &m_vkCommandBuffer);
	CrAssert(result == VK_SUCCESS);
}

CrCommandBufferVulkan::~CrCommandBufferVulkan()
{
	CrCommandQueueVulkan* commandQueueVulkan = static_cast<CrCommandQueueVulkan*>(m_ownerCommandQueue);

	vkDestroyDescriptorPool(m_vkDevice, m_vkDescriptorPool, nullptr);

	vkFreeCommandBuffers(m_vkDevice, commandQueueVulkan->GetVkCommandBufferPool(), 1, &m_vkCommandBuffer);
}

void CrCommandBufferVulkan::UpdateResourceTablesPS()
{
	const ICrGraphicsPipeline* currentPipeline = m_currentState.m_graphicsPipeline;
	const CrGraphicsShaderHandle& currentGraphicsShader = currentPipeline->m_shader;
	const CrShaderResourceSet& resourceSet = currentGraphicsShader->GetResourceSet();

	// 1. Allocate an available descriptor set for this drawcall and update it
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo;
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.pNext = nullptr;
	descriptorSetAllocInfo.descriptorPool = m_vkDescriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &resourceSet.m_vkDescriptorSetLayout;

	VkDescriptorSet descriptorSet;
	VkResult result = vkAllocateDescriptorSets(m_vkDevice, &descriptorSetAllocInfo, &descriptorSet);
	CrAssert(result == VK_SUCCESS);

	// 2. Get current resources and update the descriptor set
	CrArray<VkDescriptorBufferInfo, 64> bufferInfos;
	CrArray<VkDescriptorImageInfo, 64> imageInfos;
	CrArray<VkWriteDescriptorSet, 64> writeDescriptorSets;
	CrArray<uint32_t, 64> offsets;

	uint32_t descriptorCount = 0;
	uint32_t bufferCount = 0;
	uint32_t imageCount = 0;

	for(const CrShaderStageInfo& shaderStage : currentGraphicsShader->m_shaderStages)
	{
		cr3d::ShaderStage::T stage = shaderStage.m_stage;

		uint32_t constantBufferCount = resourceSet.GetConstantBufferCount(stage);

		for (uint32_t i = 0; i < constantBufferCount; ++i)
		{
			const ConstantBuffers::T id = resourceSet.GetConstantBufferID(stage, i);
			const bindpoint_t bindPoint = resourceSet.GetConstantBufferBindPoint(stage, i);
			const ConstantBufferMetadata& constantBufferMetadata = CrShaderManagerVulkan::GetConstantBufferMetadata(id);
			const ConstantBufferBinding& binding = m_currentState.m_constantBuffers[stage][constantBufferMetadata.id];
			const CrHardwareGPUBufferVulkan* vulkanGPUBuffer = static_cast<const CrHardwareGPUBufferVulkan*>(binding.buffer);

			// There are two ways to set buffers in Vulkan, a descriptor offset and a dynamic offset. Both are equivalent
			// in terms of functionality
			// TODO There is a limit to the offset 
			VkDescriptorBufferInfo& bufferInfo = bufferInfos[bufferCount];
			bufferInfo.buffer = vulkanGPUBuffer->GetVkBuffer();
			bufferInfo.offset = 0; // Buffer type is DYNAMIC so offset = 0 (this offset is actually taken into account so would be baseAddress + offset + dynamicOffset)
			bufferInfo.range = (VkDeviceSize)constantBufferMetadata.size;

			offsets[bufferCount] = binding.byteOffset;
			
			writeDescriptorSets[descriptorCount] = crvk::CreateVkWriteDescriptorSet(descriptorSet, bindPoint, 0, 1,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, nullptr, &bufferInfo, nullptr);
			
			descriptorCount++;
			bufferCount++;
		}

		uint32_t textureCount = resourceSet.GetTextureCount(stage);
	
		for (uint32_t i = 0; i < textureCount; ++i)
		{
			Textures::T id = resourceSet.GetTextureID(stage, i);
			bindpoint_t bindPoint = resourceSet.GetTextureBindPoint(stage, i);
			const TextureMetadata& textureMeta = CrShaderManagerVulkan::GetTextureMetadata(id);
			const CrTextureVulkan* vulkanTexture = static_cast<const CrTextureVulkan*>(m_currentState.m_textures[stage][textureMeta.id]);
	
			VkDescriptorImageInfo& imageInfo = imageInfos[imageCount];
			imageInfo.imageView = vulkanTexture->GetVkImageViewAllMipsSlices();
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.sampler = nullptr;

			writeDescriptorSets[descriptorCount] = crvk::CreateVkWriteDescriptorSet(descriptorSet, bindPoint, 0, 1, 
				VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, &imageInfo, nullptr, nullptr);

			descriptorCount++;
			imageCount++;
		}

		uint32_t samplerCount = resourceSet.GetSamplerCount(stage);

		for (uint32_t i = 0; i < samplerCount; ++i)
		{
			Samplers::T id = resourceSet.GetSamplerID(stage, i);
			bindpoint_t bindPoint = resourceSet.GetSamplerBindPoint(stage, i);
			const SamplerMetadata& samplerMeta = CrShaderManagerVulkan::GetSamplerMetadata(id);
			const CrSamplerVulkan* vulkanSampler = static_cast<const CrSamplerVulkan*>(m_currentState.m_samplers[stage][samplerMeta.id]);

			VkDescriptorImageInfo& imageInfo = imageInfos[imageCount];
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageInfo.sampler = vulkanSampler->GetVkSampler();

			writeDescriptorSets[descriptorCount] = crvk::CreateVkWriteDescriptorSet(descriptorSet, bindPoint, 0, 1, 
				VK_DESCRIPTOR_TYPE_SAMPLER, &imageInfo, nullptr, nullptr);

			descriptorCount++;
			imageCount++;
		}
	}

	CrAssert(descriptorCount < writeDescriptorSets.size());

	vkUpdateDescriptorSets(m_vkDevice, descriptorCount, writeDescriptorSets.data(), 0, nullptr);

	// Bind descriptor sets describing shader binding points
	// TODO The bind point depends on the currently bound shader or pipeline, don't just assume graphics here!
	// TODO We need an abstraction of a resource table, so that we can build it somewhere else, and simply bind it when we need to
	vkCmdBindDescriptorSets(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline->m_pipelineLayout, 
		0, 1, &descriptorSet, bufferCount, offsets.data());
}

void CrCommandBufferVulkan::BeginRenderPassPS(const ICrRenderPass* renderPass, const ICrFramebuffer* frameBuffer, const CrRenderPassBeginParams& renderPassParams)
{
	VkRenderPassBeginInfo renderPassBeginInfo;
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = static_cast<const CrRenderPassVulkan*>(renderPass)->GetVkRenderPass();
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = renderPassParams.drawArea.width;
	renderPassBeginInfo.renderArea.extent.height = renderPassParams.drawArea.height;

	VkClearValue clearValues[2];

	if (renderPassParams.clear)
	{
		renderPassBeginInfo.clearValueCount = 2;

		hlslpp::store(renderPassParams.colorClearValue, clearValues[0].color.float32);

		clearValues[1].depthStencil.depth = renderPassParams.depthClearValue;
		clearValues[1].depthStencil.stencil = renderPassParams.stencilClearValue;

		renderPassBeginInfo.pClearValues = clearValues;
	}
	else
	{
		renderPassBeginInfo.clearValueCount = 0;
		renderPassBeginInfo.pClearValues = nullptr;
	}

	renderPassBeginInfo.framebuffer = static_cast<const CrFramebufferVulkan*>(frameBuffer)->GetVkFramebuffer();

	vkCmdBeginRenderPass(m_vkCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CrCommandBufferVulkan::EndRenderPassPS(const ICrRenderPass* /* renderPass*/)
{
	vkCmdEndRenderPass(m_vkCommandBuffer);
}

void CrCommandBufferVulkan::GetVkImageLayoutAndAccessFlags(bool isDepth, cr3d::ResourceState::T resourceState, VkImageLayout& imageLayout, VkAccessFlags& accessFlags)
{
	imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	accessFlags = 0;

	switch (resourceState)
	{
		case cr3d::ResourceState::Undefined:
			imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			accessFlags = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case cr3d::ResourceState::PreInitialized:
			imageLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
			accessFlags = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case cr3d::ResourceState::RenderTarget:
			imageLayout = isDepth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			accessFlags = isDepth ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT : VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;
		case cr3d::ResourceState::UnorderedAccess:
			imageLayout = isDepth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			accessFlags = isDepth ? (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT) : 
									(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT);
			break;
		case cr3d::ResourceState::ShaderInput:
		case cr3d::ResourceState::PixelShaderInput:
			imageLayout = isDepth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			accessFlags = VK_ACCESS_SHADER_READ_BIT;
			break;
		case cr3d::ResourceState::CopySource:
			imageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			break;
		case cr3d::ResourceState::CopyDestination:
			imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			break;
		case cr3d::ResourceState::Present:
			imageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			break;
		default:
			break;
	}
}

void CrCommandBufferVulkan::TransitionTexturePS(const ICrTexture* texture, cr3d::ResourceState::T initialState, cr3d::ResourceState::T destinationState)
{
	bool isDepthFormat = texture->IsDepth();

	//--------
	// SOURCE
	//--------

	VkAccessFlags srcAccessFlags = 0;
	VkImageLayout srcImageLayout;
	GetVkImageLayoutAndAccessFlags(isDepthFormat, initialState, srcImageLayout, srcAccessFlags);

	//if (initialState == cr3d::ResourceState::Undefined)
	//{
	//	srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	//	srcAccessFlags |= (VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT); // Make sure any writes to the color buffer have been finished
	//}
	//else if (initialState == cr3d::ResourceState::PreInitialized)
	//{
	//	srcImageLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	//	srcAccessFlags |= (VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT); // Make sure any writes to the color buffer have been finished
	//}
	//else if (initialState == cr3d::ResourceState::RenderTarget)
	//{
	//	if (isDepthFormat)
	//	{
	//		srcImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	//		srcAccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	//	}
	//	else
	//	{
	//		srcImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	//		srcAccessFlags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	//	}
	//}
	//else if ((initialState == cr3d::ResourceState::ShaderInput) || (initialState == cr3d::ResourceState::PixelShaderInput))
	//{
	//	if (isDepthFormat)
	//	{
	//		srcImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	//	}
	//	else
	//	{
	//		srcImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	//	}
	//
	//	srcAccessFlags |= VK_ACCESS_SHADER_READ_BIT;
	//}
	//else if (initialState == cr3d::ResourceState::CopySource)
	//{
	//	srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	//}
	//else if (initialState == cr3d::ResourceState::CopyDestination)
	//{
	//	srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	//}
	//else if (initialState == cr3d::ResourceState::Present)
	//{
	//	srcImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	//}

	//-------------
	// DESTINATION
	//-------------

	VkImageLayout dstImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkAccessFlags dstAccessFlags = 0;
	//GetVkImageLayoutAndAccessFlags(isDepthFormat, initialState, dstImageLayout, dstAccessFlags);


	if (destinationState == cr3d::ResourceState::Undefined)
	{
		dstImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		dstAccessFlags |= (VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT); // Make sure any writes to the color buffer have been finished
	}
	else if (destinationState == cr3d::ResourceState::RenderTarget)
	{
		if (isDepthFormat)
		{
			dstImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			dstAccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
		else
		{
			dstImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			dstAccessFlags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}
	}
	else if ((destinationState == cr3d::ResourceState::ShaderInput) || (destinationState == cr3d::ResourceState::PixelShaderInput))
	{
		if (isDepthFormat)
		{
			dstImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		}
		else
		{
			dstImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		dstAccessFlags |= VK_ACCESS_SHADER_READ_BIT;
	}
	else if (destinationState == cr3d::ResourceState::CopySource)
	{
		dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	}
	else if (destinationState == cr3d::ResourceState::CopyDestination)
	{
		dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	}
	else if (destinationState == cr3d::ResourceState::Present)
	{
		dstImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}

	const CrTextureVulkan* vkTexture = static_cast<const CrTextureVulkan*>(texture);

	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

	// Some default values
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	imageMemoryBarrier.oldLayout = srcImageLayout;
	imageMemoryBarrier.newLayout = dstImageLayout;
	imageMemoryBarrier.image = vkTexture->GetVkImage();
	imageMemoryBarrier.subresourceRange.aspectMask = vkTexture->GetVkImageAspectFlags();
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	// 		// Source layouts
	// 		switch (srcImageLayout)
	// 		{
	// 		case VK_IMAGE_LAYOUT_UNDEFINED: // Only allowed as initial layout!
	// 			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT; // Make sure any writes to the image have been finished
	// 			break;
	// 		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
	// 			imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Make sure any writes to the color buffer have been finished
	// 			break;
	// 		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
	// 			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT; // Make sure any reads from the image have been finished
	// 			break;
	// 		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
	// 			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT; // Make sure any shader reads from the image have been finished
	// 			break;
	// 		}
	// 	
	// 		// Target layouts
	// 		switch (dstImageLayout)
	// 		{
	// 		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: // New layout is transfer destination (copy, blit)
	// 			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // Make sure any copies to the image have been finished
	// 			break;
	// 		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: // New layout is transfer source (copy, blit)
	// 			imageMemoryBarrier.srcAccessMask = imageMemoryBarrier.srcAccessMask | VK_ACCESS_TRANSFER_READ_BIT;
	// 			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT; // Make sure any reads from and writes to the image have been finished
	// 			break;
	// 		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: // New layout is color attachment
	// 			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Make sure any writes to the color buffer have been finished
	// 			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	// 			break;
	// 		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: // New layout is depth attachment
	// 			imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; // Make sure any writes to depth/stencil buffer have been finished
	// 			break;
	// 		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: // New layout is shader read (sampler, input attachment)
	// 			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT; // Make sure any writes to the image have been finished
	// 			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	// 			break;
	// 		}

	// Put barrier on top
	VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags destStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier(m_vkCommandBuffer, srcStageFlags, destStageFlags, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

void CrCommandBufferVulkan::BeginPS()
{
	CrAssertMsg(m_vkCommandBuffer != nullptr, "Called Begin() on a null command buffer!");

	// Reset the descriptor pool on a Begin(). The same command buffer can never be used more than once per frame, so resetting here makes sense. All resources are sent
	// back to the descriptor pool. Beware that the validation layer has a memory leak: https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/236
	vkResetDescriptorPool(m_vkDevice, m_vkDescriptorPool, 0);

	VkCommandBufferBeginInfo commandBufferInfo;
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferInfo.pNext = nullptr;
	commandBufferInfo.flags = 0;
	commandBufferInfo.pInheritanceInfo = nullptr;

	VkResult result = vkBeginCommandBuffer(m_vkCommandBuffer, &commandBufferInfo);
	CrAssert(result == VK_SUCCESS);
}

void CrCommandBufferVulkan::ClearRenderTargetPS(const ICrTexture* renderTarget, const float4& color, uint32_t level, uint32_t slice, uint32_t levelCount, uint32_t sliceCount)
{
	VkClearColorValue clearColor = { color.r, color.g, color.b, color.a };
	VkImageSubresourceRange imageSubResourceRange = {};
	imageSubResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageSubResourceRange.baseMipLevel = level;
	imageSubResourceRange.levelCount = levelCount;

	imageSubResourceRange.baseArrayLayer = slice;
	imageSubResourceRange.layerCount = sliceCount;

	// TODO Image layout needs to be correct
	vkCmdClearColorImage(m_vkCommandBuffer, static_cast<const CrTextureVulkan*>(renderTarget)->GetVkImage(), VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &imageSubResourceRange);
}

void CrCommandBufferVulkan::EndPS()
{
	CrAssertMsg(m_vkCommandBuffer != nullptr, "Called End() on a null command buffer!");

	VkResult result = vkEndCommandBuffer(m_vkCommandBuffer);
	CrAssert(result == VK_SUCCESS);
}