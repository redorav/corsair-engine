#include "CrRendering_pch.h"

#include "CrRenderDevice_vk.h"

#include "CrCommandQueue_vk.h"
#include "CrCommandBuffer_vk.h"
#include "CrTexture_vk.h"
#include "CrSampler_vk.h"
#include "CrShaderManager_vk.h"
#include "CrShaderGen.h" // TODO remove
#include "CrSwapchain_vk.h"
#include "CrFramebuffer_vk.h"
#include "CrRenderPass_vk.h"
#include "CrGPUStackAllocator_vk.h"
#include "CrGPUSynchronization_vk.h"

#include "ICrPipelineStateManager.h"
#include "ICrFramebuffer.h"

#include "CrInputManager.h" // TODO HACK Remove
#include "CrMaterial.h" // TODO Hack remove
#include "CrRenderModel.h" // TODO Hack remove
#include "CrMesh.h" // TODO Hack remove
#include "CrResourceManager.h" // TODO Hack remove
#include "CrCamera.h" // todo hack

#include "Core/CrCommandLine.h"
#include "Core/CrTime.h"

#include "Core/Logging/ICrDebug.h"

#include "CrCPUStackAllocator.h"

PFN_vkDebugMarkerSetObjectTagEXT	vkDebugMarkerSetObjectTag = nullptr;
PFN_vkDebugMarkerSetObjectNameEXT	vkDebugMarkerSetObjectName = nullptr;
PFN_vkCmdDebugMarkerBeginEXT		vkCmdDebugMarkerBegin = nullptr;
PFN_vkCmdDebugMarkerEndEXT			vkCmdDebugMarkerEnd = nullptr;
PFN_vkCmdDebugMarkerInsertEXT		vkCmdDebugMarkerInsert = nullptr;

// HACK Delete these
CrGraphicsShaderCreate g_shaderCreateInfo;
CrSamplerSharedHandle g_samplerHandle;

// TODO Delete these
CrCamera camera;
Camera cameraConstantData;

CrRenderDeviceVulkan::CrRenderDeviceVulkan()
	: m_setupCmdBuffer(nullptr)
	, m_numCommandQueues(0)
{

}

CrRenderDeviceVulkan::~CrRenderDeviceVulkan()
{

}

void CrRenderDeviceVulkan::InitPS
(
	void* platformHandle, void* platformWindow
)
{
	// TODO All of this code needs to be separated into PS functions so that we can have a common flow between different platforms

	VkResult result;

	bool enableValidationLayer = crcore::CommandLine["-debugGraphics"];
	if (enableValidationLayer)
	{
		// This is a meta layer that enables all of the standard validation layers in the correct order :
		// threading, parameter_validation, device_limits, object_tracker, image, core_validation, swapchain, and unique_objects
		m_instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
	}

	bool enableRenderdoc = crcore::CommandLine["-renderdoc"];
	if (enableRenderdoc)
	{
		m_instanceLayers.push_back("VK_LAYER_RENDERDOC_Capture");
	}

	// 1. Create the Vulkan instance
	CreateInstance(enableValidationLayer);

	// 2. Create the physical devices (can be multi-GPU)
	CreatePhysicalDevices();

	// 3. Create the rendering surface
	CreateSurface(platformHandle, platformWindow);

	// 4. Query queue families
	RetrieveQueueFamilies();
	
	// 5. Create logical device. Connects the physical device to a vkDevice.
	// Also specifies desired queues.
	CreateLogicalDevice(enableValidationLayer);

	// 6. Get the hardware supported formats, memory types, etc.
	QueryDeviceProperties();

	// 7. Create main command queue. This will take care of the main command buffers and present
	m_mainCommandQueue = CreateCommandQueue(CrCommandQueueType::Graphics);

	// 7. Create the swapchain
	m_swapChain = CreateSwapchain(m_width, m_height);

	// Create one command buffer for submitting the post present image memory barrier
	m_setupCmdBuffer = m_mainCommandQueue->CreateCommandBuffer();

	m_setupCmdBuffer->Begin();
	{
		CrTextureCreateParams depthTexParams;
		depthTexParams.width = m_swapChain->GetWidth();
		depthTexParams.height = m_swapChain->GetHeight();
		depthTexParams.format = cr3d::DataFormat::D24_Unorm_S8_Uint;
		depthTexParams.usage = cr3d::TextureUsage::Depth | cr3d::TextureUsage::RenderTarget;

		m_depthStencilTexture = CreateTexture(depthTexParams); // Create the depth buffer
	}
	m_setupCmdBuffer->End();

	m_setupCmdBuffer->Submit();

	m_drawCmdBuffers.resize(m_swapChain->GetImageCount());
	for (uint32_t i = 0; i < m_swapChain->GetImageCount(); ++i)
	{
		m_drawCmdBuffers[i] = m_mainCommandQueue->CreateCommandBuffer();
	}

	// TODO REMOVE
	SetupRenderPass();
	SetupSwapchainFramebuffer();

	RecreateSwapchain();

	result = vkQueueWaitIdle(static_cast<CrCommandQueueVulkan*>(m_mainCommandQueue.get())->m_vkQueue); // TODO Make function and remove weird casts

	if (IsDeviceExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
	{
		vkDebugMarkerSetObjectTag	= (PFN_vkDebugMarkerSetObjectTagEXT)	vkGetDeviceProcAddr(m_vkDevice, "vkDebugMarkerSetObjectTagEXT");
		vkDebugMarkerSetObjectName	= (PFN_vkDebugMarkerSetObjectNameEXT)	vkGetDeviceProcAddr(m_vkDevice, "vkDebugMarkerSetObjectNameEXT");
		vkCmdDebugMarkerBegin		= (PFN_vkCmdDebugMarkerBeginEXT)		vkGetDeviceProcAddr(m_vkDevice, "vkCmdDebugMarkerBeginEXT");
		vkCmdDebugMarkerEnd			= (PFN_vkCmdDebugMarkerEndEXT)			vkGetDeviceProcAddr(m_vkDevice, "vkCmdDebugMarkerEndEXT");
		vkCmdDebugMarkerInsert		= (PFN_vkCmdDebugMarkerInsertEXT)		vkGetDeviceProcAddr(m_vkDevice, "vkCmdDebugMarkerInsertEXT");
	}

	//vkFreeCommandBuffers(m_device, m_cmdPool, 1, &m_setupCmdBufferCr->m_commandBuffer);
	//m_setupCmdBufferCr->m_commandBuffer = nullptr; // todo : check if still necessary

	// Semaphore used to ensures that all commands submitted have been finished before submitting the image to the queue
	m_renderCompleteSemaphore = new CrGPUSemaphoreVulkan(this);

	// Semaphore used to ensures that image presentation is complete before starting to submit again
	m_presentCompleteSemaphore = new CrGPUSemaphoreVulkan(this);

	prepareVertices();
	preparePipelines();
}

void CrRenderDeviceVulkan::WaitForFencePS(const CrGPUFenceVulkan* fence, uint64_t timeoutNanoseconds)
{
	vkWaitForFences(m_vkDevice, 1, &fence->GetVkFence(), true, timeoutNanoseconds);
}

void CrRenderDeviceVulkan::ResetFencePS(const CrGPUFenceVulkan* fence)
{
	vkResetFences(m_vkDevice, 1, &fence->GetVkFence());
}

void CrRenderDeviceVulkan::PresentPS()
{
	CrSwapchainResult scResult = m_swapChain->AcquireNextImage(m_presentCompleteSemaphore, UINT64_MAX);

	if (scResult == CrSwapchainResult::Invalid)
	{
		RecreateSwapchain();
		scResult = m_swapChain->AcquireNextImage(m_presentCompleteSemaphore, UINT64_MAX);
	}

	CrGPUFenceVulkan* swapchainFence = static_cast<CrGPUFenceVulkan*>(m_swapChain->GetCurrentWaitFence().get());

	WaitForFencePS(swapchainFence, UINT64_MAX);

	ResetFencePS(swapchainFence);

	ICrCommandBuffer* drawCommandBuffer = m_drawCmdBuffers[m_swapChain->GetCurrentFrameIndex()];

	{
		drawCommandBuffer->Begin();
		drawCommandBuffer->SetViewport(CrViewport(0.0f, 0.0f, (float)m_swapChain->GetWidth(), (float)m_swapChain->GetHeight()));
		drawCommandBuffer->SetScissor(0, 0, m_swapChain->GetWidth(), m_swapChain->GetHeight());
		
		CrRenderPassBeginParams renderPassParams;
		renderPassParams.clear = true;
		renderPassParams.colorClearValue = float4(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f);
		renderPassParams.depthClearValue = 0.0f;
		renderPassParams.stencilClearValue = 0;
		renderPassParams.drawArea.x = 0;
		renderPassParams.drawArea.y = 0;
		renderPassParams.drawArea.width = m_swapChain->GetWidth();
		renderPassParams.drawArea.height = m_swapChain->GetHeight();

		drawCommandBuffer->BeginDebugEvent("RenderPass 1", float4(1.0f, 0.0, 1.0f, 1.0f));
		{
			drawCommandBuffer->BeginRenderPass(m_renderPass.get(), m_frameBuffers[m_swapChain->GetCurrentFrameIndex()].get(), renderPassParams);
			{
				drawCommandBuffer->BindGraphicsPipelineState(m_pipelineTriangleState);
				
				updateCamera();

				CrGPUBufferType<Color> colorBuffer = drawCommandBuffer->AllocateConstantBuffer<Color>();
				Color* theColorData2 = colorBuffer.Lock();
				{
					theColorData2->color = float4(1.0f, 1.0f, 1.0f, 1.0f);
					theColorData2->tint2 = float4(1.0f, 1.0f, 1.0f, 1.0f);
				}
				colorBuffer.Unlock();
				drawCommandBuffer->BindConstantBuffer(&colorBuffer);

				CrGPUBufferType<DynamicLight> dynamicLightBuffer = drawCommandBuffer->AllocateConstantBuffer<DynamicLight>();
				DynamicLight* dynamicLightBufferData = dynamicLightBuffer.Lock();
				{
					dynamicLightBufferData->positionRadius = float4(1.0f, 1.0f, 1.0f, 0.0f);
					dynamicLightBufferData->color = float4(1.0f, 0.25f, 0.25f, 0.0f);
				}
				dynamicLightBuffer.Unlock();
				drawCommandBuffer->BindConstantBuffer(&dynamicLightBuffer);

				CrGPUBufferType<Camera> cameraDataBuffer = drawCommandBuffer->AllocateConstantBuffer<Camera>();
				Camera* cameraData2 = cameraDataBuffer.Lock();
				{
					*cameraData2 = cameraConstantData;
				}
				cameraDataBuffer.Unlock();
				drawCommandBuffer->BindConstantBuffer(&cameraDataBuffer);

				drawCommandBuffer->BindSampler(cr3d::ShaderStage::Pixel, Samplers::AllLinearClampSampler, g_samplerHandle.get());

				for (uint32_t m = 0; m < m_renderModel->m_renderMeshes.size(); ++m)
				{
					const CrRenderMeshSharedHandle& renderMesh = m_renderModel->m_renderMeshes[m];
					uint32_t materialIndex = (*m_renderModel->m_materialMap.find(renderMesh.get())).second;
					const CrMaterialSharedHandle& material = m_renderModel->m_materials[materialIndex];
				
					for (uint32_t t = 0; t < material->m_textures.size(); ++t)
					{
						CrMaterial::TextureBinding binding = material->m_textures[t];
						drawCommandBuffer->BindTexture(cr3d::ShaderStage::Pixel, binding.semantic, binding.texture.get());
					}
					
					drawCommandBuffer->BindVertexBuffer(renderMesh->m_vertexBuffer.get(), 0);
					drawCommandBuffer->BindIndexBuffer(renderMesh->m_indexBuffer.get());
					drawCommandBuffer->DrawIndexed(renderMesh->m_indexBuffer->GetNumElements(), 1, 0, 0, 0);
				}
			}
			drawCommandBuffer->EndRenderPass(m_renderPass.get());
		}
		drawCommandBuffer->EndDebugEvent();

		drawCommandBuffer->End();
	}

	drawCommandBuffer->Submit(m_presentCompleteSemaphore, m_renderCompleteSemaphore, m_swapChain->GetCurrentWaitFence().get());

	m_swapChain->Present(m_mainCommandQueue.get(), m_renderCompleteSemaphore);
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

		for (const VkExtensionProperties& extension : instanceExtensions) { m_supportedInstanceExtensions.insert(extension.extensionName); }
	}

	CrVector<const char*> enabledExtensions;
	if (IsInstanceExtensionSupported(VK_KHR_SURFACE_EXTENSION_NAME))
	{
		enabledExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
	if (IsInstanceExtensionSupported(VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
	{
		enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	}
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	if (IsInstanceExtensionSupported(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME))
	{
		enabledExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
	}
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	if (IsInstanceExtensionSupported(VK_KHR_XCB_SURFACE_EXTENSION_NAME))
	{
		enabledExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
	}
#elif defined(VK_USE_PLATFORM_VI_NN)
	if (IsInstanceExtensionSupported(VK_NN_VI_SURFACE_EXTENSION_NAME))
	{
		enabledExtensions.push_back(VK_NN_VI_SURFACE_EXTENSION_NAME);
	}
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
	if (IsInstanceExtensionSupported(VK_MVK_MACOS_SURFACE_EXTENSION_NAME))
	{
		enabledExtensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
	}
#elif defined(VK_USE_PLATFORM_IOS_MVK)
	if (IsInstanceExtensionSupported(VK_MVK_IOS_SURFACE_EXTENSION_NAME))
	{
		enabledExtensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
	}
#endif

	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	CrVector<VkLayerProperties> layers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	if (enableValidationLayer && IsInstanceExtensionSupported(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
	{
		enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

	instanceCreateInfo.enabledLayerCount = (uint32_t) m_instanceLayers.size();
	instanceCreateInfo.ppEnabledLayerNames = m_instanceLayers.data();

	VkResult res = vkCreateInstance(&instanceCreateInfo, nullptr, &m_vkInstance);

	return res;
}

VkResult CrRenderDeviceVulkan::CreateSurface(void* platformHandle, void* platformWindow)
{
	VkResult result;
	// Create a surface for the supplied window handle
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = (HINSTANCE)platformHandle;
	surfaceCreateInfo.hwnd = (HWND)platformWindow;
	result = vkCreateWin32SurfaceKHR(m_vkInstance, &surfaceCreateInfo, nullptr, &m_vkSurface);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.window = platformWindow;
	result = vkCreateAndroidSurfaceKHR(m_vkInstance, &surfaceCreateInfo, nullptr, &m_vkSurface);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.connection = connection;
	surfaceCreateInfo.window = platformWindow;
	result = vkCreateXcbSurfaceKHR(m_vkInstance, &surfaceCreateInfo, nullptr, &m_vkSurface);
#elif defined(VK_USE_PLATFORM_VI_NN) // Nintendo Switch
	VkViSurfaceCreateInfoNN surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_VI_SURFACE_CREATE_INFO_NN;
	surfaceCreateInfo.window = platformWindow;
	result = vkCreateViSurfaceNN(m_vkInstance, &surfaceCreateInfo, nullptr, &m_vkSurface);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
	VkMacOSSurfaceCreateFlagsMVK surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
	surfaceCreateInfo.pView = platformWindow;
	result = vkCreateMacOSSurfaceMVK(m_vkInstance, &surfaceCreateInfo, nullptr, &m_vkSurface);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
	VkIOSSurfaceCreateFlagsMVK surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
	surfaceCreateInfo.pView = platformWindow;
	result = vkCreateIOSSurfaceMVK(m_vkInstance, &surfaceCreateInfo, nullptr, &m_vkSurface);
#endif

	return result;
}

VkResult CrRenderDeviceVulkan::CreatePhysicalDevices()
{
	VkResult result;

	uint32_t gpuCount = 0; // TODO Put in header file of RenderSystem
	result = vkEnumeratePhysicalDevices(m_vkInstance, &gpuCount, nullptr);
	CrAssertMsg(result == VK_SUCCESS && gpuCount > 0, "No GPUs found!");

	CrVector<VkPhysicalDevice> physicalDevices(gpuCount);
	result = vkEnumeratePhysicalDevices(m_vkInstance, &gpuCount, physicalDevices.data());

	m_vkPhysicalDevice = physicalDevices[0]; // TODO needs mutiple GPU handling

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

ICrRenderPass* CrRenderDeviceVulkan::CreateRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor)
{
	return new CrRenderPassVulkan(this, renderPassDescriptor);
}

ICrSampler* CrRenderDeviceVulkan::CreateSamplerPS(const CrSamplerDescriptor& descriptor)
{
	return new CrSamplerVulkan(this, descriptor);
}

ICrSwapchain* CrRenderDeviceVulkan::CreateSwapchainPS(uint32_t requestedWidth, uint32_t requestedHeight)
{
	return new CrSwapchainVulkan(this, requestedWidth, requestedHeight);
}

ICrTexture* CrRenderDeviceVulkan::CreateTexturePS(const CrTextureCreateParams& params)
{
	return new CrTextureVulkan(this, params);
}

ICrGPUStackAllocator* CrRenderDeviceVulkan::CreateGPUMemoryStreamPS()
{
	return new CrGPUStackAllocatorVulkan(this);
}

ICrHardwareGPUBuffer* CrRenderDeviceVulkan::CreateHardwareGPUBufferPS(const CrGPUBufferCreateParams& params)
{
	return new CrHardwareGPUBufferVulkan(this, params);
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
	CrVector<QueueProperties> queueProperties(queueFamilyCount);
	CrVector<VkQueueFamilyProperties> vkQueueProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueFamilyCount, vkQueueProperties.data());

	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		VkQueueFamilyProperties& queueProperty = vkQueueProperties[i];
		uint32_t flags = queueProperty.queueFlags;
		queueProperties[i].doesGraphics = (flags & VK_QUEUE_GRAPHICS_BIT) != 0;
		queueProperties[i].doesCompute = (flags & VK_QUEUE_COMPUTE_BIT) != 0;
		queueProperties[i].doesCopy = (flags & VK_QUEUE_TRANSFER_BIT) != 0;
		queueProperties[i].maxQueues = queueProperty.queueCount;
	}

	CrVector<VkBool32> supportsPresent(queueFamilyCount);
	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(m_vkPhysicalDevice, i, m_vkSurface, supportsPresent.data());
		queueProperties[i].doesPresent = supportsPresent[i] == VK_TRUE;
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

VkResult CrRenderDeviceVulkan::CreateLogicalDevice(bool enableValidation)
{
	CrVector<const char*> enabledDeviceExtensions;

	if (IsDeviceExtensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME))
	{
		enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	if (enableValidation && IsDeviceExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
	{
		enabledDeviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
	}

	if (IsDeviceExtensionSupported(VK_KHR_MAINTENANCE1_EXTENSION_NAME))
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

	if (enableValidation)
	{
		deviceCreateInfo.enabledLayerCount = (uint32_t) m_instanceLayers.size();
		deviceCreateInfo.ppEnabledLayerNames = m_instanceLayers.data();
	}

	return vkCreateDevice(m_vkPhysicalDevice, &deviceCreateInfo, nullptr, &m_vkDevice);
}

void CrRenderDeviceVulkan::RecreateSwapchain()
{
	// Ensure all operations on the device have been finished before destroying resources
	vkDeviceWaitIdle(m_vkDevice);

	// 1. Recreate swapchain (creates new and destroys old)

	// We must destroy the old swapchain before creating the new one. Otherwise the API will fail trying to create a resource
	// that becomes available after (once the pointer assignment happens and the resource is destroyed)
	m_swapChain = nullptr;

	m_swapChain = CreateSwapchain(m_width, m_height);

	// 2. Recreate depth stencil texture

	CrTextureCreateParams depthTexParams;
	depthTexParams.width = m_swapChain->GetWidth();
	depthTexParams.height = m_swapChain->GetHeight();
	depthTexParams.format = cr3d::DataFormat::D24_Unorm_S8_Uint;
	depthTexParams.usage = cr3d::TextureUsage::Depth | cr3d::TextureUsage::RenderTarget;

	m_depthStencilTexture = CreateTexture(depthTexParams); // Create the depth buffer

	// 3. Recreate framebuffers

	SetupSwapchainFramebuffer();

	// 4. Recreate command buffers

	for (uint32_t i = 0; i < m_drawCmdBuffers.size(); ++i)
	{
		m_mainCommandQueue->DestroyCommandBuffer(m_drawCmdBuffers[i]);
	}

	m_mainCommandQueue->DestroyCommandBuffer(m_setupCmdBuffer);

	m_drawCmdBuffers.resize(m_swapChain->GetImageCount());
	for (uint32_t i = 0; i < m_swapChain->GetImageCount(); ++i)
	{
		m_drawCmdBuffers[i] = m_mainCommandQueue->CreateCommandBuffer();
	}

	m_setupCmdBuffer = m_mainCommandQueue->CreateCommandBuffer();

	// Make sure all of this work is finished
	vkDeviceWaitIdle(m_vkDevice);
}

bool CrRenderDeviceVulkan::IsInstanceExtensionSupported(const CrString& extension)
{
	return m_supportedInstanceExtensions.count(extension) > 0;
}

bool CrRenderDeviceVulkan::IsDeviceExtensionSupported(const CrString& extension)
{
	return m_supportedDeviceExtensions.count(extension) > 0;
}

bool CrRenderDeviceVulkan::IsDepthStencilFormatSupported(VkFormat depthFormat)
{
	return m_supportedDepthStencilFormats.count(depthFormat) > 0;
}

void CrRenderDeviceVulkan::SetupRenderPass()
{
	CrRenderPassDescriptor renderPassDescriptor;
	renderPassDescriptor.m_colorAttachments[0] = CrAttachmentDescriptor(m_swapChain->GetFormat(), m_swapChain->GetSampleCount(), 
																		CrAttachmentLoadOp::Clear, CrAttachmentStoreOp::Store,
																		CrAttachmentLoadOp::DontCare, CrAttachmentStoreOp::DontCare, 
																		cr3d::ResourceState::Undefined, cr3d::ResourceState::Present);

	renderPassDescriptor.m_depthAttachment = CrAttachmentDescriptor(m_depthStencilTexture->GetFormat(), m_depthStencilTexture->GetSampleCount(), 
																	CrAttachmentLoadOp::Clear, CrAttachmentStoreOp::Store,
																	CrAttachmentLoadOp::DontCare, CrAttachmentStoreOp::DontCare, 
																	cr3d::ResourceState::Undefined, cr3d::ResourceState::PixelShaderInput);
	
	m_renderPass = CreateRenderPass(renderPassDescriptor);
}

void CrRenderDeviceVulkan::SetupSwapchainFramebuffer()
{
	m_frameBuffers.resize(m_swapChain->GetImageCount());

	for (uint32_t i = 0; i < m_frameBuffers.size(); i++)
	{
		CrFramebufferCreateParams frameBufferParams(m_swapChain->GetTexture(i).get(), m_depthStencilTexture.get());
		m_frameBuffers[i] = CreateFramebuffer(frameBufferParams);
	}
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

	CrAssertMsg(queueIndex < m_maxCommandQueues, "No more command queues available!");

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

void CrRenderDeviceVulkan::prepareVertices()
{
	// TODO Remove after final integration
	m_triangleVertexBuffer = ICrRenderDevice::GetRenderDevice()->CreateVertexBuffer<SimpleVertex>((uint32_t)8);

	SimpleVertex* vertexData = (SimpleVertex*)m_triangleVertexBuffer->Lock(); // Setup vertices
	{
		vertexData[0].position = { -0.5_h, -0.5_h, -0.5_h };
		vertexData[0].normal = { 255, 0, 0, 255 };
		vertexData[0].uv = { 1.0_h, 0.0_h };

		vertexData[1].position = { -0.5_h, -0.5_h, 0.5_h };
		vertexData[1].normal = { 0, 255, 0, 255 };
		vertexData[1].uv = { 0.0_h, 0.0_h };

		vertexData[2].position = { -0.5_h, 0.5_h, 0.5_h };
		vertexData[2].normal = { 0, 0, 255, 255 };
		vertexData[2].uv = { 0.0_h, 1.0_h };

		vertexData[3].position = { -0.5_h, 0.5_h, -0.5_h };
		vertexData[3].normal = { 255, 0, 255, 255 };
		vertexData[3].uv = { 1.0_h, 1.0_h };

		vertexData[4].position = { 0.5_h, -0.5_h, -0.5_h };
		vertexData[4].normal = { 255, 255, 0, 255 };
		vertexData[4].uv = { 0.0_h, 0.0_h };

		vertexData[5].position = { 0.5_h, -0.5_h, 0.5_h };
		vertexData[5].normal = { 0, 255, 255, 255 };
		vertexData[5].uv = { 1.0_h, 0.0_h };

		vertexData[6].position = { 0.5_h, 0.5_h, 0.5_h };
		vertexData[6].normal = { 255, 255, 255, 255 };
		vertexData[6].uv = { 1.0_h, 1.0_h };

		vertexData[7].position = { 0.5_h, 0.5_h, -0.5_h };
		vertexData[7].normal = { 0, 0, 0, 255 };
		vertexData[7].uv = { 0.0_h, 1.0_h };
	}
	m_triangleVertexBuffer->Unlock();

	m_triangleIndexBuffer = CreateIndexBuffer(cr3d::DataFormat::R16_Uint, 36);
	uint16_t* data = (uint16_t*)m_triangleIndexBuffer->Lock();
	{
		data[0] = 0; data[3] = 2;
		data[1] = 1; data[4] = 3;
		data[2] = 2; data[5] = 0;

		data[6] = 4; data[9] = 3;
		data[7] = 0; data[10] = 7;
		data[8] = 3; data[11] = 4;

		data[12] = 5; data[15] = 7;
		data[13] = 4; data[16] = 6;
		data[14] = 7; data[17] = 5;

		data[18] = 1; data[21] = 6;
		data[19] = 5; data[22] = 2;
		data[20] = 6; data[23] = 1;

		data[24] = 7; data[27] = 2;
		data[25] = 3; data[28] = 6;
		data[26] = 2; data[29] = 7;

		data[30] = 5; data[33] = 0;
		data[31] = 1; data[34] = 4;
		data[32] = 0; data[35] = 5;
	}
	m_triangleIndexBuffer->Unlock();

	CrResourceManager::LoadModel(m_renderModel, "nyra/nyra_pose_mod.fbx");
	//"jaina/storm_hero_jaina.fbx"
}

void CrRenderDeviceVulkan::updateCamera()
{
	camera.SetupPerspective((float)m_swapChain->GetWidth(), (float)m_swapChain->GetHeight(), 1.0f, 1000.0f);

	float3 currentLookAt = camera.m_lookAt;
	float3 currentRight = camera.m_right;

	// TODO Hack to get a bit of movement on the camera
	if (CrInput.GetKey(KeyCode::A) || CrInput.GetAxis(AxisCode::JoystickLeftAxisX) < 0.0f)
	{
		camera.Translate(currentRight * -5.0f * CrTime::GetFrameDelta());
	}

	if (CrInput.GetKey(KeyCode::D) || CrInput.GetAxis(AxisCode::JoystickLeftAxisX) > 0.0f)
	{
		camera.Translate(currentRight * 5.0f * CrTime::GetFrameDelta());
	}

	if (CrInput.GetKey(KeyCode::W) || CrInput.GetAxis(AxisCode::JoystickLeftAxisY) > 0.0f)
	{
		camera.Translate(currentLookAt * 5.0f * CrTime::GetFrameDelta());
	}

	if (CrInput.GetKey(KeyCode::S) || CrInput.GetAxis(AxisCode::JoystickLeftAxisY) < 0.0f)
	{
		camera.Translate(currentLookAt * -5.0f * CrTime::GetFrameDelta());
	}

	if (CrInput.GetKey(KeyCode::Q) || CrInput.GetAxis(AxisCode::JoystickL2) > 0.0f)
	{
		camera.Translate(float3(0.0f, -5.0f, 0.0f) * CrTime::GetFrameDelta());
	}

	if (CrInput.GetKey(KeyCode::E) || CrInput.GetAxis(AxisCode::JoystickR2) > 0.0f)
	{
		camera.Translate(float3(0.0f, 5.0f, 0.0f) * CrTime::GetFrameDelta());
	}

	if (CrInput.GetAxis(AxisCode::JoystickRightAxisX) > 0.0f)
	{
		//CrLogWarning("Moving right");
		camera.Rotate(float3(0.0f, 2.0f, 0.0f) * CrTime::GetFrameDelta());
		//camera.RotateAround(float3::zero(), float3(0.0f, 1.0f, 0.0f), 0.1f);
		//camera.LookAt(float3::zero(), float3(0, 1, 0));
	}

	if (CrInput.GetAxis(AxisCode::JoystickRightAxisX) < 0.0f)
	{
		//CrLogWarning("Moving left");
		camera.Rotate(float3(0.0f, -2.0f, 0.0f) * CrTime::GetFrameDelta());
		//camera.RotateAround(float3::zero(), float3(0.0f, 1.0f, 0.0f), -0.1f);
		//camera.LookAt(float3::zero(), float3(0, 1, 0));
	}

	camera.Update();

	cameraConstantData.world2View = transpose(camera.GetWorld2ViewMatrix());
	cameraConstantData.view2Projection = transpose(camera.GetView2ProjectionMatrix());
}

void CrRenderDeviceVulkan::preparePipelines()
{
	CrString SHADER_PATH = IN_SRC_PATH;
	SHADER_PATH = SHADER_PATH + "Rendering/Shaders/";
	
	CrSamplerDescriptor descriptor;
	g_samplerHandle = CreateSampler(descriptor);

//#define USE_GLSL
#define USE_HLSL
#if defined(USE_GLSL)

	g_shaderCreateInfo.AddShaderStage(CrGraphicsShaderStageCreate(CrPath((SHADER_PATH + "triangle.vert").c_str()), "main", cr3d::ShaderStage::Vertex, cr3d::ShaderCodeFormat::Source));
	g_shaderCreateInfo.AddShaderStage(CrGraphicsShaderStageCreate(CrPath((SHADER_PATH + "triangle.frag").c_str()), "main", cr3d::ShaderStage::Pixel, cr3d::ShaderCodeFormat::Source));

#elif defined(USE_HLSL)

	g_shaderCreateInfo.AddShaderStage(CrGraphicsShaderStageCreate(CrPath((SHADER_PATH + "triangle.hlsl").c_str()), "main_vs", cr3d::ShaderStage::Vertex, cr3d::ShaderCodeFormat::Source));
	g_shaderCreateInfo.AddShaderStage(CrGraphicsShaderStageCreate(CrPath((SHADER_PATH + "triangle.hlsl").c_str()), "main_ps", cr3d::ShaderStage::Pixel, cr3d::ShaderCodeFormat::Source));

#else

	g_shaderCreateInfo.AddShaderStage(CrGraphicsShaderStageCreate(CrPath((SHADER_PATH + "triangle.vert.spv").c_str()), "main", cr3d::ShaderStage::Vertex, cr3d::ShaderCodeFormat::Binary));
	g_shaderCreateInfo.AddShaderStage(CrGraphicsShaderStageCreate(CrPath((SHADER_PATH + "triangle.frag.spv").c_str()), "main", cr3d::ShaderStage::Pixel, cr3d::ShaderCodeFormat::Binary));

#endif

	// TODO Move block to rendering subsystem initialization function
	{
		ICrShaderManager::Get()->Init();
		ICrPipelineStateManager::Get()->Init(ICrRenderDevice::GetRenderDevice());
	}

	CrGraphicsShaderHandle graphicsShader = ICrShaderManager::Get()->LoadGraphicsShader(g_shaderCreateInfo);
	CrGraphicsPipelineDescriptor psoDescriptor;
	psoDescriptor.Hash();
	graphicsShader->renderPass			= static_cast<const CrRenderPassVulkan*>(m_renderPass.get())->m_vkRenderPass; // TODO Super hack

	// TODO Reminder for next time:
	// 1) Pass in psoDescriptor, vertexInputState (need to encapsulate) and loaded/compiled graphics shader to GetGraphicsPipeline
	// 2) Create hash for all three and combine
	// 3) Do a lookup. If not in table, call CreateGraphicsPipeline with all three again
	// 4) After creation, put in table for next time

	m_pipelineTriangleState = ICrPipelineStateManager::Get()->GetGraphicsPipeline(psoDescriptor, graphicsShader, m_triangleVertexBuffer->m_vertexDescriptor);
	m_pipelineTriangleState = ICrPipelineStateManager::Get()->GetGraphicsPipeline(psoDescriptor, graphicsShader, m_triangleVertexBuffer->m_vertexDescriptor); // Test caching

	psoDescriptor.primitiveTopology = cr3d::PrimitiveTopology::LineList;
	psoDescriptor.Hash();
	m_pipelineLineState = ICrPipelineStateManager::Get()->GetGraphicsPipeline(psoDescriptor, graphicsShader, m_triangleVertexBuffer->m_vertexDescriptor);
}

void CrRenderDeviceVulkan::QueryDeviceProperties()
{
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
	for (uint32_t i = VK_FORMAT_BEGIN_RANGE; i < VK_FORMAT_END_RANGE; ++i)
	{
		VkFormat format = (VkFormat)i;
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
}

bool CrRenderDeviceVulkan::GetIsFeatureSupported(CrRenderingFeature::T feature)
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
