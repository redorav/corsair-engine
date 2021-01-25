#include "CrRendering_pch.h"

#include "CrRenderDevice_vk.h"

#include "CrCommandQueue_vk.h"
#include "CrCommandBuffer_vk.h"
#include "CrTexture_vk.h"
#include "CrSampler_vk.h"
#include "CrSwapchain_vk.h"
#include "CrFramebuffer_vk.h"
#include "CrRenderPass_vk.h"
#include "CrGPUSynchronization_vk.h"
#include "CrShader_vk.h"

#include "Core/CrCommandLine.h"

#include "Core/Logging/ICrDebug.h"

PFN_vkDebugMarkerSetObjectTagEXT	vkDebugMarkerSetObjectTag = nullptr;
PFN_vkDebugMarkerSetObjectNameEXT	vkDebugMarkerSetObjectName = nullptr;
PFN_vkCmdDebugMarkerBeginEXT		vkCmdDebugMarkerBegin = nullptr;
PFN_vkCmdDebugMarkerEndEXT			vkCmdDebugMarkerEnd = nullptr;
PFN_vkCmdDebugMarkerInsertEXT		vkCmdDebugMarkerInsert = nullptr;

CrRenderDeviceVulkan::CrRenderDeviceVulkan()
	: m_numCommandQueues(0)
{

}

CrRenderDeviceVulkan::~CrRenderDeviceVulkan()
{

}

void CrRenderDeviceVulkan::InitPS()
{
	// TODO Move this to platform-independent layer
	bool enableValidationLayer = crcore::CommandLine["-debugGraphics"];

	bool enableRenderdoc = crcore::CommandLine["-renderdoc"];
	if (enableRenderdoc)
	{
		m_instanceLayers.push_back("VK_LAYER_RENDERDOC_Capture");
	}

	// 1. Create the Vulkan instance
	// TODO Move this to RenderSystem
	CreateInstance(enableValidationLayer);

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

	if (IsVkDeviceExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
	{
		vkDebugMarkerSetObjectTag	= (PFN_vkDebugMarkerSetObjectTagEXT)	vkGetDeviceProcAddr(m_vkDevice, "vkDebugMarkerSetObjectTagEXT");
		vkDebugMarkerSetObjectName	= (PFN_vkDebugMarkerSetObjectNameEXT)	vkGetDeviceProcAddr(m_vkDevice, "vkDebugMarkerSetObjectNameEXT");
		vkCmdDebugMarkerBegin		= (PFN_vkCmdDebugMarkerBeginEXT)		vkGetDeviceProcAddr(m_vkDevice, "vkCmdDebugMarkerBeginEXT");
		vkCmdDebugMarkerEnd			= (PFN_vkCmdDebugMarkerEndEXT)			vkGetDeviceProcAddr(m_vkDevice, "vkCmdDebugMarkerEndEXT");
		vkCmdDebugMarkerInsert		= (PFN_vkCmdDebugMarkerInsertEXT)		vkGetDeviceProcAddr(m_vkDevice, "vkCmdDebugMarkerInsertEXT");
	}

	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	VkResult result = vkCreatePipelineCache(m_vkDevice, &pipelineCacheCreateInfo, nullptr, &m_vkPipelineCache);
	CrAssertMsg(result == VK_SUCCESS, "Failed to create pipeline cache");
}

cr3d::GPUWaitResult CrRenderDeviceVulkan::WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds)
{
	VkResult result = vkWaitForFences(m_vkDevice, 1, &static_cast<const CrGPUFenceVulkan*>(fence)->GetVkFence(), true, timeoutNanoseconds);

	switch (result)
	{
		case VK_SUCCESS: return cr3d::GPUWaitResult::Success;
		case VK_TIMEOUT: return cr3d::GPUWaitResult::Timeout;
		default: return cr3d::GPUWaitResult::Error;
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

VkResult CrRenderDeviceVulkan::CreateInstance(bool enableValidationLayer)
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Corsair Engine";		// TODO Come from application settings
	appInfo.pEngineName = "Corsair Engine";				// TODO Come from engine settings
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	// Enumerate instance extensions
	{
		uint32_t numInstanceExtensions;
		vkEnumerateInstanceExtensionProperties(nullptr, &numInstanceExtensions, nullptr);
		CrVector<VkExtensionProperties> instanceExtensions(numInstanceExtensions);
		vkEnumerateInstanceExtensionProperties(nullptr, &numInstanceExtensions, instanceExtensions.data());

		for (const VkExtensionProperties& extension : instanceExtensions)
		{
			m_supportedInstanceExtensions.insert(extension.extensionName);
		}
	}

	// Enumerate instance layers
	{
		uint32_t numInstanceLayers;
		vkEnumerateInstanceLayerProperties(&numInstanceLayers, nullptr);
		CrVector<VkLayerProperties> instanceLayers(numInstanceLayers);
		vkEnumerateInstanceLayerProperties(&numInstanceLayers, instanceLayers.data());

		for (const VkLayerProperties& layer : instanceLayers)
		{
			m_supportedInstanceLayers.insert(layer.layerName);
		}
	}

	CrVector<const char*> enabledExtensions;
	if (IsVkInstanceExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME))
	{
		enabledExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
	if (IsVkInstanceExtensionSupported(VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
	{
		enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	}
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	if (IsVkInstanceExtensionSupported(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME))
	{
		enabledExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
	}
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	if (IsVkInstanceExtensionSupported(VK_KHR_XCB_SURFACE_EXTENSION_NAME))
	{
		enabledExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
	}
#elif defined(VK_USE_PLATFORM_VI_NN)
	if (IsVkInstanceExtensionSupported(VK_NN_VI_SURFACE_EXTENSION_NAME))
	{
		enabledExtensions.push_back(VK_NN_VI_SURFACE_EXTENSION_NAME);
	}
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
	if (IsVkInstanceExtensionSupported(VK_MVK_MACOS_SURFACE_EXTENSION_NAME))
	{
		enabledExtensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
	}
#elif defined(VK_USE_PLATFORM_IOS_MVK)
	if (IsVkInstanceExtensionSupported(VK_MVK_IOS_SURFACE_EXTENSION_NAME))
	{
		enabledExtensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
	}
#endif

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	if (enableValidationLayer && IsVkInstanceExtensionSupported(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
	{
		enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

	if (enableValidationLayer && IsVkInstanceLayerSupported("VK_LAYER_KHRONOS_validation"))
	{
		m_instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
	}

	instanceCreateInfo.enabledLayerCount = (uint32_t)m_instanceLayers.size();
	instanceCreateInfo.ppEnabledLayerNames = m_instanceLayers.data();

	VkResult res = vkCreateInstance(&instanceCreateInfo, nullptr, &m_vkInstance);

	CrAssertMsg(res == VK_SUCCESS, "Error creating vkInstance");

	return res;
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
	uint32_t highestPriority = 0; highestPriority;
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

	vkGetPhysicalDeviceProperties(m_vkPhysicalDevice, &m_vkPhysicalDeviceProperties);

	// Populate the render device properties into the platform-independent structure
	m_renderDeviceProperties.maxConstantBufferRange = m_vkPhysicalDeviceProperties.limits.maxUniformBufferRange;
	m_renderDeviceProperties.maxTextureDimension1D = m_vkPhysicalDeviceProperties.limits.maxImageDimension1D;
	m_renderDeviceProperties.maxTextureDimension2D = m_vkPhysicalDeviceProperties.limits.maxImageDimension2D;
	m_renderDeviceProperties.maxTextureDimension3D = m_vkPhysicalDeviceProperties.limits.maxImageDimension3D;

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

ICrFramebuffer* CrRenderDeviceVulkan::CreateFramebufferPS(const CrFramebufferCreateParams& params)
{
	return new CrFramebufferVulkan(this, params);
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

ICrHardwareGPUBuffer* CrRenderDeviceVulkan::CreateHardwareGPUBufferPS(const CrGPUBufferDescriptor& descriptor)
{
	return new CrHardwareGPUBufferVulkan(this, descriptor);
}

ICrRenderPass* CrRenderDeviceVulkan::CreateRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor)
{
	return new CrRenderPassVulkan(this, renderPassDescriptor);
}

ICrSampler* CrRenderDeviceVulkan::CreateSamplerPS(const CrSamplerDescriptor& descriptor)
{
	return new CrSamplerVulkan(this, descriptor);
}

ICrSwapchain* CrRenderDeviceVulkan::CreateSwapchainPS(const CrSwapchainDescriptor& swapchainDescriptor)
{
	return new CrSwapchainVulkan(this, swapchainDescriptor);
}

ICrTexture* CrRenderDeviceVulkan::CreateTexturePS(const CrTextureCreateParams& params)
{
	return new CrTextureVulkan(this, params);
}

ICrGraphicsPipeline* CrRenderDeviceVulkan::CreateGraphicsPipelinePS
(
	const CrGraphicsPipelineDescriptor& pipelineDescriptor,
	const ICrGraphicsShader* graphicsShader,
	const CrVertexDescriptor& vertexDescriptor,
	const CrRenderPassDescriptor& renderPassDescriptor
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

	uint32_t numRenderTargets = pipelineDescriptor.blendState.numRenderTargets;

	CrVector<VkPipelineColorBlendAttachmentState> blendAttachments(numRenderTargets);
	for (uint32_t i = 0; i < numRenderTargets; ++i)
	{
		const CrRenderTargetBlendDescriptor& renderTargetBlend = pipelineDescriptor.blendState.renderTargetBlends[i];
		blendAttachments[i].colorWriteMask = renderTargetBlend.colorWriteMask; // TODO Careful with this, needs a platform-specific translation
		blendAttachments[i].blendEnable = renderTargetBlend.enable;
		if (renderTargetBlend.enable)
		{
			blendAttachments[i].colorBlendOp = crvk::GetVkBlendOp(renderTargetBlend.colorBlendOp);
			blendAttachments[i].dstColorBlendFactor = crvk::GetVkBlendFactor(renderTargetBlend.dstColorBlendFactor);
			blendAttachments[i].srcColorBlendFactor = crvk::GetVkBlendFactor(renderTargetBlend.srcColorBlendFactor);

			blendAttachments[i].alphaBlendOp = crvk::GetVkBlendOp(renderTargetBlend.alphaBlendOp);
			blendAttachments[i].dstAlphaBlendFactor = crvk::GetVkBlendFactor(renderTargetBlend.dstAlphaBlendFactor);
			blendAttachments[i].srcAlphaBlendFactor = crvk::GetVkBlendFactor(renderTargetBlend.srcAlphaBlendFactor);
		}
	}

	colorBlendState.attachmentCount = numRenderTargets;
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
	multisampleState.rasterizationSamples = crvk::GetVkSampleCount(pipelineDescriptor.multisampleState.sampleCount);
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

	depthStencilState.front.depthFailOp = crvk::GetVkStencilOp(pipelineDescriptor.depthStencilState.front.depthFailOp);
	depthStencilState.front.failOp = crvk::GetVkStencilOp(pipelineDescriptor.depthStencilState.front.stencilFailOp);
	depthStencilState.front.passOp = crvk::GetVkStencilOp(pipelineDescriptor.depthStencilState.front.stencilPassOp);
	depthStencilState.front.compareOp = crvk::GetVkCompareOp(pipelineDescriptor.depthStencilState.front.stencilCompareOp);
	depthStencilState.front.compareMask = pipelineDescriptor.depthStencilState.front.stencilReadMask;
	depthStencilState.front.writeMask = pipelineDescriptor.depthStencilState.front.stencilWriteMask;
	depthStencilState.front.reference = pipelineDescriptor.depthStencilState.front.reference;

	depthStencilState.back.depthFailOp = crvk::GetVkStencilOp(pipelineDescriptor.depthStencilState.back.depthFailOp);
	depthStencilState.back.failOp = crvk::GetVkStencilOp(pipelineDescriptor.depthStencilState.back.stencilFailOp);
	depthStencilState.back.passOp = crvk::GetVkStencilOp(pipelineDescriptor.depthStencilState.back.stencilPassOp);
	depthStencilState.back.compareOp = crvk::GetVkCompareOp(pipelineDescriptor.depthStencilState.back.stencilCompareOp);
	depthStencilState.back.compareMask = pipelineDescriptor.depthStencilState.back.stencilReadMask;
	depthStencilState.back.writeMask = pipelineDescriptor.depthStencilState.back.stencilWriteMask;
	depthStencilState.back.reference = pipelineDescriptor.depthStencilState.back.reference;

	depthStencilState.minDepthBounds = 0.0f;
	depthStencilState.maxDepthBounds = 1.0f;

	// Viewport state
	VkPipelineViewportStateCreateInfo viewportState;
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;
	viewportState.flags = 0;
	viewportState.viewportCount = 1; // One viewport
	viewportState.pViewports = nullptr;
	viewportState.scissorCount = 1; // One scissor rectangle
	viewportState.pScissors = nullptr;

	// Dynamic states can be set even after the pipeline has been created, so there is no need to create new pipelines
	// just for changing a viewport's dimensions or a scissor box
	// The dynamic state properties themselves are stored in the command buffer
	CrArray<VkDynamicState, 2> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicState;
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pNext = nullptr;
	dynamicState.flags = 0;
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = (uint32_t)dynamicStateEnables.size();

	VkResult vkResult;

	// Shader information
	VkPipelineShaderStageCreateInfo shaderStages[cr3d::ShaderStage::GraphicsStageCount] = {};
	VkPipelineShaderStageCreateInfo shaderStageInfo = {};
	shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

	const CrGraphicsShaderVulkan* vulkanGraphicsShader = static_cast<const CrGraphicsShaderVulkan*>(graphicsShader);

	const CrVector<VkShaderModule>& vkShaderModules = vulkanGraphicsShader->GetVkShaderModules();

	const CrVector<CrShaderStageInfo>& stageInfos = vulkanGraphicsShader->GetStages();

	uint32_t usedShaderStages = 0;

	for (uint32_t i = 0; i < stageInfos.size(); ++i)
	{
		const CrShaderStageInfo& stageInfo = stageInfos[i];
		const VkShaderModule vkShaderModule = vkShaderModules[i];

		shaderStageInfo.module = vkShaderModule;
		shaderStageInfo.stage = crvk::GetVkShaderStage(stageInfo.stage);
		shaderStageInfo.pName = stageInfo.entryPoint.c_str();
		shaderStages[usedShaderStages++] = shaderStageInfo;
	}

	const CrShaderBindingTableVulkan& resourceTable = static_cast<const CrShaderBindingTableVulkan&>(graphicsShader->GetBindingTable());

	// Pipeline Resource Layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.flags = 0;
	pipelineLayoutCreateInfo.setLayoutCount = 1; // TODO Handle when we have more than one layout
	pipelineLayoutCreateInfo.pSetLayouts = &resourceTable.m_vkDescriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
	// TODO Push constants? Need to be part of the psoDescriptor?

	vkResult = vkCreatePipelineLayout(m_vkDevice, &pipelineLayoutCreateInfo, nullptr, &vulkanGraphicsPipeline->m_vkPipelineLayout);

	CrAssertMsg(vkResult == VK_SUCCESS, "Failed to create pipeline layout");

	// Binding description
	CrVector<VkVertexInputBindingDescription> bindingDescriptions;
	bindingDescriptions.resize(1);
	bindingDescriptions[0].binding = 0; // TODO Shader binding location
	bindingDescriptions[0].stride = vertexDescriptor.GetDataSize();
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	uint32_t offset = 0;

	// Create vertex input state
	CrVector<VkVertexInputAttributeDescription> attributeDescriptions;

	attributeDescriptions.resize(vertexDescriptor.GetNumAttributes());

	for (uint32_t i = 0; i < vertexDescriptor.GetNumAttributes(); ++i)
	{
		const cr3d::DataFormatInfo& vertexFormatInfo = vertexDescriptor.GetVertexInfo(i);
		attributeDescriptions[i].binding = 0; // TODO Shader binding location
		attributeDescriptions[i].location = i;
		attributeDescriptions[i].format = crvk::GetVkFormat(vertexFormatInfo.format);
		attributeDescriptions[i].offset = offset;
		offset += vertexFormatInfo.dataOrBlockSize;
	}

	// Assign to vertex buffer
	VkPipelineVertexInputStateCreateInfo vertexInputState;
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.pNext = nullptr;
	vertexInputState.flags = 0;
	vertexInputState.vertexBindingDescriptionCount = (uint32_t)bindingDescriptions.size();
	vertexInputState.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputState.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
	vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();

	// We don't need the real renderpass here to create pipeline state objects, a compatible one is enough
	// to make it work. I suspect the only thing it really needs is the description of how we're going to
	// be using the render pass. In D3D12 this is actually part of the psoDescriptor (RTVFormats, DSVFormat)
	// TODO This should probably be optimized either by having another function that passes an actual render pass
	// or by having a cache of render pass descriptors. The reason for using a dummy render pass is so that
	// pipelines can be precreated without needing access to the actual draw commands.
	CrRenderPassVulkan dummyRenderPass = CrRenderPassVulkan(this, renderPassDescriptor);

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = (uint32_t)graphicsShader->GetStages().size();
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.layout = vulkanGraphicsPipeline->m_vkPipelineLayout; // TODO Hack
	pipelineInfo.pVertexInputState = &vertexInputState;				// TODO Create this pipeline layout first from the shader/vertex descriptor
	pipelineInfo.renderPass = dummyRenderPass.GetVkRenderPass();

	pipelineInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineInfo.pRasterizationState = &rasterizerState;
	pipelineInfo.pColorBlendState = &colorBlendState;
	pipelineInfo.pMultisampleState = &multisampleState;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pDepthStencilState = &depthStencilState;
	pipelineInfo.pDynamicState = &dynamicState;

	vkResult = vkCreateGraphicsPipelines(m_vkDevice, m_vkPipelineCache, 1, &pipelineInfo, nullptr, &vulkanGraphicsPipeline->m_vkPipeline);
	CrAssertMsg(vkResult == VK_SUCCESS, "Failed to create graphics pipeline");

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
	shaderStage.pName = computeShader->m_stageInfo.entryPoint.c_str();

	const CrShaderBindingTableVulkan& bindingTable = static_cast<const CrShaderBindingTableVulkan&>(computeShader->GetBindingTable());

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

	VkComputePipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.stage = shaderStage;
	pipelineInfo.layout = vulkanComputePipeline->m_vkPipelineLayout;

	vkResult = vkCreateComputePipelines(m_vkDevice, m_vkPipelineCache, 1, &pipelineInfo, nullptr, &vulkanComputePipeline->m_vkPipeline);
	CrAssertMsg(vkResult == VK_SUCCESS, "Failed to create compute pipeline");

	return vulkanComputePipeline;
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

	VkResult result = vkCreateDevice(m_vkPhysicalDevice, &deviceCreateInfo, nullptr, &m_vkDevice);
	CrAssertMsg(result == VK_SUCCESS, "Could not create vkDevice");

	return result;
}

bool CrRenderDeviceVulkan::IsVkDeviceExtensionSupported(const CrString& extension)
{
	return m_supportedDeviceExtensions.count(extension) > 0;
}

bool CrRenderDeviceVulkan::IsVkInstanceExtensionSupported(const CrString& extension)
{
	return m_supportedInstanceExtensions.count(extension) > 0;
}

bool CrRenderDeviceVulkan::IsVkInstanceLayerSupported(const CrString& layer)
{
	return m_supportedInstanceLayers.count(layer) > 0;
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
