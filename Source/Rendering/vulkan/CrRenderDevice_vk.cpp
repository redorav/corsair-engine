#include "CrRendering_pch.h"

#include "CrRenderSystem_vk.h"
#include "CrRenderDevice_vk.h"

#include "CrCommandQueue_vk.h"
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

	// 5. Create main command queue. This will take care of the main command buffers and present
	m_mainCommandQueue = CreateCommandQueue(CrCommandQueueType::Graphics);

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

	VkResult result = vkCreatePipelineCache(m_vkDevice, &pipelineCacheCreateInfo, nullptr, &m_vkPipelineCache);
	CrAssertMsg(result == VK_SUCCESS, "Failed to create pipeline cache");
}

CrRenderDeviceVulkan::~CrRenderDeviceVulkan()
{
	// Store pipeline cache to disk
	size_t pipelineCacheSize = 0;
	vkGetPipelineCacheData(m_vkDevice, m_vkPipelineCache, &pipelineCacheSize, nullptr);

	CrVector<char> pipelineCacheData(pipelineCacheSize);
	vkGetPipelineCacheData(m_vkDevice, m_vkPipelineCache, &pipelineCacheSize, pipelineCacheData.data());

	StorePipelineCache(pipelineCacheData.data(), pipelineCacheSize);
}

cr3d::GPUFenceResult CrRenderDeviceVulkan::WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds) const
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

void CrRenderDeviceVulkan::ResetFencePS(const ICrGPUFence* fence)
{
	vkResetFences(m_vkDevice, 1, &static_cast<const CrGPUFenceVulkan*>(fence)->GetVkFence());
}

void CrRenderDeviceVulkan::WaitIdlePS()
{
	vkDeviceWaitIdle(m_vkDevice);
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

ICrCommandQueue* CrRenderDeviceVulkan::CreateCommandQueuePS(CrCommandQueueType::T type)
{
	return new CrCommandQueueVulkan(this, type);
}

ICrGPUFence* CrRenderDeviceVulkan::CreateGPUFencePS()
{
	return new CrGPUFenceVulkan(this);
}

ICrGPUSemaphore* CrRenderDeviceVulkan::CreateGPUSemaphorePS()
{
	return new CrGPUSemaphoreVulkan(this);
}

ICrGraphicsShader* CrRenderDeviceVulkan::CreateGraphicsShaderPS(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor) const
{
	return new CrGraphicsShaderVulkan(this, graphicsShaderDescriptor);
}

ICrComputeShader* CrRenderDeviceVulkan::CreateComputeShaderPS(const CrComputeShaderDescriptor& computeShaderDescriptor) const
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
	const ICrGraphicsShader* graphicsShader,
	const CrVertexDescriptor& vertexDescriptor
)
{
	CrGraphicsPipelineVulkan* vulkanGraphicsPipeline = new CrGraphicsPipelineVulkan();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.pNext = nullptr;
	inputAssemblyState.flags = 0;
	inputAssemblyState.topology = crvk::GetVkPrimitiveTopology(pipelineDescriptor.primitiveTopology);
	inputAssemblyState.primitiveRestartEnable = false;

	VkPipelineRasterizationStateCreateInfo rasterizerState = {};
	rasterizerState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerState.pNext = nullptr;
	rasterizerState.flags = 0;
	rasterizerState.depthClampEnable = false;
	rasterizerState.polygonMode = crvk::GetVkPolygonFillMode(pipelineDescriptor.rasterizerState.fillMode);
	rasterizerState.cullMode = crvk::GetVkPolygonCullMode(pipelineDescriptor.rasterizerState.cullMode);
	rasterizerState.frontFace = crvk::GetVkFrontFace(pipelineDescriptor.rasterizerState.frontFace);

	// TODO Complete this section
	rasterizerState.depthClampEnable = pipelineDescriptor.rasterizerState.depthClipEnable;
	rasterizerState.lineWidth = 1.0f;
	//rasterizerState.rasterizerDiscardEnable = false;
	//rasterizerState.depthBiasEnable = false;

	VkPipelineColorBlendStateCreateInfo colorBlendState;
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.pNext = nullptr;
	colorBlendState.flags = 0;
	colorBlendState.logicOpEnable = false;
	colorBlendState.logicOp = VK_LOGIC_OP_NO_OP;

	CrFixedVector<VkPipelineColorBlendAttachmentState, cr3d::MaxRenderTargets> blendAttachments;
	for (uint32_t i = 0, end = cr3d::MaxRenderTargets; i < end; ++i)
	{
		const CrRenderTargetBlendDescriptor& renderTargetBlend = pipelineDescriptor.blendState.renderTargetBlends[i];
		const CrRenderTargetFormatDescriptor& renderTarget = pipelineDescriptor.renderTargets;

		if (renderTarget.colorFormats[i] != cr3d::DataFormat::Invalid)
		{
			VkPipelineColorBlendAttachmentState& blendAttachment = blendAttachments.push_back();
			blendAttachment.colorWriteMask = renderTargetBlend.colorWriteMask;
			blendAttachment.blendEnable = renderTargetBlend.enable;

			if (renderTargetBlend.enable)
			{
				blendAttachment.colorBlendOp        = crvk::GetVkBlendOp(renderTargetBlend.colorBlendOp);
				blendAttachment.dstColorBlendFactor = crvk::GetVkBlendFactor(renderTargetBlend.dstColorBlendFactor);
				blendAttachment.srcColorBlendFactor = crvk::GetVkBlendFactor(renderTargetBlend.srcColorBlendFactor);

				blendAttachment.alphaBlendOp        = crvk::GetVkBlendOp(renderTargetBlend.alphaBlendOp);
				blendAttachment.dstAlphaBlendFactor = crvk::GetVkBlendFactor(renderTargetBlend.dstAlphaBlendFactor);
				blendAttachment.srcAlphaBlendFactor = crvk::GetVkBlendFactor(renderTargetBlend.srcAlphaBlendFactor);
			}
		}
		else
		{
			break;
		}
	}

	colorBlendState.attachmentCount = (uint32_t)blendAttachments.size();
	colorBlendState.pAttachments = blendAttachments.data();
	colorBlendState.blendConstants[0] = 1.0f;
	colorBlendState.blendConstants[1] = 1.0f;
	colorBlendState.blendConstants[2] = 1.0f;
	colorBlendState.blendConstants[3] = 1.0f;

	// Multi sampling state
	VkPipelineMultisampleStateCreateInfo multisampleState;
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.pNext = nullptr;
	multisampleState.flags = 0;
	multisampleState.rasterizationSamples = crvk::GetVkSampleCount(pipelineDescriptor.sampleCount);
	multisampleState.sampleShadingEnable = false;
	multisampleState.minSampleShading = 0.0f;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = false;
	multisampleState.alphaToOneEnable = false;

	// Depth stencil state
	VkPipelineDepthStencilStateCreateInfo depthStencilState;
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.pNext = nullptr;
	depthStencilState.flags = 0;

	// Depth
	depthStencilState.depthTestEnable = pipelineDescriptor.depthStencilState.depthTestEnable;
	depthStencilState.depthWriteEnable = pipelineDescriptor.depthStencilState.depthWriteEnable;
	depthStencilState.depthCompareOp = crvk::GetVkCompareOp(pipelineDescriptor.depthStencilState.depthCompareOp);
	depthStencilState.depthBoundsTestEnable = pipelineDescriptor.depthStencilState.depthBoundsTestEnable;

	// Stencil
	depthStencilState.stencilTestEnable = pipelineDescriptor.depthStencilState.stencilTestEnable;

	depthStencilState.front.depthFailOp = crvk::GetVkStencilOp(pipelineDescriptor.depthStencilState.frontDepthFailOp);
	depthStencilState.front.failOp      = crvk::GetVkStencilOp(pipelineDescriptor.depthStencilState.frontStencilFailOp);
	depthStencilState.front.passOp      = crvk::GetVkStencilOp(pipelineDescriptor.depthStencilState.frontStencilPassOp);
	depthStencilState.front.compareOp   = crvk::GetVkCompareOp(pipelineDescriptor.depthStencilState.frontStencilCompareOp);
	depthStencilState.front.compareMask = pipelineDescriptor.depthStencilState.stencilReadMask;
	depthStencilState.front.writeMask   = pipelineDescriptor.depthStencilState.stencilWriteMask;
	depthStencilState.front.reference   = pipelineDescriptor.depthStencilState.reference;

	depthStencilState.back.depthFailOp  = crvk::GetVkStencilOp(pipelineDescriptor.depthStencilState.backDepthFailOp);
	depthStencilState.back.failOp       = crvk::GetVkStencilOp(pipelineDescriptor.depthStencilState.backStencilFailOp);
	depthStencilState.back.passOp       = crvk::GetVkStencilOp(pipelineDescriptor.depthStencilState.backStencilPassOp);
	depthStencilState.back.compareOp    = crvk::GetVkCompareOp(pipelineDescriptor.depthStencilState.backStencilCompareOp);
	depthStencilState.back.compareMask  = pipelineDescriptor.depthStencilState.stencilReadMask;
	depthStencilState.back.writeMask    = pipelineDescriptor.depthStencilState.stencilWriteMask;
	depthStencilState.back.reference    = pipelineDescriptor.depthStencilState.reference;

	depthStencilState.minDepthBounds = 0.0f;
	depthStencilState.maxDepthBounds = 1.0f;

	// Viewport state
	VkPipelineViewportStateCreateInfo viewportState;
	viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext         = nullptr;
	viewportState.flags         = 0;
	viewportState.viewportCount = 1; // One viewport
	viewportState.pViewports    = nullptr;
	viewportState.scissorCount  = 1; // One scissor rectangle
	viewportState.pScissors     = nullptr;

	// Dynamic states can be set even after the pipeline has been created, so there is no need to create new pipelines
	// just for changing a viewport's dimensions or a scissor box
	// The dynamic state properties themselves are stored in the command buffer
	CrArray<VkDynamicState, 3> dynamicStateEnables =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_STENCIL_REFERENCE
	};

	VkPipelineDynamicStateCreateInfo dynamicState;
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pNext = nullptr;
	dynamicState.flags = 0;
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = (uint32_t)dynamicStateEnables.size();

	VkResult vkResult;

	// Shader information
	VkPipelineShaderStageCreateInfo shaderStages[cr3d::ShaderStage::GraphicsStageCount] = {};
	VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
	shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

	const CrGraphicsShaderVulkan* vulkanGraphicsShader = static_cast<const CrGraphicsShaderVulkan*>(graphicsShader);

	const CrVector<VkShaderModule>& vkShaderModules = vulkanGraphicsShader->GetVkShaderModules();

	const CrVector<CrShaderBytecodeSharedHandle>& bytecodes = vulkanGraphicsShader->GetBytecodes();

	uint32_t usedShaderStages = 0;

	for (uint32_t i = 0; i < bytecodes.size(); ++i)
	{
		const CrShaderBytecodeSharedHandle& bytecode = bytecodes[i];
		const VkShaderModule vkShaderModule = vkShaderModules[i];

		shaderStageCreateInfo.module = vkShaderModule;
		shaderStageCreateInfo.stage = crvk::GetVkShaderStage(bytecode->GetShaderStage());
		shaderStageCreateInfo.pName = bytecode->GetEntryPoint().c_str();
		shaderStages[usedShaderStages++] = shaderStageCreateInfo;
	}

	const CrShaderBindingLayoutVulkan& bindingTable = static_cast<const CrShaderBindingLayoutVulkan&>(graphicsShader->GetBindingLayout());

	// Pipeline Resource Layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = 1; // TODO Handle when we have more than one layout
	pipelineLayoutCreateInfo.pSetLayouts = &bindingTable.m_vkDescriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
	// TODO Push constants? Need to be part of the psoDescriptor?

	vkResult = vkCreatePipelineLayout(m_vkDevice, &pipelineLayoutCreateInfo, nullptr, &vulkanGraphicsPipeline->m_vkPipelineLayout);

	CrAssertMsg(vkResult == VK_SUCCESS, "Failed to create pipeline layout");

	uint32_t vertexStreamCount = vertexDescriptor.GetStreamCount();
	CrArray<VkVertexInputBindingDescription, cr3d::MaxVertexStreams> bindingDescriptions;

	for (uint32_t streamId = 0; streamId < vertexStreamCount; ++streamId)
	{
		bindingDescriptions[streamId].binding = streamId;
		bindingDescriptions[streamId].stride = vertexDescriptor.GetStreamStride(streamId);
		bindingDescriptions[streamId].inputRate = crvk::GetVkVertexInputRate(vertexDescriptor.GetInputRate(streamId));
	}

	uint32_t attributeCount = vertexDescriptor.GetAttributeCount();
	CrArray<VkVertexInputAttributeDescription, cr3d::MaxVertexAttributes> attributeDescriptions;

	uint32_t offset   = 0;
	uint32_t streamId = 0;
	for (uint32_t i = 0; i < attributeCount; ++i)
	{
		const CrVertexAttribute& vertexAttribute = vertexDescriptor.GetAttribute(i);
		const cr3d::DataFormatInfo& vertexFormatInfo = cr3d::DataFormats[vertexAttribute.format];
		attributeDescriptions[i].binding = vertexAttribute.streamId;
		attributeDescriptions[i].location = i; // We assume attributes come in the order the shader expects them
		attributeDescriptions[i].format = crvk::GetVkFormat((cr3d::DataFormat::T)vertexAttribute.format);

		if (streamId != vertexAttribute.streamId)
		{
			streamId = vertexAttribute.streamId;
			offset = 0;
		}

		attributeDescriptions[i].offset = offset;
		offset += vertexFormatInfo.dataOrBlockSize;
	}

	// Assign to vertex buffer
	VkPipelineVertexInputStateCreateInfo vertexInputState;
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.pNext = nullptr;
	vertexInputState.flags = 0;
	vertexInputState.vertexBindingDescriptionCount = vertexStreamCount;
	vertexInputState.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputState.vertexAttributeDescriptionCount = attributeCount;
	vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();

	// We don't need the real renderpass here to create pipeline state objects, a compatible one is enough
	// to make it work. I suspect the only thing it really needs is the description of how we're going to
	// be using the render pass. In D3D12 this is actually part of the pipeline descriptor (RTVFormats, DSVFormat)
	// TODO This should probably be optimized either by having another function that passes an actual render pass
	// or by having a cache of render pass descriptors. The reason for using a dummy render pass is so that
	// pipelines can be precreated without needing access to the actual draw commands. Another alternative is
	// to create the render pass on the stack by using a custom allocator.
	VkRenderPass vkCompatibleRenderPass;
	{
		CrFixedVector<VkAttachmentDescription, cr3d::MaxRenderTargets + 1> attachments;
		CrFixedVector<VkAttachmentReference, cr3d::MaxRenderTargets> colorReferences;
		VkAttachmentReference depthReference;

		uint32_t numColorAttachments = colorBlendState.attachmentCount;
		uint32_t numDepthAttachments = 0;

		for (uint32_t i = 0; i < numColorAttachments; ++i)
		{
			colorReferences.push_back({ i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			VkAttachmentDescription attachmentDescription = {};
			attachmentDescription.flags = 0;
			attachmentDescription.format = crvk::GetVkFormat(pipelineDescriptor.renderTargets.colorFormats[i]);
			attachmentDescription.samples = crvk::GetVkSampleCount(pipelineDescriptor.renderTargets.sampleCount);
			attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
			attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments.push_back(attachmentDescription);
		}

		if (pipelineDescriptor.renderTargets.depthFormat != cr3d::DataFormat::Invalid)
		{
			depthReference = { numColorAttachments, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
			VkAttachmentDescription attachmentDescription = {};
			attachmentDescription.flags = 0;
			attachmentDescription.format = crvk::GetVkFormat(pipelineDescriptor.renderTargets.depthFormat);
			attachmentDescription.samples = crvk::GetVkSampleCount(pipelineDescriptor.renderTargets.sampleCount);
			attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
			attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments.push_back(attachmentDescription);
			numDepthAttachments = 1;
		}

		// All render passes need at least one subpass to work.
		// By defining a subpass dependency as external on both sides, we get the simplest render pass
		VkSubpassDescription subpassDescription;
		subpassDescription.flags = 0;
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.inputAttachmentCount = 0;
		subpassDescription.pInputAttachments = nullptr;
		subpassDescription.colorAttachmentCount = numColorAttachments;
		subpassDescription.pColorAttachments = colorReferences.data();
		subpassDescription.pResolveAttachments = nullptr;
		subpassDescription.pDepthStencilAttachment = numDepthAttachments > 0 ? &depthReference : nullptr;
		subpassDescription.preserveAttachmentCount = 0;
		subpassDescription.pPreserveAttachments = nullptr;

		// Create the renderpass
		VkRenderPassCreateInfo renderPassInfo;
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pNext = nullptr;
		renderPassInfo.flags = 0;
		renderPassInfo.attachmentCount = (uint32_t)attachments.size();
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = 0;
		renderPassInfo.pDependencies = nullptr;

		vkResult = vkCreateRenderPass(m_vkDevice, &renderPassInfo, nullptr, &vkCompatibleRenderPass);
		CrAssert(vkResult == VK_SUCCESS);
	}

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
	graphicsPipelineCreateInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.stageCount          = (uint32_t)graphicsShader->GetBytecodes().size();
	graphicsPipelineCreateInfo.pStages             = shaderStages;

	graphicsPipelineCreateInfo.layout              = vulkanGraphicsPipeline->m_vkPipelineLayout;
	graphicsPipelineCreateInfo.pVertexInputState   = &vertexInputState;
	graphicsPipelineCreateInfo.renderPass          = vkCompatibleRenderPass;

	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizerState;
	graphicsPipelineCreateInfo.pColorBlendState    = &colorBlendState;
	graphicsPipelineCreateInfo.pMultisampleState   = &multisampleState;
	graphicsPipelineCreateInfo.pViewportState      = &viewportState;
	graphicsPipelineCreateInfo.pDepthStencilState  = &depthStencilState;
	graphicsPipelineCreateInfo.pDynamicState       = &dynamicState;

	vkResult = vkCreateGraphicsPipelines(m_vkDevice, m_vkPipelineCache, 1, &graphicsPipelineCreateInfo, nullptr, &vulkanGraphicsPipeline->m_vkPipeline);
	CrAssertMsg(vkResult == VK_SUCCESS, "Failed to create graphics pipeline");

	SetVkObjectName((uint64_t)vulkanGraphicsPipeline->m_vkPipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, graphicsShader->GetDebugName());

	vkDestroyRenderPass(m_vkDevice, vkCompatibleRenderPass, nullptr);

	return vulkanGraphicsPipeline;
}

ICrComputePipeline* CrRenderDeviceVulkan::CreateComputePipelinePS
(
	const CrComputePipelineDescriptor& /*pipelineDescriptor*/,
	const ICrComputeShader* computeShader
)
{
	CrComputePipelineVulkan* vulkanComputePipeline = new CrComputePipelineVulkan();

	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shaderStage.module = static_cast<const CrComputeShaderVulkan*>(computeShader)->GetVkShaderModule();
	shaderStage.pName = computeShader->GetBytecode()->GetEntryPoint().c_str();

	const CrShaderBindingLayoutVulkan& bindingTable = static_cast<const CrShaderBindingLayoutVulkan&>(computeShader->GetBindingLayout());

	VkResult vkResult;

	// Pipeline Resource Layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = 1; // TODO Handle when we have more than one layout
	pipelineLayoutCreateInfo.pSetLayouts = &bindingTable.m_vkDescriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
	// TODO Push constants? Need to be part of the psoDescriptor?

	vkResult = vkCreatePipelineLayout(m_vkDevice, &pipelineLayoutCreateInfo, nullptr, &vulkanComputePipeline->m_vkPipelineLayout);
	CrAssertMsg(vkResult == VK_SUCCESS, "Failed to create compute pipeline layout");

	VkComputePipelineCreateInfo computePipelineCreateInfo = {};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.stage = shaderStage;
	computePipelineCreateInfo.layout = vulkanComputePipeline->m_vkPipelineLayout;

	vkResult = vkCreateComputePipelines(m_vkDevice, m_vkPipelineCache, 1, &computePipelineCreateInfo, nullptr, &vulkanComputePipeline->m_vkPipeline);
	CrAssertMsg(vkResult == VK_SUCCESS, "Failed to create compute pipeline");

	SetVkObjectName((uint64_t)vulkanComputePipeline->m_vkPipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, computeShader->GetDebugName());

	return vulkanComputePipeline;
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
	// them later via CreateCommandQueue(). This is different to what DX12/Metal do in that we need to allocate them ourselves up front.
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

	// We allocate queues up front, which are later retrieved via CreateCommandQueues. We don't really
	// allocate command queues on demand, we have them cached within the device at creation time. This
	// scheme is a bit inflexible, and it means that the CrCommandQueue needs to relay this information
	// back to the render device.
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

	return vkResult;
}

bool CrRenderDeviceVulkan::IsVkDeviceExtensionSupported(const CrString& extension)
{
	return m_supportedDeviceExtensions.count(extension) > 0;
}

bool CrRenderDeviceVulkan::IsDepthStencilFormatSupported(VkFormat depthFormat)
{
	return m_supportedDepthStencilFormats.count(depthFormat) > 0;
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

void CrRenderDeviceVulkan::SetVkObjectName(uint64_t vkObject, VkDebugReportObjectTypeEXT objectType, const CrFixedString128& name) const
{
	if (vkDebugMarkerSetObjectName && !name.empty())
	{
		VkDebugMarkerObjectNameInfoEXT nameInfo = {};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = objectType;
		nameInfo.object = vkObject;
		nameInfo.pObjectName = name.c_str();
		vkDebugMarkerSetObjectName(m_vkDevice, &nameInfo);
	}
}

bool CrRenderDeviceVulkan::GetIsFeatureSupported(CrRenderingFeature::T feature) const
{
	switch (feature)
	{
		case CrRenderingFeature::Tessellation:
			return m_vkDeviceSupportedFeatures.tessellationShader;
		case CrRenderingFeature::GeometryShaders:
			return m_vkDeviceSupportedFeatures.geometryShader;
		case CrRenderingFeature::TextureCompressionBC:
			return m_vkDeviceSupportedFeatures.textureCompressionBC;
		case CrRenderingFeature::TextureCompressionETC:
			return m_vkDeviceSupportedFeatures.textureCompressionETC2;
		case CrRenderingFeature::TextureCompressionASTC:
			return m_vkDeviceSupportedFeatures.textureCompressionASTC_LDR;
	}

	return false;
}
