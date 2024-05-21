#include "Rendering/CrRendering_pch.h"

#include "CrCommandBuffer_vk.h"
#include "CrRenderDevice_vk.h"
#include "CrTexture_vk.h"
#include "CrSampler_vk.h"
#include "CrShader_vk.h"
#include "CrGPUBuffer_vk.h"
#include "Rendering/CrRenderPassDescriptor.h"
#include "Rendering/CrShaderResourceMetadata.h"
#include "Rendering/CrRendering.h"

#include "Core/Containers/CrArray.h"
#include "Core/Logging/ICrDebug.h"

CrCommandBufferVulkan::CrCommandBufferVulkan(CrRenderDeviceVulkan* vulkanRenderDevice, const CrCommandBufferDescriptor& descriptor)
	: ICrCommandBuffer(vulkanRenderDevice, descriptor)
{
	VkDevice vkDevice = vulkanRenderDevice->GetVkDevice();

	// We need to tell the API the number of max. requested descriptors per type
	CrArray<VkDescriptorPoolSize, 4> typeCounts;

	typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	typeCounts[0].descriptorCount = 64;

	typeCounts[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	typeCounts[1].descriptorCount = 6400;

	typeCounts[2].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	typeCounts[2].descriptorCount = 64;

	typeCounts[3].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	typeCounts[3].descriptorCount = 64;

	// Create the global descriptor pool
	// All descriptors used in this example are allocated from this pool
	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.pNext = nullptr;
	descriptorPoolInfo.poolSizeCount = (uint32_t)typeCounts.size();
	descriptorPoolInfo.pPoolSizes = typeCounts.data();
	// The command buffer pool should not have the VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT flag,
	// because the pool gets reset in block with every command buffer Begin()

	// Set the max. number of sets that can be requested
	// Requesting descriptors beyond maxSets will result in an error
	// Affects amount of CPU memory Vulkan Fast Paths, Timothy Lottes
	descriptorPoolInfo.maxSets = 10000; // TODO as part of constructor with sensible defaults

	VkResult result = VK_SUCCESS;
		
	result = vkCreateDescriptorPool(vkDevice, &descriptorPoolInfo, nullptr, &m_vkDescriptorPool);
	CrAssert(result == VK_SUCCESS);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = vulkanRenderDevice->GetVkCommandPool(descriptor.queueType);
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;

	result = vkAllocateCommandBuffers(vkDevice, &commandBufferAllocateInfo, &m_vkCommandBuffer);
	CrAssert(result == VK_SUCCESS);

	vulkanRenderDevice->SetVkObjectName((uint64_t)m_vkCommandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, descriptor.name.c_str());

	// Set up render pass allocation resources. Each command buffer manages their own render passes

	m_renderPassAllocator.Initialize(2 * 1024 * 1024); // 2 MB
	
	m_usedRenderPasses.reserve(128);

	static auto renderPassAllocationFn = [](void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope /*allocationScope*/) -> void*
	{
		CrCommandBufferVulkan* vulkanCommandBuffer = (CrCommandBufferVulkan*)pUserData;
		return vulkanCommandBuffer->m_renderPassAllocator.AllocateAligned(size, alignment).memory;
	};

	// We don't free because we reuse the memory every time the command buffer is reset
	static auto renderPassFreeFn = [](void* /*pUserData*/, void* /*pMemory*/) {};

	static auto renderPassReallocationFn = [](void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope /*allocationScope*/) -> void*
	{
		CrCommandBufferVulkan* vulkanCommandBuffer = (CrCommandBufferVulkan*)pUserData;
		void* memory = vulkanCommandBuffer->m_renderPassAllocator.AllocateAligned(size, alignment).memory;
		memcpy(memory, pOriginal, size);
		return memory;
	};

	m_renderPassAllocationCallbacks.pUserData = this;
	m_renderPassAllocationCallbacks.pfnAllocation = renderPassAllocationFn;
	m_renderPassAllocationCallbacks.pfnFree = renderPassFreeFn;
	m_renderPassAllocationCallbacks.pfnReallocation = renderPassReallocationFn;
}

CrCommandBufferVulkan::~CrCommandBufferVulkan()
{
	CrRenderDeviceVulkan* vulkanRenderDevice = static_cast<CrRenderDeviceVulkan*>(m_renderDevice);

	vkDestroyDescriptorPool(vulkanRenderDevice->GetVkDevice(), m_vkDescriptorPool, nullptr);

	vkFreeCommandBuffers(vulkanRenderDevice->GetVkDevice(), vulkanRenderDevice->GetVkCommandPool(m_queueType), 1, &m_vkCommandBuffer);
}

// TODO This should become CreateShaderResourceTable and should be cached, reused, etc
// The ShaderResourceTable is a collection of resources that can be instantly bound to
// a shader without any checks or validation of any sort. This needs to be thought out
// properly because if these tables persist across frames, they should have the same
// lifetime as the resources they contain
void CrCommandBufferVulkan::UpdateResourceTableVulkan
(const ICrShaderBindingLayout& bindingLayout, VkPipelineBindPoint vkPipelineBindPoint, VkDescriptorSetLayout vkDescriptorSetLayout, VkPipelineLayout vkPipelineLayout)
{
	CrRenderDeviceVulkan* vulkanRenderDevice = static_cast<CrRenderDeviceVulkan*>(m_renderDevice);

	// 1. Allocate an available descriptor set for this drawcall and update it
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo;
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.pNext = nullptr;
	descriptorSetAllocInfo.descriptorPool = m_vkDescriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &vkDescriptorSetLayout;

	VkDescriptorSet descriptorSet;
	VkResult result = vkAllocateDescriptorSets(vulkanRenderDevice->GetVkDevice(), &descriptorSetAllocInfo, &descriptorSet);
	CrAssert(result == VK_SUCCESS);

	// 2. Get current resources and update the descriptor set
	CrArray<VkDescriptorBufferInfo, 64> bufferInfos;
	CrArray<VkDescriptorImageInfo, 64>  imageInfos;
	CrArray<VkBufferView, 64>           bufferViews;
	CrArray<VkWriteDescriptorSet, 64>   writeDescriptorSets;
	CrArray<uint32_t, 64>               dynamicOffsets;

	uint32_t descriptorCount = 0;    // Total number of descriptors
	uint32_t bufferCount = 0;        // Total number of buffers (constant + storage)
	uint32_t dynamicOffsetCount = 0; // Total number of dynamic offsets
	uint32_t imageCount = 0;         // Total number of images
	uint32_t texelBufferCount = 0;   // Total number of texel buffers

	bindingLayout.ForEachConstantBuffer([&](cr3d::ShaderStage::T, ConstantBuffers::T id, bindpoint_t bindPoint)
	{
		const ConstantBufferBinding& binding = m_currentState.m_constantBuffers[id];
		const CrHardwareGPUBufferVulkan* vulkanGPUBuffer = static_cast<const CrHardwareGPUBufferVulkan*>(binding.buffer);

		// There are two ways to set buffers in Vulkan, a descriptor offset and a dynamic offset. Both are equivalent
		// in terms of functionality
		// TODO There is a limit to the offset 
		VkDescriptorBufferInfo& bufferInfo = bufferInfos[bufferCount];
		bufferInfo.buffer = vulkanGPUBuffer->GetVkBuffer();
		bufferInfo.offset = 0; // Buffer type is DYNAMIC so offset = 0 (this offset is actually taken into account so would be baseAddress + offset + dynamicOffset)
		bufferInfo.range = (VkDeviceSize)binding.sizeBytes;

		dynamicOffsets[dynamicOffsetCount] = binding.offsetBytes;
		dynamicOffsetCount++;

		writeDescriptorSets[descriptorCount] = crvk::CreateVkWriteDescriptorSet(descriptorSet, bindPoint, 0, 1,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, nullptr, &bufferInfo, nullptr);

		descriptorCount++;
		bufferCount++;
	});

	bindingLayout.ForEachSampler([&](cr3d::ShaderStage::T, Samplers::T id, bindpoint_t bindPoint)
	{
		const CrSamplerVulkan* vulkanSampler = static_cast<const CrSamplerVulkan*>(m_currentState.m_samplers[id]);

		VkDescriptorImageInfo& imageInfo = imageInfos[imageCount];
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageInfo.sampler = vulkanSampler->GetVkSampler();

		writeDescriptorSets[descriptorCount] = crvk::CreateVkWriteDescriptorSet(descriptorSet, bindPoint, 0, 1,
			VK_DESCRIPTOR_TYPE_SAMPLER, &imageInfo, nullptr, nullptr);

		descriptorCount++;
		imageCount++;
	});

	bindingLayout.ForEachTexture([&](cr3d::ShaderStage::T, Textures::T id, bindpoint_t bindPoint)
	{
		const TextureBinding& textureBinding = m_currentState.m_textures[id];
		const CrTextureVulkan* vulkanTexture = static_cast<const CrTextureVulkan*>(textureBinding.texture);

		VkDescriptorImageInfo& imageInfo = imageInfos[imageCount];

		if (textureBinding.plane == cr3d::TexturePlane::Stencil)
		{
			imageInfo.imageView = vulkanTexture->GetVkImageViewStencil();
		}
		else
		{
			imageInfo.imageView = vulkanTexture->GetVkImageViewShaderAllMipsAllSlices();
		}

		// TODO Handle when image is bound to depth stencil and also as shader input
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.sampler = nullptr;

		writeDescriptorSets[descriptorCount] = crvk::CreateVkWriteDescriptorSet(descriptorSet, bindPoint, 0, 1,
			VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, &imageInfo, nullptr, nullptr);

		descriptorCount++;
		imageCount++;
	});

	bindingLayout.ForEachRWTexture([&](cr3d::ShaderStage::T, RWTextures::T id, bindpoint_t bindPoint)
	{
		const RWTextureBinding& binding = m_currentState.m_rwTextures[id];
		const CrTextureVulkan* vulkanTexture = static_cast<const CrTextureVulkan*>(binding.texture);

		VkDescriptorImageInfo& imageInfo = imageInfos[imageCount];
		imageInfo.imageView = vulkanTexture->GetVkImageViewSingleMipAllSlices(binding.mip);
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageInfo.sampler = nullptr;

		writeDescriptorSets[descriptorCount] = crvk::CreateVkWriteDescriptorSet(descriptorSet, bindPoint, 0, 1,
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &imageInfo, nullptr, nullptr);

		descriptorCount++;
		imageCount++;
	});

	bindingLayout.ForEachStorageBuffer([&](cr3d::ShaderStage::T, StorageBuffers::T id, bindpoint_t bindPoint)
	{
		const StorageBufferBinding& binding = m_currentState.m_storageBuffers[id];
		const CrHardwareGPUBufferVulkan* vulkanGPUBuffer = static_cast<const CrHardwareGPUBufferVulkan*>(binding.buffer);

		VkDescriptorBufferInfo& bufferInfo = bufferInfos[bufferCount];
		bufferInfo.buffer = vulkanGPUBuffer->GetVkBuffer();
		bufferInfo.offset = binding.offsetBytes;
		bufferInfo.range = (VkDeviceSize)binding.sizeBytes;

		writeDescriptorSets[descriptorCount] = crvk::CreateVkWriteDescriptorSet(descriptorSet, bindPoint, 0, 1, 
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &bufferInfo, nullptr);

		descriptorCount++;
		bufferCount++;
	});

	bindingLayout.ForEachRWStorageBuffer([&](cr3d::ShaderStage::T, RWStorageBuffers::T id, bindpoint_t bindPoint)
	{
		const StorageBufferBinding& binding = m_currentState.m_rwStorageBuffers[id];
		const CrHardwareGPUBufferVulkan* vulkanGPUBuffer = static_cast<const CrHardwareGPUBufferVulkan*>(binding.buffer);
	
		VkDescriptorBufferInfo& bufferInfo = bufferInfos[bufferCount];
		bufferInfo.buffer = vulkanGPUBuffer->GetVkBuffer();
		bufferInfo.offset = binding.offsetBytes;
		bufferInfo.range = (VkDeviceSize)binding.sizeBytes;
	
		writeDescriptorSets[descriptorCount] = crvk::CreateVkWriteDescriptorSet
		(descriptorSet, bindPoint, 0, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr, &bufferInfo, nullptr);

		descriptorCount++;
		bufferCount++;
	});

	bindingLayout.ForEachRWTypedBuffer([&](cr3d::ShaderStage::T, RWTypedBuffers::T id, bindpoint_t bindPoint)
	{
		const CrHardwareGPUBufferVulkan* vulkanGPUBuffer = static_cast<const CrHardwareGPUBufferVulkan*>(m_currentState.m_rwTypedBuffers[id].buffer);

		bufferViews[texelBufferCount] = vulkanGPUBuffer->GetVkBufferView();

		writeDescriptorSets[descriptorCount] = crvk::CreateVkWriteDescriptorSet(descriptorSet, bindPoint, 0, 1,
			VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, nullptr, nullptr, &bufferViews[texelBufferCount]);

		descriptorCount++;
		texelBufferCount++;
	});

	CrAssert(descriptorCount < writeDescriptorSets.size());

	vkUpdateDescriptorSets(vulkanRenderDevice->GetVkDevice(), descriptorCount, writeDescriptorSets.data(), 0, nullptr);

	// Bind descriptor sets describing shader binding points
	// TODO We need an abstraction of a resource table, so that we can build it somewhere else, and simply bind it when we need to
	vkCmdBindDescriptorSets(m_vkCommandBuffer, vkPipelineBindPoint, vkPipelineLayout, 0, 1, &descriptorSet, dynamicOffsetCount, dynamicOffsets.data());
}

void CrCommandBufferVulkan::ResetGPUQueriesPS(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count)
{
	const CrGPUQueryPoolVulkan* vulkanQueryPool = static_cast<const CrGPUQueryPoolVulkan*>(queryPool);
	vkCmdResetQueryPool(m_vkCommandBuffer, vulkanQueryPool->GetVkQueryPool(), start, count);
}

void CrCommandBufferVulkan::ResolveGPUQueriesPS(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count)
{
	const CrGPUQueryPoolVulkan* vulkanQueryPool = static_cast<const CrGPUQueryPoolVulkan*>(queryPool);
	const CrHardwareGPUBufferVulkan* vulkanGPUBuffer = static_cast<const CrHardwareGPUBufferVulkan*>(vulkanQueryPool->GetResultsBuffer());

	// The wait flag here doesn't wait on the CPU, rather the GPU will ensure query results are all ready before resolving. In practice this means
	// it won't try to reorder the copy on the GPU to a point before the query was actually finished
	vkCmdCopyQueryPoolResults(m_vkCommandBuffer, vulkanQueryPool->GetVkQueryPool(), start, count, vulkanGPUBuffer->GetVkBuffer(), 0, vulkanQueryPool->GetQuerySize(), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
}

void CrCommandBufferVulkan::FlushGraphicsRenderStatePS()
{
	const CrGraphicsPipelineVulkan* vulkanGraphicsPipeline = static_cast<const CrGraphicsPipelineVulkan*>(m_currentState.m_graphicsPipeline);
	const CrGraphicsShaderHandle& graphicsShader = vulkanGraphicsPipeline->GetShader();
	const CrGraphicsShaderVulkan* vulkanGraphicsShader = static_cast<CrGraphicsShaderVulkan*>(graphicsShader.get());
	
	if (m_currentState.m_indexBufferDirty)
	{
		const CrHardwareGPUBufferVulkan* vulkanGPUBuffer = static_cast<const CrHardwareGPUBufferVulkan*>(m_currentState.m_indexBuffer);
		VkIndexType indexType = crvk::GetVkIndexType(m_currentState.m_indexBufferFormat);
		vkCmdBindIndexBuffer(m_vkCommandBuffer, vulkanGPUBuffer->GetVkBuffer(), m_currentState.m_indexBufferOffset, indexType);
		m_currentState.m_indexBufferDirty = false;
	}

	if (m_currentState.m_vertexBufferDirty)
	{
		if (vulkanGraphicsPipeline->GetVertexStreamCount() > 0)
		{
			VkDeviceSize vkOffsets[cr3d::MaxVertexStreams];
			VkBuffer vkBuffers[cr3d::MaxVertexStreams];

			uint32_t usedVertexStreamCount = vulkanGraphicsPipeline->GetVertexStreamCount();

			for (uint32_t streamId = 0; streamId < usedVertexStreamCount; ++streamId)
			{
				const VertexBufferBinding& binding = m_currentState.m_vertexBuffers[streamId];
				vkOffsets[streamId] = binding.offset;
				vkBuffers[streamId] = static_cast<const CrHardwareGPUBufferVulkan*>(binding.vertexBuffer)->GetVkBuffer();
			}

			// Assume we always start at binding 0
			vkCmdBindVertexBuffers(m_vkCommandBuffer, 0, usedVertexStreamCount, vkBuffers, vkOffsets);
		}

		m_currentState.m_vertexBufferDirty = false;
	}

	if (m_currentState.m_scissorDirty)
	{
		const CrRectangle& scissor = m_currentState.m_scissor;
		VkRect2D vkRect2D = { { (int32_t)scissor.x, (int32_t)scissor.y }, { scissor.width, scissor.height } };
		vkCmdSetScissor(m_vkCommandBuffer, 0, 1, &vkRect2D);
		m_currentState.m_scissorDirty = false;
	}

	if (m_currentState.m_viewportDirty)
	{
		const CrViewport& viewport = m_currentState.m_viewport;

		// TODO Be able to set multiple viewports
		VkViewport vkViewport =
		{
			viewport.x,
			viewport.y + viewport.height,
			viewport.width,
			-viewport.height, // Requires VK_KHR_maintenance1. For easier compatibility with D3D
			viewport.minDepth,
			viewport.maxDepth
		};

		vkCmdSetViewport(m_vkCommandBuffer, 0, 1, &vkViewport);

		m_currentState.m_viewportDirty = false;
	}

	if (m_currentState.m_graphicsPipelineDirty)
	{
		// In Vulkan we specify the type of pipeline. In DX12 for instance they are separate objects
		vkCmdBindPipeline(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<const CrGraphicsPipelineVulkan*>(m_currentState.m_graphicsPipeline)->GetVkPipeline());
		m_currentState.m_graphicsPipelineDirty = false;
	}

	if (m_currentState.m_stencilRefDirty)
	{
		vkCmdSetStencilReference(m_vkCommandBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, m_currentState.m_stencilRef);
		m_currentState.m_stencilRefDirty = false;
	}

	UpdateResourceTableVulkan(graphicsShader->GetBindingLayout(), VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanGraphicsShader->GetVkDescriptorSetLayout(), vulkanGraphicsPipeline->GetVkPipelineLayout());
}

void CrCommandBufferVulkan::FlushComputeRenderStatePS()
{
	const CrComputePipelineVulkan* vulkanComputePipeline = static_cast<const CrComputePipelineVulkan*>(m_currentState.m_computePipeline);
	const CrComputeShaderHandle& computeShader = vulkanComputePipeline->GetShader();
	const CrComputeShaderVulkan* vulkanComputeShader = static_cast<CrComputeShaderVulkan*>(computeShader.get());

	if (m_currentState.m_computePipelineDirty)
	{
		// In Vulkan we specify the type of pipeline. In D3D12 for instance they are separate objects
		vkCmdBindPipeline(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, static_cast<const CrComputePipelineVulkan*>(m_currentState.m_computePipeline)->GetVkPipeline());
		m_currentState.m_computePipelineDirty = false;
	}

	UpdateResourceTableVulkan(computeShader->GetBindingLayout(), VK_PIPELINE_BIND_POINT_COMPUTE, vulkanComputeShader->GetVkDescriptorSetLayout(), vulkanComputePipeline->GetVkPipelineLayout());
}

static VkAttachmentDescription GetVkAttachmentDescription(const CrRenderTargetDescriptor& renderTargetDescriptor)
{
	VkAttachmentDescription attachmentDescription;

	attachmentDescription.flags          = 0;
	attachmentDescription.format         = crvk::GetVkFormat(renderTargetDescriptor.texture->GetFormat());
	attachmentDescription.samples        = crvk::GetVkSampleCount(renderTargetDescriptor.texture->GetSampleCount());
	attachmentDescription.loadOp         = crvk::GetVkAttachmentLoadOp(renderTargetDescriptor.loadOp);
	attachmentDescription.storeOp        = crvk::GetVkAttachmentStoreOp(renderTargetDescriptor.storeOp);
	attachmentDescription.stencilLoadOp  = crvk::GetVkAttachmentLoadOp(renderTargetDescriptor.stencilLoadOp);
	attachmentDescription.stencilStoreOp = crvk::GetVkAttachmentStoreOp(renderTargetDescriptor.stencilStoreOp);

	// We don't let the Vulkan render pass try to do transitions as it will do them incorrectly unless there's some subpass
	// magic going on. Instead, we take care of transitions manually and ignore the layouts here. For more information read
	// https://themaister.net/blog/2019/08/14/yet-another-blog-explaining-vulkan-synchronization/
	attachmentDescription.initialLayout  = crvk::GetVkImageStateInfo(renderTargetDescriptor.texture->GetFormat(), renderTargetDescriptor.initialState.layout).imageLayout;
	attachmentDescription.finalLayout    = attachmentDescription.initialLayout;

	return attachmentDescription;
}

void PopulateVkBufferBarrier(VkBufferMemoryBarrier& bufferMemoryBarrier,
	const CrRenderPassBufferDescriptor& bufferDescriptor, cr3d::BufferState::T sourceState, cr3d::BufferState::T destinationState)
{
	const CrHardwareGPUBufferVulkan* vulkanGPUBuffer = static_cast<const CrHardwareGPUBufferVulkan*>(bufferDescriptor.hardwareBuffer);

	bufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	bufferMemoryBarrier.pNext = nullptr;

	bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	bufferMemoryBarrier.srcAccessMask = CrHardwareGPUBufferVulkan::GetVkBufferStateInfo(sourceState).accessMask;
	bufferMemoryBarrier.dstAccessMask = CrHardwareGPUBufferVulkan::GetVkBufferStateInfo(destinationState).accessMask;

	bufferMemoryBarrier.buffer = vulkanGPUBuffer->GetVkBuffer();
	bufferMemoryBarrier.offset = bufferDescriptor.offset;
	bufferMemoryBarrier.size = bufferDescriptor.size;
}

void PopulateVkImageBarrier(VkImageMemoryBarrier& imageMemoryBarrier, const ICrTexture* texture,
	uint32_t mipmapStart, uint32_t mipmapCount, uint32_t sliceStart, uint32_t sliceCount,
	cr3d::TextureLayout::T sourceLayout, cr3d::TextureLayout::T destinationLayout)
{
	const CrTextureVulkan* vulkanTexture = static_cast<const CrTextureVulkan*>(texture);

	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;

	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	const CrVkImageStateInfo& resourceStateInfoSource = crvk::GetVkImageStateInfo(texture->GetFormat(), sourceLayout);
	const CrVkImageStateInfo& resourceStateInfoDestination = crvk::GetVkImageStateInfo(texture->GetFormat(), destinationLayout);

	imageMemoryBarrier.oldLayout = resourceStateInfoSource.imageLayout;
	imageMemoryBarrier.newLayout = resourceStateInfoDestination.imageLayout;
	imageMemoryBarrier.image = vulkanTexture->GetVkImage();
	imageMemoryBarrier.subresourceRange.aspectMask = crvk::GetVkImageAspectFlags(texture->GetFormat());
	imageMemoryBarrier.subresourceRange.baseMipLevel = mipmapStart;
	imageMemoryBarrier.subresourceRange.levelCount = mipmapCount;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = sliceStart;
	imageMemoryBarrier.subresourceRange.layerCount = sliceCount;
	imageMemoryBarrier.srcAccessMask = resourceStateInfoSource.accessMask;
	imageMemoryBarrier.dstAccessMask = resourceStateInfoDestination.accessMask;
}

void CrCommandBufferVulkan::BeginRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor)
{
	VkDevice vkDevice = static_cast<CrRenderDeviceVulkan*>(m_renderDevice)->GetVkDevice();

	// Always process buffers and textures
	GatherImageAndBufferBarriers(renderPassDescriptor.beginBuffers, renderPassDescriptor.beginTextures);

	if(renderPassDescriptor.type == cr3d::RenderPassType::Graphics)
	{
		// Attachment are up to number of render targets, plus depth
		CrFixedVector<VkAttachmentDescription, cr3d::MaxRenderTargets + 1> attachments;
		CrFixedVector<VkImageView, cr3d::MaxRenderTargets + 1> attachmentImageViews;
		CrFixedVector<VkClearValue, cr3d::MaxRenderTargets + 1> clearValues;

		// Attachment references are set in subpasses
		CrFixedVector<VkAttachmentReference, cr3d::MaxRenderTargets> colorReferences;
		VkAttachmentReference depthReference;

		uint32_t numColorAttachments = (uint32_t)renderPassDescriptor.color.size();
		uint32_t numDepthAttachments = 0;

		for (uint32_t i = 0; i < numColorAttachments; ++i)
		{
			const CrRenderTargetDescriptor& colorAttachment = renderPassDescriptor.color[i];
			colorReferences.push_back({ i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			attachmentImageViews.push_back(static_cast<const CrTextureVulkan*>(colorAttachment.texture)->GetVkImageViewSingleMipSlice(colorAttachment.mipmap, colorAttachment.slice));
			attachments.push_back(GetVkAttachmentDescription(colorAttachment));

			if (colorAttachment.initialState.layout != colorAttachment.usageState.layout)
			{
				QueueVkImageBarrier(colorAttachment.texture, colorAttachment.mipmap, 1, colorAttachment.slice, 1, colorAttachment.initialState, colorAttachment.usageState);
			}

			VkClearValue& colorClearValue = clearValues.push_back();
			hlslpp::store(colorAttachment.clearColor, colorClearValue.color.float32);
		}

		if (renderPassDescriptor.depth.texture)
		{
			const CrRenderTargetDescriptor& depthAttachment = renderPassDescriptor.depth;
			const CrTextureVulkan* vulkanTexture = static_cast<const CrTextureVulkan*>(depthAttachment.texture);
			VkAttachmentDescription attachmentDescription = GetVkAttachmentDescription(depthAttachment);

			depthReference = { numColorAttachments, attachmentDescription.initialLayout };
			attachmentImageViews.push_back(vulkanTexture->GetVkImageViewSingleMipSlice(depthAttachment.mipmap, depthAttachment.slice));
			attachments.push_back(attachmentDescription);

			if (depthAttachment.initialState.layout != depthAttachment.usageState.layout)
			{
				QueueVkImageBarrier(depthAttachment.texture, depthAttachment.mipmap, 1, depthAttachment.slice, 1, depthAttachment.initialState, depthAttachment.usageState);
			}

			VkClearValue& depthClearValue = clearValues.push_back();
			depthClearValue.depthStencil.depth = depthAttachment.depthClearValue;
			depthClearValue.depthStencil.stencil = depthAttachment.stencilClearValue;
			numDepthAttachments = 1;
		}

		// All render passes need at least one subpass to work
		VkSubpassDescription subpassDescription;
		subpassDescription.flags                   = 0;
		subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.inputAttachmentCount    = 0;
		subpassDescription.pInputAttachments       = nullptr;
		subpassDescription.colorAttachmentCount    = (uint32_t)colorReferences.size();
		subpassDescription.pColorAttachments       = colorReferences.data();
		subpassDescription.pResolveAttachments     = nullptr;
		subpassDescription.pDepthStencilAttachment = numDepthAttachments > 0 ? &depthReference : nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments    = nullptr;

		// Create the renderpass
		VkRenderPassCreateInfo renderPassInfo;
		renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pNext           = nullptr;
		renderPassInfo.flags           = 0;
		renderPassInfo.attachmentCount = (uint32_t)attachments.size();
		renderPassInfo.pAttachments    = attachments.data();
		renderPassInfo.subpassCount    = 1;
		renderPassInfo.pSubpasses      = &subpassDescription;
		renderPassInfo.dependencyCount = 0;
		renderPassInfo.pDependencies   = nullptr;

		VkRenderPass& vkRenderPass = m_usedRenderPasses.push_back();
		VkResult vkResult = vkCreateRenderPass(vkDevice, &renderPassInfo, &m_renderPassAllocationCallbacks, &vkRenderPass);
		CrAssert(vkResult == VK_SUCCESS);

		uint32_t width = !renderPassDescriptor.color.empty() ? renderPassDescriptor.color[0].texture->GetWidth() : renderPassDescriptor.depth.texture->GetWidth();
		uint32_t height = !renderPassDescriptor.color.empty() ? renderPassDescriptor.color[0].texture->GetHeight() : renderPassDescriptor.depth.texture->GetHeight();

		VkFramebufferCreateInfo frameBufferCreateInfo =
		{
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			nullptr, 0, vkRenderPass, (uint32_t)attachmentImageViews.size(),
			attachmentImageViews.data(), width, height, 1
		};

		VkFramebuffer vkFramebuffer;
		vkResult = vkCreateFramebuffer(vkDevice, &frameBufferCreateInfo, &m_renderPassAllocationCallbacks, &vkFramebuffer);
		CrAssert(vkResult == VK_SUCCESS);

		// Create render pass begin parameters
		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = vkRenderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.framebuffer = vkFramebuffer;
		renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();
		vkCmdBeginRenderPass(m_vkCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}
	else
	{
		FlushImageAndBufferBarriers();
	}
}

void CrCommandBufferVulkan::EndRenderPassPS()
{
	if (m_currentState.m_currentRenderPass.type == cr3d::RenderPassType::Graphics)
	{
		uint32_t numColorAttachments = (uint32_t)m_currentState.m_currentRenderPass.color.size();

		for (uint32_t i = 0; i < numColorAttachments; ++i)
		{
			const CrRenderTargetDescriptor& colorAttachment = m_currentState.m_currentRenderPass.color[i];

			if (colorAttachment.usageState.layout != colorAttachment.finalState.layout)
			{
				QueueVkImageBarrier(colorAttachment.texture, colorAttachment.mipmap, 1, colorAttachment.slice, 1, colorAttachment.usageState, colorAttachment.finalState);
			}
		}

		if (m_currentState.m_currentRenderPass.depth.texture)
		{
			const CrRenderTargetDescriptor& depthAttachment = m_currentState.m_currentRenderPass.depth;

			if (depthAttachment.usageState.layout != depthAttachment.finalState.layout)
			{
				QueueVkImageBarrier(depthAttachment.texture, depthAttachment.mipmap, 1, depthAttachment.slice, 1, depthAttachment.usageState, depthAttachment.finalState);
			}
		}

		vkCmdEndRenderPass(m_vkCommandBuffer);
	}

	GatherImageAndBufferBarriers(m_currentState.m_currentRenderPass.endBuffers, m_currentState.m_currentRenderPass.endTextures);
	FlushImageAndBufferBarriers();
}

// Create the image and buffer barriers for the buffer and texture transitions specified in the arrays
// They're all calculated and batched together for efficiency
void CrCommandBufferVulkan::GatherImageAndBufferBarriers(const CrRenderPassDescriptor::BufferTransitionVector& buffers, const CrRenderPassDescriptor::TextureTransitionVector& textures)
{
	for (const CrRenderPassBufferDescriptor& bufferDescriptor : buffers)
	{
		VkBufferMemoryBarrier& bufferMemoryBarrier = m_bufferMemoryBarriers.push_back();
		PopulateVkBufferBarrier(bufferMemoryBarrier, bufferDescriptor, bufferDescriptor.sourceState, bufferDescriptor.destinationState);
		m_srcStageMask |= CrHardwareGPUBufferVulkan::GetVkPipelineStageFlags(bufferDescriptor.sourceState, bufferDescriptor.sourceShaderStages);
		m_destStageMask |= CrHardwareGPUBufferVulkan::GetVkPipelineStageFlags(bufferDescriptor.destinationState, bufferDescriptor.destinationShaderStages);
	}

	for (const CrRenderPassTextureDescriptor& textureDescriptor : textures)
	{
		QueueVkImageBarrier(textureDescriptor.texture, textureDescriptor.mipmapStart, textureDescriptor.mipmapCount,
			textureDescriptor.sliceStart, textureDescriptor.sliceCount, textureDescriptor.sourceState, textureDescriptor.destinationState);
	}
}

void CrCommandBufferVulkan::QueueVkImageBarrier(const ICrTexture* texture, uint32_t mipmapStart, uint32_t mipmapCount, uint32_t sliceStart, uint32_t sliceCount,
	const cr3d::TextureState& sourceState, const cr3d::TextureState& destinationState)
{
	VkImageMemoryBarrier& imageMemoryBarrier = m_imageMemoryBarriers.push_back();
	PopulateVkImageBarrier(imageMemoryBarrier, texture, mipmapStart, mipmapCount, sliceStart, sliceCount, sourceState.layout, destinationState.layout);
	m_srcStageMask |= crvk::GetVkPipelineStageFlags(sourceState);
	m_destStageMask |= crvk::GetVkPipelineStageFlags(destinationState);
}

void CrCommandBufferVulkan::FlushImageAndBufferBarriers()
{
	if (!m_imageMemoryBarriers.empty() || !m_bufferMemoryBarriers.empty())
	{
		VkDependencyFlags dependencyFlags = 0;
		uint32_t memoryBarrierCount = 0;
		const VkMemoryBarrier* memoryBarriers = nullptr;

		// VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT specifies no stage of execution when specified in the first scope
		// VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT specifies no stage of execution when specified in the second scope
		VkPipelineStageFlags srcStageMask = m_srcStageMask != 0 ? m_srcStageMask : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkPipelineStageFlags destStageMask = m_destStageMask != 0 ? m_destStageMask : VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

		vkCmdPipelineBarrier(
			m_vkCommandBuffer,
			srcStageMask,
			destStageMask,
			dependencyFlags,
			memoryBarrierCount,
			memoryBarriers,
			(uint32_t)m_bufferMemoryBarriers.size(),
			m_bufferMemoryBarriers.data(),
			(uint32_t)m_imageMemoryBarriers.size(),
			m_imageMemoryBarriers.data());

		// Reset state
		m_srcStageMask = VK_PIPELINE_STAGE_NONE;
		m_destStageMask = VK_PIPELINE_STAGE_NONE;
		m_imageMemoryBarriers.clear();
		m_bufferMemoryBarriers.clear();
	}
}

void CrCommandBufferVulkan::BeginPS()
{
	CrAssertMsg(m_vkCommandBuffer != nullptr, "Called Begin() on a null command buffer");

	VkDevice vkDevice = static_cast<CrRenderDeviceVulkan*>(m_renderDevice)->GetVkDevice();

	// Reset the descriptor pool on a Begin(). The same command buffer can never be used more than once per frame, so resetting here makes sense. All resources are sent
	// back to the descriptor pool. Beware that the validation layer has a memory leak: https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/236
	vkResetDescriptorPool(vkDevice, m_vkDescriptorPool, 0);

	VkCommandBufferBeginInfo commandBufferInfo;
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferInfo.pNext = nullptr;
	commandBufferInfo.flags = 0;
	commandBufferInfo.pInheritanceInfo = nullptr;

	// Is this really needed or can I ignore if no validation layers?
	{
		for (VkRenderPass pass : m_usedRenderPasses)
		{
			vkDestroyRenderPass(vkDevice, pass, &m_renderPassAllocationCallbacks);
		}

		for (VkFramebuffer framebuffer : m_usedFramebuffers)
		{
			vkDestroyFramebuffer(vkDevice, framebuffer, &m_renderPassAllocationCallbacks);
		}
	}

	m_usedRenderPasses.clear();
	m_usedFramebuffers.clear();
	m_renderPassAllocator.Reset();

	VkResult result = vkBeginCommandBuffer(m_vkCommandBuffer, &commandBufferInfo);
	CrAssert(result == VK_SUCCESS);
}

void CrCommandBufferVulkan::ClearRenderTargetPS(const ICrTexture* renderTarget, const float4& color, uint32_t mip, uint32_t slice, uint32_t mipCount, uint32_t sliceCount)
{
	VkClearColorValue clearColor;
	store(color, clearColor.float32);

	VkImageSubresourceRange imageSubResourceRange = {};
	imageSubResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageSubResourceRange.baseMipLevel = mip;
	imageSubResourceRange.levelCount = mipCount;

	imageSubResourceRange.baseArrayLayer = slice;
	imageSubResourceRange.layerCount = sliceCount;

	// TODO Image layout needs to be correct
	vkCmdClearColorImage(m_vkCommandBuffer, static_cast<const CrTextureVulkan*>(renderTarget)->GetVkImage(), VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &imageSubResourceRange);
}

void CrCommandBufferVulkan::DrawIndirectPS(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count)
{
	VkBuffer vkIndirectBuffer = static_cast<const CrHardwareGPUBufferVulkan*>(indirectBuffer)->GetVkBuffer();
	vkCmdDrawIndirect(m_vkCommandBuffer, vkIndirectBuffer, offset, count, sizeof(VkDrawIndirectCommand));
}

void CrCommandBufferVulkan::DrawIndexedIndirectPS(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count)
{
	VkBuffer vkIndirectBuffer = static_cast<const CrHardwareGPUBufferVulkan*>(indirectBuffer)->GetVkBuffer();
	vkCmdDrawIndexedIndirect(m_vkCommandBuffer, vkIndirectBuffer, offset, count, sizeof(VkDrawIndexedIndirectCommand));
}

void CrCommandBufferVulkan::DispatchIndirectPS(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset)
{
	VkBuffer vkIndirectBuffer = static_cast<const CrHardwareGPUBufferVulkan*>(indirectBuffer)->GetVkBuffer();
	vkCmdDispatchIndirect(m_vkCommandBuffer, vkIndirectBuffer, offset);
}

void CrCommandBufferVulkan::EndPS()
{
	CrAssertMsg(m_vkCommandBuffer != nullptr, "Called End() on a null command buffer");

	VkResult result = vkEndCommandBuffer(m_vkCommandBuffer);
	CrAssert(result == VK_SUCCESS);
}