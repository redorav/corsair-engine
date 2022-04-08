#include "CrRendering_pch.h"

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

PFN_vkDebugMarkerSetObjectTagEXT	vkDebugMarkerSetObjectTag = nullptr;
PFN_vkDebugMarkerSetObjectNameEXT	vkDebugMarkerSetObjectName = nullptr;
PFN_vkCmdDebugMarkerBeginEXT		vkCmdDebugMarkerBegin = nullptr;
PFN_vkCmdDebugMarkerEndEXT			vkCmdDebugMarkerEnd = nullptr;
PFN_vkCmdDebugMarkerInsertEXT		vkCmdDebugMarkerInsert = nullptr;

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

CrRenderDeviceVulkan::CrRenderDeviceVulkan(const ICrRenderSystem* renderSystem)
	: ICrRenderDevice(renderSystem)
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
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // TODO Transient or Reset? Check Metal and DX12
	VkResult vkResult = vkCreateCommandPool(m_vkDevice, &cmdPoolInfo, nullptr, &m_vkGraphicsCommandPool);
	CrAssert(vkResult == VK_SUCCESS);

	// TODO This is per-device but it's currently global
	if (IsVkDeviceExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
	{
		vkDebugMarkerSetObjectTag  = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(m_vkDevice, "vkDebugMarkerSetObjectTagEXT");
		vkDebugMarkerSetObjectName = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(m_vkDevice, "vkDebugMarkerSetObjectNameEXT");
		vkCmdDebugMarkerBegin      = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(m_vkDevice, "vkCmdDebugMarkerBeginEXT");
		vkCmdDebugMarkerEnd        = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(m_vkDevice, "vkCmdDebugMarkerEndEXT");
		vkCmdDebugMarkerInsert     = (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(m_vkDevice, "vkCmdDebugMarkerInsertEXT");
	}

	// Load serialized pipeline cache from disk. This pipeline cache is invalid if the uuid doesn't match
	CrVector<char> pipelineCacheData;
	LoadPipelineCache(pipelineCacheData);

	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	if (!pipelineCacheData.empty())
	{
		const VkPipelineCacheHeader& pipelineCacheHeader = reinterpret_cast<VkPipelineCacheHeader&>(*pipelineCacheData.data());
		bool matchesUUID = memcmp(pipelineCacheHeader.uuid, m_vkPhysicalDeviceProperties.pipelineCacheUUID, VK_UUID_SIZE) == 0;

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

	// Select from the list of available GPUs which one we want to use. A priority system will automatically select
	// the best available from the list
	uint32_t highestPriority = 0;
	uint32_t highestPriorityIndex = 0;
	for (uint32_t i = 0; i < physicalDevices.size(); ++i)
	{
		VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevices[i], &vkPhysicalDeviceProperties);

		VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevices[i], &vkPhysicalDeviceMemoryProperties);

		uint32_t priority = 0;
		priority |= vkPhysicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU   ? (1 << 31) : 0;
		priority |= vkPhysicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? (1 << 30) : 0;
		priority |= vkPhysicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU    ? (1 << 29) : 0;
		priority |= vkPhysicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU            ? (1 << 28) : 0;

		if (priority > highestPriority)
		{
			highestPriority = priority;
			highestPriorityIndex = i;
		}
	}

	m_vkPhysicalDevice = physicalDevices[highestPriorityIndex];

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

	vkGetPhysicalDeviceProperties(m_vkPhysicalDevice, &m_vkPhysicalDeviceProperties);

	// Populate the render device properties into the platform-independent structure
	m_renderDeviceProperties.vendor = GetVendorFromVendorID(m_vkPhysicalDeviceProperties.vendorID);
	m_renderDeviceProperties.description = m_vkPhysicalDeviceProperties.deviceName;
	m_renderDeviceProperties.maxConstantBufferRange = m_vkPhysicalDeviceProperties.limits.maxUniformBufferRange;
	m_renderDeviceProperties.maxTextureDimension1D = m_vkPhysicalDeviceProperties.limits.maxImageDimension1D;
	m_renderDeviceProperties.maxTextureDimension2D = m_vkPhysicalDeviceProperties.limits.maxImageDimension2D;
	m_renderDeviceProperties.maxTextureDimension3D = m_vkPhysicalDeviceProperties.limits.maxImageDimension3D;
	m_renderDeviceProperties.isUMA = m_vkPhysicalDeviceProperties.deviceType & VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;

	// A pipeline cache belonging to renderdoc is invalid (it never has the same hash), so we don't want to 
	// store it or even delete a previous cache just because renderdoc can't use or create one.
	uint8_t* cacheUUID = m_vkPhysicalDeviceProperties.pipelineCacheUUID;
	m_isValidPipelineCache = !(cacheUUID[0] == 'r' && cacheUUID[1] == 'd' && cacheUUID[2] == 'o' && cacheUUID[3] == 'c');

	// Loop through all available formats and add to supported lists. These will be useful later 
	// when determining availability and features.

	VkFormatProperties formatProperties;
	for (uint32_t dataFormat = 0; dataFormat < cr3d::DataFormat::Count; ++dataFormat)
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

	return result;
}

ICrCommandBuffer* CrRenderDeviceVulkan::CreateCommandBufferPS(CrCommandQueueType::T type)
{
	return new CrCommandBufferVulkan(this, type);
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

ICrComputePipeline* CrRenderDeviceVulkan::CreateComputePipelinePS(const CrComputePipelineDescriptor& /*pipelineDescriptor*/, const CrComputeShaderHandle& computeShader)
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

	if (IsVkDeviceExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
	{
		enabledDeviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
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

	// Store the physical features as we can query them
	vkGetPhysicalDeviceFeatures(m_vkPhysicalDevice, &m_vkDeviceSupportedFeatures);

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1; // TODO Potentially create other types of queues
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.pEnabledFeatures = &m_vkDeviceSupportedFeatures; // Enable all available features

	if (enabledDeviceExtensions.size() > 0)
	{
		deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledDeviceExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensions.data();
	}

	VkResult vkResult = vkCreateDevice(m_vkPhysicalDevice, &deviceCreateInfo, nullptr, &m_vkDevice);
	CrAssertMsg(vkResult == VK_SUCCESS, "Could not create vkDevice");

	m_renderDeviceProperties.supportedFeatures[CrRenderingFeature::Tessellation]           = m_vkDeviceSupportedFeatures.tessellationShader;
	m_renderDeviceProperties.supportedFeatures[CrRenderingFeature::GeometryShaders]        = m_vkDeviceSupportedFeatures.geometryShader;
	m_renderDeviceProperties.supportedFeatures[CrRenderingFeature::TextureCompressionBC]   = m_vkDeviceSupportedFeatures.textureCompressionBC;
	m_renderDeviceProperties.supportedFeatures[CrRenderingFeature::TextureCompressionETC]  = m_vkDeviceSupportedFeatures.textureCompressionETC2;
	m_renderDeviceProperties.supportedFeatures[CrRenderingFeature::TextureCompressionASTC] = m_vkDeviceSupportedFeatures.textureCompressionASTC_LDR;

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

void CrRenderDeviceVulkan::SetVkObjectName(uint64_t vkObject, VkDebugReportObjectTypeEXT objectType, const char* name) const
{
	if (vkDebugMarkerSetObjectName && name && name[0] != 0)
	{
		VkDebugMarkerObjectNameInfoEXT nameInfo = {};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = objectType;
		nameInfo.object = vkObject;
		nameInfo.pObjectName = name;
		vkDebugMarkerSetObjectName(m_vkDevice, &nameInfo);
	}
}