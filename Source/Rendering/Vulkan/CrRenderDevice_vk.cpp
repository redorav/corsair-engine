#include "Rendering/CrRendering_pch.h"

#include "CrRenderSystem_vk.h"
#include "CrRenderDevice_vk.h"

#include "CrCommandBuffer_vk.h"
#include "CrTexture_vk.h"
#include "CrSampler_vk.h"
#include "CrSwapchain_vk.h"
#include "CrGPUBuffer_vk.h"
#include "CrGPUSynchronization_vk.h"
#include "CrShader_vk.h"
#include "CrGPUQueryPool_vk.h"

#include "Core/CrCommandLine.h"
#include "Core/Logging/ICrDebug.h"

#include "Math/CrMath.h"

// https://zeux.io/2019/07/17/serializing-pipeline-cache/
// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#pipelines-cache-header
struct VkPipelineCacheHeader
{
	uint32_t dataSize; // Length in bytes of the entire pipeline cache header written as a stream of bytes, with the least significant byte first
	uint32_t version;  // A VkPipelineCacheHeaderVersion value written as a stream of bytes, with the least significant byte first

	uint32_t vendorID; // A vendor ID equal to VkPhysicalDeviceProperties::vendorID written as a stream of bytes, with the least significant byte first
	uint32_t deviceID; // A device ID equal to VkPhysicalDeviceProperties::deviceID written as a stream of bytes, with the least significant byte first

	uint8_t uuid[VK_UUID_SIZE]; // A pipeline cache ID equal to VkPhysicalDeviceProperties::pipelineCacheUUID
};

CrRenderDeviceVulkan::CrRenderDeviceVulkan(const ICrRenderSystem* renderSystem, const CrRenderDeviceDescriptor& descriptor) : ICrRenderDevice(renderSystem, descriptor)
	, m_numCommandQueues(0)
{
	m_vkInstance = static_cast<const CrRenderSystemVulkan*>(renderSystem)->GetVkInstance();

	// 2. Select the physical device (can be multi-GPU)
	// TODO This needs to come from outside as part of the render device creation
	// We select a physical device, and create as many logical devices as we need
	SelectPhysicalDevice();

	// 3. Query queue families
	// TODO This also needs to live in the RenderSystem
	RetrieveQueueFamilies();

	// 4. Create logical device. Connects the physical device to a logical vkDevice.
	// Also specifies desired queues.
	CreateLogicalDevice();

	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.flags                          = 0; // Use to enable via certain extensions
	allocatorCreateInfo.physicalDevice                 = m_vkPhysicalDevice;
	allocatorCreateInfo.device                         = m_vkDevice;
	allocatorCreateInfo.preferredLargeHeapBlockSize    = 0;
	allocatorCreateInfo.pAllocationCallbacks           = nullptr;
	allocatorCreateInfo.pDeviceMemoryCallbacks         = nullptr;
	allocatorCreateInfo.pHeapSizeLimit                 = nullptr;
	allocatorCreateInfo.pVulkanFunctions               = nullptr;
	allocatorCreateInfo.instance                       = m_vkInstance;
	allocatorCreateInfo.vulkanApiVersion               = VK_API_VERSION_1_0;
	allocatorCreateInfo.pTypeExternalMemoryHandleTypes = nullptr;
	
	vmaCreateAllocator(&allocatorCreateInfo, &m_vmaAllocator);

	// 5. Create main command queue. This will take care of the main command buffers and present

	// TODO Rework the queue indices
	uint32_t commandQueueIndex = ReserveVkQueueIndex();
	vkGetDeviceQueue(m_vkDevice, GetVkQueueFamilyIndex(), commandQueueIndex, &m_vkGraphicsQueue);

	// Create a command pool from which the queue can allocate command buffers
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = GetVkQueueFamilyIndex();

	// TODO Apparently it's better to not specify the RESET flag and reset the command pool, but we don't
	// do that at the moment
	// https://www.reddit.com/r/vulkan/comments/5zwfot/whats_your_command_buffer_allocation_strategy/
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VkResult vkResult = vkCreateCommandPool(m_vkDevice, &cmdPoolInfo, nullptr, &m_vkGraphicsCommandPool);
	CrAssert(vkResult == VK_SUCCESS);

	// Load serialized pipeline cache from disk. This pipeline cache is invalid if the uuid doesn't match
	CrVector<char> pipelineCacheData;
	LoadPipelineCache(pipelineCacheData);

	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	if (!pipelineCacheData.empty())
	{
		const VkPipelineCacheHeader& pipelineCacheHeader = reinterpret_cast<VkPipelineCacheHeader&>(*pipelineCacheData.data());
		bool matchesUUID = memcmp(pipelineCacheHeader.uuid, m_vkPhysicalDeviceProperties2.properties.pipelineCacheUUID, VK_UUID_SIZE) == 0;

		if (matchesUUID)
		{
			CrLog("Serialized pipeline cache matches UUID");
			pipelineCacheCreateInfo.pInitialData = pipelineCacheData.data();
			pipelineCacheCreateInfo.initialDataSize = pipelineCacheData.size();
		}
		else
		{
			CrLog("Serialized pipeline cache does not match UUID. Creating empty pipeline cache");
		}
	}

	vkResult = vkCreatePipelineCache(m_vkDevice, &pipelineCacheCreateInfo, nullptr, &m_vkPipelineCache);
	CrAssertMsg(vkResult == VK_SUCCESS, "Failed to create pipeline cache");
}

CrRenderDeviceVulkan::~CrRenderDeviceVulkan()
{
	vmaDestroyAllocator(m_vmaAllocator);

	// Store pipeline cache to disk
	size_t pipelineCacheSize = 0;
	vkGetPipelineCacheData(m_vkDevice, m_vkPipelineCache, &pipelineCacheSize, nullptr);

	CrVector<char> pipelineCacheData(pipelineCacheSize);
	vkGetPipelineCacheData(m_vkDevice, m_vkPipelineCache, &pipelineCacheSize, pipelineCacheData.data());

	StorePipelineCache(pipelineCacheData.data(), pipelineCacheSize);
}

cr3d::GPUFenceResult CrRenderDeviceVulkan::WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds)
{
	VkResult result = vkWaitForFences(m_vkDevice, 1, &static_cast<const CrGPUFenceVulkan*>(fence)->GetVkFence(), true, timeoutNanoseconds);

	switch (result)
	{
		case VK_SUCCESS: return cr3d::GPUFenceResult::Success;
		case VK_TIMEOUT: return cr3d::GPUFenceResult::TimeoutOrNotReady;
		default: return cr3d::GPUFenceResult::Error;
	}
}

cr3d::GPUFenceResult CrRenderDeviceVulkan::GetFenceStatusPS(const ICrGPUFence* fence) const
{
	VkResult result = vkGetFenceStatus(m_vkDevice, static_cast<const CrGPUFenceVulkan*>(fence)->GetVkFence());

	switch (result)
	{
		case VK_SUCCESS: return cr3d::GPUFenceResult::Success;
		case VK_NOT_READY: return cr3d::GPUFenceResult::TimeoutOrNotReady;
		default: return cr3d::GPUFenceResult::Error;
	}
}

void CrRenderDeviceVulkan::SignalFencePS(CrCommandQueueType::T queueType, const ICrGPUFence* fence)
{
	CrAssert(fence != nullptr);

	if (queueType == CrCommandQueueType::Graphics)
	{
		VkResult result = vkQueueSubmit(m_vkGraphicsQueue, 0, nullptr, static_cast<const CrGPUFenceVulkan*>(fence)->GetVkFence());
		CrAssert(result == VK_SUCCESS);
	}
	else
	{
		CrAssertMsg(false, "Not implemented");
	}
}

void CrRenderDeviceVulkan::ResetFencePS(const ICrGPUFence* fence)
{
	vkResetFences(m_vkDevice, 1, &static_cast<const CrGPUFenceVulkan*>(fence)->GetVkFence());
}

void CrRenderDeviceVulkan::WaitIdlePS()
{
	vkDeviceWaitIdle(m_vkDevice);
}

uint8_t* CrRenderDeviceVulkan::BeginTextureUploadPS(const ICrTexture* texture)
{
	uint32_t stagingBufferSizeBytes = texture->GetUsedGPUMemory();

	CrHardwareGPUBufferDescriptor stagingBufferDescriptor(cr3d::BufferUsage::TransferSrc, cr3d::MemoryAccess::StagingUpload, (uint32_t)stagingBufferSizeBytes);
	stagingBufferDescriptor.name = "Texture Upload Staging Buffer";
	CrHardwareGPUBufferHandle stagingBuffer = CreateHardwareGPUBuffer(stagingBufferDescriptor);
	CrHardwareGPUBufferVulkan* vulkanStagingBuffer = static_cast<CrHardwareGPUBufferVulkan*>(stagingBuffer.get());

	CrTextureUpload textureUpload;
	textureUpload.buffer = stagingBuffer;
	textureUpload.mipmapStart = 0;
	textureUpload.mipmapCount = texture->GetMipmapCount();
	textureUpload.sliceStart = 0;
	textureUpload.sliceCount = texture->GetArraySize();

	CrHash textureHash(&texture, sizeof(texture));

	// Add to the open uploads for when we end the texture upload
	m_openTextureUploads.insert({ textureHash, textureUpload });

	return (uint8_t*)vulkanStagingBuffer->Lock();
}

void CrRenderDeviceVulkan::EndTextureUploadPS(const ICrTexture* texture)
{
	CrHash textureHash(&texture, sizeof(texture));
	const auto textureUploadIter = m_openTextureUploads.find(textureHash);
	if (textureUploadIter != m_openTextureUploads.end())
	{
		const CrTextureUpload& textureUpload = textureUploadIter->second;

		const CrTextureVulkan* vulkanTexture = static_cast<const CrTextureVulkan*>(texture);
		CrHardwareGPUBufferVulkan* vulkanStagingBuffer = static_cast<CrHardwareGPUBufferVulkan*>(textureUpload.buffer.get());

		vulkanStagingBuffer->Unlock();

		const CrVkImageStateInfo& vkImageStateInfo = CrTextureVulkan::GetVkImageStateInfo(vulkanTexture->GetDefaultState().layout);

		CrCommandBufferVulkan* vulkanCommandBuffer = static_cast<CrCommandBufferVulkan*>(GetAuxiliaryCommandBuffer().get());
		{
			VkImageAspectFlags vkImageAspectMask = crvk::GetVkImageAspectFlags(vulkanTexture->GetFormat());

			// Transition the texture image layout to transfer target, so we can safely copy our buffer data into it
			VkImageSubresourceRange subresourceRange;
			subresourceRange.aspectMask     = vkImageAspectMask;
			subresourceRange.baseMipLevel   = textureUpload.mipmapStart;
			subresourceRange.levelCount     = textureUpload.mipmapCount;
			subresourceRange.baseArrayLayer = textureUpload.sliceStart;
			subresourceRange.layerCount     = textureUpload.sliceCount;

			VkImageMemoryBarrier imageMemoryBarrier;
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.pNext = nullptr;
			imageMemoryBarrier.image = vulkanTexture->GetVkImage();
			imageMemoryBarrier.subresourceRange = subresourceRange;
			imageMemoryBarrier.srcAccessMask = 0;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.oldLayout = vkImageStateInfo.imageLayout;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition 
			// Source pipeline stage is host write/read execution (VK_PIPELINE_STAGE_HOST_BIT)
			// Destination pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
			vkCmdPipelineBarrier(vulkanCommandBuffer->GetVkCommandBuffer(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

			// Setup buffer copy regions for each mip level
			CrFixedVector<VkBufferImageCopy, cr3d::MaxMipmaps> bufferCopyRegions;
			
			for (uint32_t mip = textureUpload.mipmapStart; mip < textureUpload.mipmapStart + textureUpload.mipmapCount; mip++)
			{
				VkBufferImageCopy bufferCopyRegion;
				bufferCopyRegion.imageSubresource.aspectMask = vkImageAspectMask;
				bufferCopyRegion.imageSubresource.mipLevel = mip;
				bufferCopyRegion.imageSubresource.baseArrayLayer = textureUpload.sliceStart;
				bufferCopyRegion.imageSubresource.layerCount = textureUpload.sliceCount;
				bufferCopyRegion.imageExtent = { CrMax(texture->GetWidth() >> mip, 1u), CrMax(texture->GetHeight() >> mip, 1u), CrMax(texture->GetDepth() >> mip, 1u) };
				bufferCopyRegion.imageOffset = { 0, 0, 0 };

				bufferCopyRegion.bufferOffset = texture->GetGenericMipSliceLayout(mip, 0).offsetBytes;
				bufferCopyRegion.bufferRowLength = 0;
				bufferCopyRegion.bufferImageHeight = 0;
				bufferCopyRegions.push_back(bufferCopyRegion);
			}

			// Copy mip levels from staging buffer into texture memory
			vkCmdCopyBufferToImage(vulkanCommandBuffer->GetVkCommandBuffer(), vulkanStagingBuffer->GetVkBuffer(), vulkanTexture->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());

			// Once the data has been uploaded we transfer to the texture image to the shader read layout, so it can be sampled from
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.newLayout = vkImageStateInfo.imageLayout;

			// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition 
			// Source pipeline stage stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
			// Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
			vkCmdPipelineBarrier(vulkanCommandBuffer->GetVkCommandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		}

		// Cast to const_iterator to conform to EASTL's interface
		m_openTextureUploads.erase((CrHashMap<CrHash, CrTextureUpload>::const_iterator)textureUploadIter);
	}
}

uint8_t* CrRenderDeviceVulkan::BeginBufferUploadPS(const ICrHardwareGPUBuffer* destinationBuffer)
{
	uint32_t stagingBufferSizeBytes = destinationBuffer->GetSizeBytes();

	CrHardwareGPUBufferDescriptor stagingBufferDescriptor(cr3d::BufferUsage::TransferSrc, cr3d::MemoryAccess::StagingUpload, (uint32_t)stagingBufferSizeBytes);
	stagingBufferDescriptor.name = "Buffer Upload Staging Buffer";
	CrHardwareGPUBufferHandle stagingBuffer = CreateHardwareGPUBuffer(stagingBufferDescriptor);
	CrHardwareGPUBufferVulkan* vulkanStagingBuffer = static_cast<CrHardwareGPUBufferVulkan*>(stagingBuffer.get());

	CrBufferUpload bufferUpload;
	bufferUpload.stagingBuffer = stagingBuffer;
	bufferUpload.destinationBuffer = destinationBuffer;
	bufferUpload.sizeBytes = destinationBuffer->GetSizeBytes();
	bufferUpload.sourceOffsetBytes = 0;
	bufferUpload.destinationOffsetBytes = 0; // TODO Add as parameter

	CrHash textureHash(&destinationBuffer, sizeof(destinationBuffer));

	// Add to the open uploads for when we end the texture upload
	m_openBufferUploads.insert({ textureHash, bufferUpload });

	return (uint8_t*)vulkanStagingBuffer->Lock();
}

void CrRenderDeviceVulkan::EndBufferUploadPS(const ICrHardwareGPUBuffer* destinationBuffer)
{
	CrHash bufferHash(&destinationBuffer, sizeof(destinationBuffer));
	const auto bufferUploadIter = m_openBufferUploads.find(bufferHash);
	if (bufferUploadIter != m_openBufferUploads.end())
	{
		const CrBufferUpload& bufferUpload = bufferUploadIter->second;

		const CrHardwareGPUBufferVulkan* vulkanDestinationBuffer = static_cast<const CrHardwareGPUBufferVulkan*>(destinationBuffer);
		CrHardwareGPUBufferVulkan* vulkanStagingBuffer = static_cast<CrHardwareGPUBufferVulkan*>(bufferUpload.stagingBuffer.get());

		vulkanStagingBuffer->Unlock();

		CrCommandBufferVulkan* vulkanCommandBuffer = static_cast<CrCommandBufferVulkan*>(GetAuxiliaryCommandBuffer().get());
		{
			VkBufferMemoryBarrier bufferMemoryBarrier;
			bufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			bufferMemoryBarrier.pNext = nullptr;
			bufferMemoryBarrier.srcAccessMask = VK_ACCESS_NONE_KHR;
			bufferMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferMemoryBarrier.buffer = vulkanDestinationBuffer->GetVkBuffer();
			bufferMemoryBarrier.offset = 0;
			bufferMemoryBarrier.size = vulkanDestinationBuffer->GetSizeBytes();

			// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition 
			// Source pipeline stage is host write/read execution (VK_PIPELINE_STAGE_HOST_BIT)
			// Destination pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
			vkCmdPipelineBarrier(vulkanCommandBuffer->GetVkCommandBuffer(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &bufferMemoryBarrier, 0, nullptr);

			VkBufferCopy bufferCopyRegion;
			bufferCopyRegion.size = vulkanDestinationBuffer->GetSizeBytes();
			bufferCopyRegion.srcOffset = bufferUpload.sourceOffsetBytes;
			bufferCopyRegion.dstOffset = bufferUpload.destinationOffsetBytes;

			vkCmdCopyBuffer(vulkanCommandBuffer->GetVkCommandBuffer(), vulkanStagingBuffer->GetVkBuffer(), vulkanDestinationBuffer->GetVkBuffer(), 1, &bufferCopyRegion);

			// Once the data has been uploaded we transfer to the texture image to the shader read layout, so it can be sampled from
			bufferMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			bufferMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition 
			// Source pipeline stage stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
			// Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
			vkCmdPipelineBarrier(vulkanCommandBuffer->GetVkCommandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 1, &bufferMemoryBarrier, 0, nullptr);
		}

		// Cast to const_iterator to conform to EASTL's interface
		m_openBufferUploads.erase((CrHashMap<CrHash, CrBufferUpload>::const_iterator)bufferUploadIter);
	}
}

CrHardwareGPUBufferHandle CrRenderDeviceVulkan::DownloadBufferPS(const ICrHardwareGPUBuffer* sourceBuffer)
{
	uint32_t stagingBufferSizeBytes = sourceBuffer->GetSizeBytes();

	CrHardwareGPUBufferDescriptor stagingBufferDescriptor(cr3d::BufferUsage::TransferDst, cr3d::MemoryAccess::StagingDownload, (uint32_t)stagingBufferSizeBytes);
	CrHardwareGPUBufferHandle stagingBuffer = CreateHardwareGPUBuffer(stagingBufferDescriptor);
	CrHardwareGPUBufferVulkan* vulkanStagingBuffer = static_cast<CrHardwareGPUBufferVulkan*>(stagingBuffer.get());

	CrCommandBufferVulkan* vulkanCommandBuffer = static_cast<CrCommandBufferVulkan*>(GetAuxiliaryCommandBuffer().get());
	{
		const CrHardwareGPUBufferVulkan* vulkanSourceBuffer = static_cast<const CrHardwareGPUBufferVulkan*>(sourceBuffer);

		VkBufferCopy copyRegions;
		copyRegions.srcOffset = 0;
		copyRegions.dstOffset = 0;
		copyRegions.size = stagingBufferSizeBytes;

		VkBufferMemoryBarrier bufferMemoryBarrier;
		bufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferMemoryBarrier.pNext = nullptr;
		bufferMemoryBarrier.srcAccessMask = VK_ACCESS_NONE_KHR;
		bufferMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		bufferMemoryBarrier.buffer = vulkanSourceBuffer->GetVkBuffer();
		bufferMemoryBarrier.offset = 0;
		bufferMemoryBarrier.size = stagingBufferSizeBytes;

		vkCmdPipelineBarrier(vulkanCommandBuffer->GetVkCommandBuffer(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &bufferMemoryBarrier, 0, nullptr);

		vkCmdCopyBuffer(vulkanCommandBuffer->GetVkCommandBuffer(), vulkanSourceBuffer->GetVkBuffer(), vulkanStagingBuffer->GetVkBuffer(), 1, &copyRegions);

		bufferMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		bufferMemoryBarrier.dstAccessMask = VK_ACCESS_NONE_KHR;

		vkCmdPipelineBarrier(vulkanCommandBuffer->GetVkCommandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 1, &bufferMemoryBarrier, 0, nullptr);
	}

	return stagingBuffer;
}

void CrRenderDeviceVulkan::SubmitCommandBufferPS(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence)
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;

	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT; // TODO Need more control over this, probably best to put inside the semaphore object

	if (waitSemaphore)
	{
		submitInfo.pWaitSemaphores = &static_cast<const CrGPUSemaphoreVulkan*>(waitSemaphore)->GetVkSemaphore();
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitDstStageMask = &waitStageMask;
	}

	if (signalSemaphore)
	{
		submitInfo.pSignalSemaphores = &static_cast<const CrGPUSemaphoreVulkan*>(signalSemaphore)->GetVkSemaphore();
		submitInfo.signalSemaphoreCount = 1;
	}

	submitInfo.pCommandBuffers = &static_cast<const CrCommandBufferVulkan*>(commandBuffer)->GetVkCommandBuffer();

	VkResult result = vkQueueSubmit(m_vkGraphicsQueue, 1, &submitInfo, signalFence ? static_cast<const CrGPUFenceVulkan*>(signalFence)->GetVkFence() : nullptr);
	CrAssert(result == VK_SUCCESS);
}

VkResult CrRenderDeviceVulkan::SelectPhysicalDevice()
{
	VkResult result;

	uint32_t gpuCount = 0; // TODO Put in header file of RenderSystem
	result = vkEnumeratePhysicalDevices(m_vkInstance, &gpuCount, nullptr);
	CrAssertMsg(result == VK_SUCCESS && gpuCount > 0, "No GPUs found");

	CrVector<VkPhysicalDevice> physicalDevices(gpuCount);
	result = vkEnumeratePhysicalDevices(m_vkInstance, &gpuCount, physicalDevices.data());
	CrAssertMsg(result == VK_SUCCESS && gpuCount > 0, "No GPUs found");

	cr3d::GraphicsVendor::T preferredVendor = m_renderDeviceProperties.preferredVendor;

	struct SelectedDevice
	{
		cr3d::GraphicsVendor::T vendor = cr3d::GraphicsVendor::Unknown;
		uint32_t priority = 0;
		uint32_t index = 0xffffffff;
	};

	SelectedDevice maxPriorityDevice;
	SelectedDevice maxPriorityPreferredDevice;

	// Select from the list of available GPUs which one we want to use. A priority system will automatically select the best available from the list
	for (uint32_t i = 0; i < physicalDevices.size(); ++i)
	{
		VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevices[i], &vkPhysicalDeviceProperties);

		cr3d::GraphicsVendor::T graphicsVendor = cr3d::GraphicsVendor::FromVendorID(vkPhysicalDeviceProperties.vendorID);

		uint32_t priority = 0;
		priority |= vkPhysicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? (1 << 31) : 0;
		priority |= vkPhysicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? (1 << 30) : 0;
		priority |= vkPhysicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU ? (1 << 29) : 0;
		priority |= vkPhysicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU ? (1 << 28) : 0;

		if (maxPriorityDevice.priority < priority)
		{
			maxPriorityDevice.priority = priority;
			maxPriorityDevice.index = i;
		}

		if (graphicsVendor == preferredVendor)
		{
			if (maxPriorityPreferredDevice.priority < priority)
			{
				maxPriorityPreferredDevice.priority = priority;
				maxPriorityPreferredDevice.index = i;
			}
		}
	}

	// If we've found a device from the vendor we prefer, use that
	// Otherwise, use the best detected device
	if (maxPriorityPreferredDevice.index != 0xffffffff)
	{
		m_vkPhysicalDevice = physicalDevices[maxPriorityPreferredDevice.index];
	}
	else
	{
		m_vkPhysicalDevice = physicalDevices[maxPriorityDevice.index];
	}

	// Query physical device properties
	vkGetPhysicalDeviceMemoryProperties(m_vkPhysicalDevice, &m_vkPhysicalDeviceMemoryProperties);

	for (uint32_t i = 0; i < m_vkPhysicalDeviceMemoryProperties.memoryHeapCount; ++i)
	{
		VkMemoryHeap memoryHeap = m_vkPhysicalDeviceMemoryProperties.memoryHeaps[i];
		if (memoryHeap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
		{
			m_renderDeviceProperties.gpuMemoryBytes += memoryHeap.size;
		}
	}

	// Enumerate device extensions
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(m_vkPhysicalDevice, nullptr, &extensionCount, nullptr);
		CrVector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(m_vkPhysicalDevice, nullptr, &extensionCount, extensions.data());

		for (const VkExtensionProperties& extension : extensions)
		{
			m_supportedDeviceExtensions.insert(extension.extensionName);
		}
	}

	// Get initial properties from a function we know is supported in version 1.0
	// This is here only to get the Vulkan version supported by this physical device

	vkGetPhysicalDeviceProperties(m_vkPhysicalDevice, &m_vkPhysicalDeviceProperties2.properties);
	m_vkVersion = m_vkPhysicalDeviceProperties2.properties.apiVersion;
	uint32_t vulkanVersionMajor = VK_API_VERSION_MAJOR(m_vkVersion);
	uint32_t vulkanVersionMinor = VK_API_VERSION_MINOR(m_vkVersion);
	uint32_t vulkanVersionPatch = VK_API_VERSION_PATCH(m_vkVersion);

	m_renderDeviceProperties.graphicsApiDisplay.append_sprintf("%s %d.%d.%d", cr3d::GraphicsApi::ToString(cr3d::GraphicsApi::Vulkan), vulkanVersionMajor, vulkanVersionMinor, vulkanVersionPatch);

	// Populate the render device properties into the platform-independent structure
	m_renderDeviceProperties.vendor = cr3d::GraphicsVendor::FromVendorID(m_vkPhysicalDeviceProperties2.properties.vendorID);

	m_vk11PhysicalDeviceProperties = {};
	m_vk11PhysicalDeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;

	m_vk12PhysicalDeviceProperties = {};
	m_vk12PhysicalDeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;

	m_vk11DeviceSupportedFeatures = {};
	m_vk11DeviceSupportedFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;

	m_vk12DeviceSupportedFeatures = {};
	m_vk12DeviceSupportedFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

	VkDriverId driverId = VK_DRIVER_ID_MAX_ENUM;

	if (m_renderDeviceProperties.vendor == cr3d::GraphicsVendor::NVIDIA)
	{
		driverId = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
	}
	else if (m_renderDeviceProperties.vendor == cr3d::GraphicsVendor::Intel)
	{
		driverId = VK_DRIVER_ID_INTEL_PROPRIETARY_WINDOWS;
	}
	else if(m_renderDeviceProperties.vendor == cr3d::GraphicsVendor::AMD)
	{
		driverId = VK_DRIVER_ID_AMD_PROPRIETARY;
	}

	// Starting from Vulkan 1.1 we have this alternative available. There is also an extension that basically no one supports
	if (m_vkVersion >= VK_VERSION_1_1)
	{
		m_vkPhysicalDeviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		m_vkPhysicalDeviceProperties2.pNext = &m_vk11PhysicalDeviceProperties;

		m_vkDeviceSupportedFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		m_vkDeviceSupportedFeatures2.pNext = &m_vk11DeviceSupportedFeatures;

		// Starting from 1.2 we get even more properties
		if (m_vkVersion >= VK_VERSION_1_2)
		{
			m_vk11PhysicalDeviceProperties.pNext = &m_vk12PhysicalDeviceProperties;
			m_vk11DeviceSupportedFeatures.pNext = &m_vk12DeviceSupportedFeatures;
		}

		vkGetPhysicalDeviceProperties2(m_vkPhysicalDevice, &m_vkPhysicalDeviceProperties2);

		vkGetPhysicalDeviceFeatures2(m_vkPhysicalDevice, &m_vkDeviceSupportedFeatures2);

		// We check for the Vulkan version to avoid overriding the previously derived driverId
		if (m_vkVersion >= VK_VERSION_1_2)
		{
			driverId = m_vk12PhysicalDeviceProperties.driverID;
		}
	}
	else
	{
		vkGetPhysicalDeviceFeatures(m_vkPhysicalDevice, &m_vkDeviceSupportedFeatures2.features);
	}

	uint32_t driverVersionEncoded = m_vkPhysicalDeviceProperties2.properties.driverVersion;

	m_renderDeviceProperties.driverVersion.major = VK_API_VERSION_MAJOR(driverVersionEncoded);
	m_renderDeviceProperties.driverVersion.minor = VK_API_VERSION_MINOR(driverVersionEncoded);
	m_renderDeviceProperties.driverVersion.patch = VK_API_VERSION_PATCH(driverVersionEncoded);

	// https://github.com/baldurk/renderdoc/blob/v1.x/renderdoc/driver/vulkan/vk_common.cpp
	if (driverId == VK_DRIVER_ID_NVIDIA_PROPRIETARY)
	{
		m_renderDeviceProperties.driverVersion.major = ((uint32_t)(driverVersionEncoded) >> (8 + 8 + 6)) & 0x3ff;
		m_renderDeviceProperties.driverVersion.minor = ((uint32_t)(driverVersionEncoded) >> (8 + 6)) & 0xff;

		uint32_t secondary = ((uint32_t)(driverVersionEncoded) >> 6) & 0x0ff;
		uint32_t tertiary = driverVersionEncoded & 0x03f;

		m_renderDeviceProperties.driverVersion.patch = (secondary << 8) | tertiary;
	}
	else if (driverId == VK_DRIVER_ID_INTEL_PROPRIETARY_WINDOWS)
	{
		m_renderDeviceProperties.driverVersion.major = ((uint32_t)(driverVersionEncoded) >> 14) & 0x3fff;
		m_renderDeviceProperties.driverVersion.minor = (uint32_t)(driverVersionEncoded) & 0x3fff;
		m_renderDeviceProperties.driverVersion.patch = 0;
	}

	m_renderDeviceProperties.description = m_vkPhysicalDeviceProperties2.properties.deviceName;
	m_renderDeviceProperties.maxConstantBufferRange = m_vkPhysicalDeviceProperties2.properties.limits.maxUniformBufferRange;
	m_renderDeviceProperties.maxTextureDimension1D = m_vkPhysicalDeviceProperties2.properties.limits.maxImageDimension1D;
	m_renderDeviceProperties.maxTextureDimension2D = m_vkPhysicalDeviceProperties2.properties.limits.maxImageDimension2D;
	m_renderDeviceProperties.maxTextureDimension3D = m_vkPhysicalDeviceProperties2.properties.limits.maxImageDimension3D;
	m_renderDeviceProperties.isUMA = m_vkPhysicalDeviceProperties2.properties.deviceType & VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;

	// A pipeline cache belonging to renderdoc is invalid (it never has the same hash), so we don't want to 
	// store it or even delete a previous cache just because renderdoc can't use or create one.
	uint8_t* cacheUUID = m_vkPhysicalDeviceProperties2.properties.pipelineCacheUUID;
	m_isValidPipelineCache = !(cacheUUID[0] == 'r' && cacheUUID[1] == 'd' && cacheUUID[2] == 'o' && cacheUUID[3] == 'c');

	// Loop through all available formats and add to supported lists. These will be useful later 
	// when determining availability and features.

	VkFormatProperties formatProperties;
	for (cr3d::DataFormat::T dataFormat = (cr3d::DataFormat::T)0; dataFormat < cr3d::DataFormat::Count; ++dataFormat)
	{
		VkFormat format = crvk::GetVkFormat((cr3d::DataFormat::T)dataFormat);
		vkGetPhysicalDeviceFormatProperties(m_vkPhysicalDevice, (VkFormat)format, &formatProperties);

		// Format must support depth stencil attachment for optimal tiling
		if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			m_supportedDepthStencilFormats.insert(format);
		}

		if (formatProperties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT)
		{
			m_supportedVertexBufferFormats.insert(format);
		}

		if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
		{
			m_supportedTextureFormats.insert(format);
		}

		if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT)
		{
			m_supportedRenderTargetFormats.insert(format);
		}
	}

	return result;
}

ICrCommandBuffer* CrRenderDeviceVulkan::CreateCommandBufferPS(const CrCommandBufferDescriptor& descriptor)
{
	return new CrCommandBufferVulkan(this, descriptor);
}

ICrGPUFence* CrRenderDeviceVulkan::CreateGPUFencePS()
{
	return new CrGPUFenceVulkan(this);
}

ICrGPUSemaphore* CrRenderDeviceVulkan::CreateGPUSemaphorePS()
{
	return new CrGPUSemaphoreVulkan(this);
}

ICrGraphicsShader* CrRenderDeviceVulkan::CreateGraphicsShaderPS(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor)
{
	return new CrGraphicsShaderVulkan(this, graphicsShaderDescriptor);
}

ICrComputeShader* CrRenderDeviceVulkan::CreateComputeShaderPS(const CrComputeShaderDescriptor& computeShaderDescriptor)
{
	return new CrComputeShaderVulkan(this, computeShaderDescriptor);
}

ICrHardwareGPUBuffer* CrRenderDeviceVulkan::CreateHardwareGPUBufferPS(const CrHardwareGPUBufferDescriptor& descriptor)
{
	return new CrHardwareGPUBufferVulkan(this, descriptor);
}

ICrSampler* CrRenderDeviceVulkan::CreateSamplerPS(const CrSamplerDescriptor& descriptor)
{
	return new CrSamplerVulkan(this, descriptor);
}

ICrSwapchain* CrRenderDeviceVulkan::CreateSwapchainPS(const CrSwapchainDescriptor& swapchainDescriptor)
{
	return new CrSwapchainVulkan(this, swapchainDescriptor);
}

ICrTexture* CrRenderDeviceVulkan::CreateTexturePS(const CrTextureDescriptor& descriptor)
{
	return new CrTextureVulkan(this, descriptor);
}

ICrGraphicsPipeline* CrRenderDeviceVulkan::CreateGraphicsPipelinePS
(
	const CrGraphicsPipelineDescriptor& pipelineDescriptor,
	const CrGraphicsShaderHandle& graphicsShader,
	const CrVertexDescriptor& vertexDescriptor
)
{
	return new CrGraphicsPipelineVulkan(this, pipelineDescriptor, graphicsShader, vertexDescriptor);
}

ICrComputePipeline* CrRenderDeviceVulkan::CreateComputePipelinePS(const CrComputeShaderHandle& computeShader)
{
	return new CrComputePipelineVulkan(this, computeShader);
}

ICrGPUQueryPool* CrRenderDeviceVulkan::CreateGPUQueryPoolPS(const CrGPUQueryPoolDescriptor& queryPoolDescriptor)
{
	return new CrGPUQueryPoolVulkan(this, queryPoolDescriptor);
}

void CrRenderDeviceVulkan::RetrieveQueueFamilies()
{
	struct QueueProperties
	{
		bool doesGraphics;
		bool doesCompute;
		bool doesCopy;
		bool doesPresent;
		uint32_t maxQueues;
	};

	// Select appropriate queues that can ideally do graphics, compute and copy. Some graphics hardware can have multiple queue types.
	// We also create the logical device with the information of how many queues of a type we want. We create them up front and we retrieve
	// them later. This is different to what DX12/Metal do in that we need to allocate them ourselves up front.
	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueFamilyCount, nullptr);

	CrVector<VkQueueFamilyProperties> vkQueueProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueFamilyCount, vkQueueProperties.data());

	CrVector<QueueProperties> queueProperties(queueFamilyCount);

	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		VkQueueFamilyProperties& queueProperty = vkQueueProperties[i];
		uint32_t flags = queueProperty.queueFlags;
		queueProperties[i].doesGraphics = (flags & VK_QUEUE_GRAPHICS_BIT) != 0;
		queueProperties[i].doesCompute = (flags & VK_QUEUE_COMPUTE_BIT) != 0;
		queueProperties[i].doesCopy = (flags & VK_QUEUE_TRANSFER_BIT) != 0;
		queueProperties[i].maxQueues = queueProperty.queueCount;

		queueProperties[i].doesPresent = queueProperties[i].doesGraphics;
	}

	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		if (queueProperties[i].doesGraphics &&
			queueProperties[i].doesCompute &&
			queueProperties[i].doesCopy &&
			queueProperties[i].doesPresent)
		{
			m_commandQueueFamilyIndex = i;
			m_maxCommandQueues = queueProperties[i].maxQueues;
			break;
		}
	}

	CrAssertMsg(m_maxCommandQueues > 0, "Couldn't find appropriate queue for the render device");
}

VkResult CrRenderDeviceVulkan::CreateLogicalDevice()
{
	CrVector<const char*> enabledDeviceExtensions;

	if (IsVkDeviceExtensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
	{
		enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	if (IsVkDeviceExtensionSupported(VK_KHR_MAINTENANCE1_EXTENSION_NAME))
	{
		enabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
	}

	// To use instance id inside a shader
	if (IsVkDeviceExtensionSupported(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME))
	{
		enabledDeviceExtensions.push_back(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
	}

	// We allocate queues up front, which are later retrieved. We don't really allocate command queues
	// on demand, we have them cached within the device at creation time
	CrVector<float> queuePriorities(m_maxCommandQueues);
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = m_commandQueueFamilyIndex;
	queueCreateInfo.queueCount = m_maxCommandQueues;
	queueCreateInfo.pQueuePriorities = queuePriorities.data();

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1; // TODO Potentially create other types of queues
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

	// Enable all available features
	if (m_vkVersion >= VK_VERSION_1_1)
	{
		deviceCreateInfo.pNext = &m_vkDeviceSupportedFeatures2;
	}
	else
	{
		deviceCreateInfo.pEnabledFeatures = &m_vkDeviceSupportedFeatures2.features;
	}

	if (enabledDeviceExtensions.size() > 0)
	{
		deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledDeviceExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();
	}

	VkResult vkResult = vkCreateDevice(m_vkPhysicalDevice, &deviceCreateInfo, nullptr, &m_vkDevice);
	CrAssertMsg(vkResult == VK_SUCCESS, "Could not create vkDevice");

	m_renderDeviceProperties.supportedFeatures[CrRenderingFeature::Tessellation]           = m_vkDeviceSupportedFeatures2.features.tessellationShader;
	m_renderDeviceProperties.supportedFeatures[CrRenderingFeature::GeometryShaders]        = m_vkDeviceSupportedFeatures2.features.geometryShader;
	m_renderDeviceProperties.supportedFeatures[CrRenderingFeature::TextureCompressionBC]   = m_vkDeviceSupportedFeatures2.features.textureCompressionBC;
	m_renderDeviceProperties.supportedFeatures[CrRenderingFeature::TextureCompressionETC]  = m_vkDeviceSupportedFeatures2.features.textureCompressionETC2;
	m_renderDeviceProperties.supportedFeatures[CrRenderingFeature::TextureCompressionASTC] = m_vkDeviceSupportedFeatures2.features.textureCompressionASTC_LDR;

	return vkResult;
}

bool CrRenderDeviceVulkan::IsVkDeviceExtensionSupported(const CrString& extension)
{
	return m_supportedDeviceExtensions.count(extension) > 0;
}

uint32_t CrRenderDeviceVulkan::GetVkMemoryType(uint32_t typeBits, VkFlags properties) const
{
	for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((m_vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}
		typeBits >>= 1;
	}

	return VK_MAX_MEMORY_TYPES;
}

uint32_t CrRenderDeviceVulkan::ReserveVkQueueIndex()
{
	uint32_t queueIndex = m_numCommandQueues;

	CrAssertMsg(queueIndex < m_maxCommandQueues, "No more command queues available");

	m_numCommandQueues++;

	// TODO The number of queues depends on the type of queue and the capabilities
	return queueIndex;
}

uint32_t CrRenderDeviceVulkan::GetVkQueueMaxCount() const
{
	// TODO There is a maximum based on type of command queue and capability
	return m_maxCommandQueues;
}

uint32_t CrRenderDeviceVulkan::GetVkQueueFamilyIndex() const
{
	// TODO Need to return an index based on type of queue, capabilities, etc.
	return m_commandQueueFamilyIndex;
}

VkQueue CrRenderDeviceVulkan::GetVkQueue(CrCommandQueueType::T queueType) const
{
	CrAssertMsg(queueType == CrCommandQueueType::Graphics, "Graphics queue not implemented");

	switch (queueType)
	{
		case CrCommandQueueType::Graphics: return m_vkGraphicsQueue;
		default: return m_vkGraphicsQueue;
	}
}

VkCommandPool CrRenderDeviceVulkan::GetVkCommandPool(CrCommandQueueType::T queueType) const
{
	CrAssertMsg(queueType == CrCommandQueueType::Graphics, "Graphics queue not implemented");

	switch (queueType)
	{
		case CrCommandQueueType::Graphics: return m_vkGraphicsCommandPool;
		default: return m_vkGraphicsCommandPool;
	}
}

// Transitions texture to an initial, predictable state
void CrRenderDeviceVulkan::TransitionVkTextureToInitialLayout(const CrTextureVulkan* vulkanTexture, const cr3d::TextureState& textureState)
{
	// Transition all resources to their initial state
	VkImageSubresourceRange subresourceRange;
	subresourceRange.aspectMask = crvk::GetVkImageAspectFlags(vulkanTexture->GetFormat());
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = vulkanTexture->GetMipmapCount();
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = vulkanTexture->GetArraySize();

	const CrVkImageStateInfo& imageStateInfo = CrTextureVulkan::GetVkImageStateInfo(textureState.layout);

	VkImageMemoryBarrier imageMemoryBarrier;
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;
	imageMemoryBarrier.image = vulkanTexture->GetVkImage();
	imageMemoryBarrier.subresourceRange = subresourceRange;
	imageMemoryBarrier.srcAccessMask = 0;
	imageMemoryBarrier.dstAccessMask = imageStateInfo.accessMask;

	// Initial layout is undefined. We'll never use this state again
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageMemoryBarrier.newLayout = imageStateInfo.imageLayout;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	// No need to specify anything special
	CrCommandBufferVulkan* vulkanCommanddBuffer = static_cast<CrCommandBufferVulkan*>(GetAuxiliaryCommandBuffer().get());
	vkCmdPipelineBarrier(vulkanCommanddBuffer->GetVkCommandBuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

void CrRenderDeviceVulkan::SetVkObjectName(uint64_t vkObject, VkObjectType objectType, const char* name) const
{
	if (vkSetDebugUtilsObjectName && name && name[0] != 0)
	{
		VkDebugUtilsObjectNameInfoEXT nameInfo = {};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = objectType;
		nameInfo.objectHandle = vkObject;
		nameInfo.pObjectName = name;
		vkSetDebugUtilsObjectName(m_vkDevice, &nameInfo);
	}
}