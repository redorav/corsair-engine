#include "Rendering/CrRendering_pch.h"

#include "CrSwapchain_vk.h"
#include "CrTexture_vk.h"
#include "CrRenderDevice_vk.h"
#include "CrGPUSynchronization_vk.h"
#include "CrVulkan.h"

#include "Core/Logging/ICrDebug.h"

#include "Math/CrMath.h"

CrSwapchainVulkan::CrSwapchainVulkan(ICrRenderDevice* renderDevice, const CrSwapchainDescriptor& swapchainDescriptor)
	: ICrSwapchain(renderDevice, swapchainDescriptor)

	// The initialization value is important to start at 0 on the first call to present
	, m_currentSemaphoreIndex((uint32_t)-1)
{
	CrRenderDeviceVulkan* vulkanDevice = static_cast<CrRenderDeviceVulkan*>(renderDevice);

	VkInstance vkInstance = vulkanDevice->GetVkInstance();
	VkDevice vkDevice = vulkanDevice->GetVkDevice();
	VkPhysicalDevice vkPhysicalDevice = vulkanDevice->GetVkPhysicalDevice();

	VkResult vkResult;

	// Create a surface for the supplied window handle
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = (HINSTANCE)swapchainDescriptor.platformHandle;
	surfaceCreateInfo.hwnd = (HWND)swapchainDescriptor.platformWindow;
	vkResult = vkCreateWin32SurfaceKHR(vkInstance, &surfaceCreateInfo, nullptr, &m_vkSurface);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.window = (ANativeWindow*)swapchainDescriptor.platformWindow;
	vkResult = vkCreateAndroidSurfaceKHR(vkInstance, &surfaceCreateInfo, nullptr, &m_vkSurface);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.connection = connection;
	surfaceCreateInfo.window = *(xcb_window_t*)swapchainDescriptor.platformWindow;
	surfaceCreateInfo.connection = (xcb_connection_t*)swapchainDescriptor.platformHandle;
	vkResult = vkCreateXcbSurfaceKHR(vkInstance, &surfaceCreateInfo, nullptr, &m_vkSurface);
#elif defined(VK_USE_PLATFORM_VI_NN) // Nintendo Switch
	VkViSurfaceCreateInfoNN surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_VI_SURFACE_CREATE_INFO_NN;
	surfaceCreateInfo.window = *(nn::vi::NativeWindowHandle*)swapchainDescriptor.platformWindow;
	vkResult = vkCreateViSurfaceNN(vkInstance, &surfaceCreateInfo, nullptr, &m_vkSurface);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
	VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
	surfaceCreateInfo.pView = swapchainDescriptor.platformWindow;
	vkResult = vkCreateMacOSSurfaceMVK(vkInstance, &surfaceCreateInfo, nullptr, &m_vkSurface);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
	VkIOSSurfaceCreateInfoMVK surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
	surfaceCreateInfo.pView = swapchainDescriptor.platformWindow;
	vkResult = vkCreateIOSSurfaceMVK(vkInstance, &surfaceCreateInfo, nullptr, &m_vkSurface);
#endif

	CrAssertMsg(vkResult == VK_SUCCESS, "Surface creation failed");

	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);

	// We make an assumption here that Graphics queues can always present. The reason is that
	// in Vulkan it's awkward to query whether a queue can present without creating a surface first.
	// https://vulkan.gpuinfo.org/ seems to validate this assumption, but the validation layers will complain if we don't call this.
	CrVector<VkBool32> supportsPresent(queueFamilyCount);
	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, m_vkSurface, supportsPresent.data());
	}

	// Get list of supported color formats and spaces for the surface (backbuffer)
	{
		uint32_t formatCount;
		vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, m_vkSurface, &formatCount, nullptr);
		CrAssertMsg(vkResult == VK_SUCCESS, "Could not retrieve surface formats");

		CrVector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, m_vkSurface, &formatCount, surfaceFormats.data());
		CrAssertMsg(vkResult == VK_SUCCESS, "Could not retrieve surface formats");

		CrAssertMsg(formatCount > 0, "Must have at least one preferred color space");

		VkFormat desiredVkFormat = crvk::GetVkFormat(swapchainDescriptor.format);

		bool foundMatchingFormat = false;

		for (uint32_t i = 0; i < surfaceFormats.size(); ++i)
		{
			if (surfaceFormats[i].format == desiredVkFormat)
			{
				m_format = swapchainDescriptor.format;
				m_vkFormat = surfaceFormats[i].format;
				m_vkColorSpace = surfaceFormats[i].colorSpace;
				foundMatchingFormat = true;
				break;
			}
		}

		CrAssertMsg(foundMatchingFormat, "Could not create swapchain with requested format");
	}

	// Get physical device surface properties and formats
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, m_vkSurface, &surfaceCapabilities);
	CrAssertMsg(vkResult == VK_SUCCESS, "Could not retrieve surface capabilities");

	uint32_t presentModeCount;
	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, m_vkSurface, &presentModeCount, nullptr);
	CrAssertMsg(vkResult == VK_SUCCESS, "Could not retrieve present modes count");

	CrVector<VkPresentModeKHR> presentModes(presentModeCount);
	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, m_vkSurface, &presentModeCount, presentModes.data());
	CrAssertMsg(vkResult == VK_SUCCESS, "Could not retrieve surface present modes");

	VkExtent2D swapchainExtent;

	// currentExtent is the current width and height of the surface, or the special value (0xFFFFFFFF, 0xFFFFFFFF) 
	// indicating that the surface size will be determined by the extent of a swapchain targeting the surface.
	if (surfaceCapabilities.currentExtent.width == 0xffffffff)
	{
		// If the surface size is undefined, the size is set to the size of the images requested.
		m_width = swapchainExtent.width = swapchainDescriptor.requestedWidth;
		m_height = swapchainExtent.height = swapchainDescriptor.requestedHeight;
	}
	else
	{
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfaceCapabilities.currentExtent;
		m_width = surfaceCapabilities.currentExtent.width;
		m_height = surfaceCapabilities.currentExtent.height;
	}

	CrAssertMsg(m_width > 0, "Must have a width greater than 0");
	CrAssertMsg(m_height > 0, "Must have a height greater than 0");

	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_MAX_ENUM_KHR;

	bool hasMailbox = false;
	bool hasImmediate = false;
	bool hasRelaxedFifo = false;

	for (size_t i = 0; i < presentModeCount; i++)
	{
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) { hasMailbox = true; }
		if (presentModes[i] == VK_PRESENT_MODE_FIFO_RELAXED_KHR) { hasRelaxedFifo = true; }
		if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) { hasImmediate = true; }
	}

	// FIFO modes are effectively vsync, whereas mailbox and immediate are essentially uncapped
	// Mailbox is non-tearing whereas immediate can tear
	if (hasMailbox)
	{
		// Try to use mailbox mode. Low latency and non-tearing. It will push frames before the
		// vsync and will only use the last one (effectively discarding an entire, unpresented, frame)
		swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	}
	else if (hasRelaxedFifo)
	{
		// Try to vsync, but if frame comes in late, present (and potentially tear)
		swapchainPresentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
	}
	else if (hasImmediate)
	{
		// Present as fast as possible. May cause tearing
		swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	}
	else
	{
		// FIFO is required so is always present as fallback
		swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	}

	// Determine the number of images
	uint32_t desiredNumberOfSwapchainImages = CrMax(surfaceCapabilities.minImageCount, swapchainDescriptor.requestedBufferCount);
	if (surfaceCapabilities.maxImageCount > 0)
	{
		desiredNumberOfSwapchainImages = CrMin(desiredNumberOfSwapchainImages, surfaceCapabilities.maxImageCount);
	}

	VkSurfaceTransformFlagsKHR preTransform;
	if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surfaceCapabilities.currentTransform;
	}

	m_vkSwapchainCreateInfo.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	m_vkSwapchainCreateInfo.pNext                 = nullptr;
	m_vkSwapchainCreateInfo.flags                 = 0;
	m_vkSwapchainCreateInfo.surface               = m_vkSurface;
	m_vkSwapchainCreateInfo.minImageCount         = desiredNumberOfSwapchainImages;
	m_vkSwapchainCreateInfo.imageFormat           = m_vkFormat;
	m_vkSwapchainCreateInfo.imageColorSpace       = m_vkColorSpace;
	m_vkSwapchainCreateInfo.imageExtent           = { swapchainExtent.width, swapchainExtent.height };
	m_vkSwapchainCreateInfo.imageArrayLayers      = 1;
	m_vkSwapchainCreateInfo.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	m_vkSwapchainCreateInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
	m_vkSwapchainCreateInfo.queueFamilyIndexCount = 0;
	m_vkSwapchainCreateInfo.pQueueFamilyIndices   = nullptr;
	m_vkSwapchainCreateInfo.preTransform          = (VkSurfaceTransformFlagBitsKHR)preTransform;
	m_vkSwapchainCreateInfo.presentMode           = swapchainPresentMode;
	m_vkSwapchainCreateInfo.oldSwapchain          = nullptr;
	m_vkSwapchainCreateInfo.clipped               = true;
	m_vkSwapchainCreateInfo.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	vkResult = vkCreateSwapchainKHR(vkDevice, &m_vkSwapchainCreateInfo, nullptr, &m_vkSwapchain);
	CrAssertMsg(vkResult == VK_SUCCESS, "Swapchain creation failed");
	
	vkResult = vkGetSwapchainImagesKHR(vkDevice, m_vkSwapchain, &m_imageCount, nullptr);
	CrAssertMsg(vkResult == VK_SUCCESS, "Could not retrieve swapchain images");

	VkFenceCreateInfo vkFenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	vkCreateFence(vkDevice, &vkFenceCreateInfo, nullptr, &m_swapchainRecreationFence);

	CreateSwapchainTextures();

	m_presentCompleteSemaphores.resize(m_imageCount);

	for (CrGPUSemaphoreHandle& waitSemaphore : m_presentCompleteSemaphores)
	{
		waitSemaphore = m_renderDevice->CreateGPUSemaphore();
	}
}

CrSwapchainVulkan::~CrSwapchainVulkan()
{
	VkInstance vkInstance = static_cast<CrRenderDeviceVulkan*>(m_renderDevice)->GetVkInstance();
	VkDevice vkDevice = static_cast<CrRenderDeviceVulkan*>(m_renderDevice)->GetVkDevice();

	vkDestroySwapchainKHR(vkDevice, m_vkSwapchain, nullptr);

	vkDestroySurfaceKHR(vkInstance, m_vkSurface, nullptr);

	vkDestroyFence(vkDevice, m_swapchainRecreationFence, nullptr);
}

CrSwapchainResult CrSwapchainVulkan::AcquireNextImagePS(uint64_t timeoutNanoseconds)
{
	m_currentSemaphoreIndex = (m_currentSemaphoreIndex + 1) % m_imageCount;

	const ICrGPUSemaphore* signalSemaphore = m_presentCompleteSemaphores[m_currentSemaphoreIndex].get();

	// Acquire image signals the semaphore we pass in
	VkResult res = vkAcquireNextImageKHR(static_cast<CrRenderDeviceVulkan*>(m_renderDevice)->GetVkDevice(), m_vkSwapchain, timeoutNanoseconds, 
		static_cast<const CrGPUSemaphoreVulkan*>(signalSemaphore)->GetVkSemaphore(), (VkFence)nullptr, &m_currentBufferIndex);

	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
	{
		return CrSwapchainResult::Invalid;
	}

	return CrSwapchainResult::Success;
}

void CrSwapchainVulkan::PresentPS()
{
	VkQueue vkQueue = static_cast<CrRenderDeviceVulkan*>(m_renderDevice)->GetVkQueue(CrCommandQueueType::Graphics);
	CrGPUSemaphoreVulkan* waitSemaphore = static_cast<CrGPUSemaphoreVulkan*>(m_presentCompleteSemaphores[m_currentSemaphoreIndex].get());

	// Present image (swap buffers)
	// TODO Put this on the device and batch submit swapchains
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_vkSwapchain;
	presentInfo.pImageIndices = &m_currentBufferIndex;
	presentInfo.pWaitSemaphores = &waitSemaphore->GetVkSemaphore();
	presentInfo.waitSemaphoreCount = 1;
	vkQueuePresentKHR(vkQueue, &presentInfo);
}

void CrSwapchainVulkan::ResizePS(uint32_t width, uint32_t height)
{
	CrRenderDeviceVulkan* vulkanDevice = static_cast<CrRenderDeviceVulkan*>(m_renderDevice);
	VkDevice vkDevice = vulkanDevice->GetVkDevice();
	VkPhysicalDevice vkPhysicalDevice = vulkanDevice->GetVkPhysicalDevice();

	VkResult vkResult = VK_SUCCESS;

	m_width = width;
	m_height = height;

	// Call this function again to silence the validation layer about resolution mismatches. I think it has the values cached and
	// checks against those, which are the old ones. What we can do is assert that the width and height are not larger
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, m_vkSurface, &surfaceCapabilities);
	CrAssertMsg(vkResult == VK_SUCCESS, "Could not retrieve surface capabilities");
	CrAssertMsg(width <= surfaceCapabilities.maxImageExtent.width && height <= surfaceCapabilities.maxImageExtent.height, "Resolution larger than max extent");

	// Reuse the previously created swapchain create info. This has all the properties we need
	// distilled into this structure, without needing to go through the entire creation process
	VkSwapchainKHR oldSwapchain = m_vkSwapchain;
	m_vkSwapchainCreateInfo.imageExtent = { width, height };
	m_vkSwapchainCreateInfo.oldSwapchain = m_vkSwapchain;

	vkResult = vkCreateSwapchainKHR(vkDevice, &m_vkSwapchainCreateInfo, nullptr, &m_vkSwapchain);
	CrAssertMsg(vkResult == VK_SUCCESS, "Swapchain creation failed");

	// If we just re-created an existing swapchain, we should destroy the old swapchain at this point.
	// Note: destroying the swapchain also cleans up all its associated presentable images once the platform is done with them.
	// This is unlike D3D12 that needs the images to be explicitly cleared
	if (oldSwapchain != nullptr)
	{
		vkDestroySwapchainKHR(vkDevice, oldSwapchain, nullptr);
	}

	CreateSwapchainTextures();
}

void CrSwapchainVulkan::CreateSwapchainTextures()
{
	CrRenderDeviceVulkan* vulkanDevice = static_cast<CrRenderDeviceVulkan*>(m_renderDevice);

	CrFixedVector<VkImage, 8> images(m_imageCount);
	VkResult vkResult = vkGetSwapchainImagesKHR(vulkanDevice->GetVkDevice(), m_vkSwapchain, &m_imageCount, images.data());
	CrAssertMsg(vkResult == VK_SUCCESS, "Could not retrieve swapchain images");

	vkWaitForFences(vulkanDevice->GetVkDevice(), 1, &m_swapchainRecreationFence, VK_TRUE, UINT64_MAX);

	m_textures = CrVector<CrTextureHandle>(m_imageCount);

	CrTextureDescriptor swapchainTextureParams;
	swapchainTextureParams.width = m_width;
	swapchainTextureParams.height = m_height;
	swapchainTextureParams.format = m_format;
	swapchainTextureParams.usage = cr3d::TextureUsage::SwapChain;

	CrFixedVector<VkImageMemoryBarrier, 8> imageMemoryBarriers(m_imageCount);

	for (uint32_t i = 0; i < m_imageCount; i++)
	{
		CrFixedString128 swapchainTextureName(m_name);
		swapchainTextureName.append_sprintf(" Texture %i", i);
		swapchainTextureParams.name = swapchainTextureName.c_str();
		swapchainTextureParams.extraDataPtr = images[i]; // Swapchain texture
		m_textures[i] = vulkanDevice->CreateTexture(swapchainTextureParams);

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;

		imageMemoryBarriers[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarriers[i].image = images[i];
		imageMemoryBarriers[i].subresourceRange = subresourceRange;
		imageMemoryBarriers[i].srcAccessMask = VK_ACCESS_NONE;
		imageMemoryBarriers[i].dstAccessMask = VK_ACCESS_NONE;

		imageMemoryBarriers[i].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageMemoryBarriers[i].newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		imageMemoryBarriers[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarriers[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	}

	// Transition swapchain textures. We don't do that in the CreateTexture as we need
	// a more immediate response to window resize changes. We use a special command
	// buffer to queue only these transitions
	VkCommandBuffer swapchainCommandBuffer = vulkanDevice->GetVkSwapchainCommandBuffer();

	VkCommandBufferBeginInfo commandBufferInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	vkBeginCommandBuffer(swapchainCommandBuffer, &commandBufferInfo);
	vkCmdPipelineBarrier(vulkanDevice->GetVkSwapchainCommandBuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, (uint32_t)imageMemoryBarriers.size(), imageMemoryBarriers.data());
	vkEndCommandBuffer(swapchainCommandBuffer);

	vkResetFences(vulkanDevice->GetVkDevice(), 1, &m_swapchainRecreationFence);

	// Submit to the queue immediately
	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &swapchainCommandBuffer;
	VkResult result = vkQueueSubmit(vulkanDevice->GetVkQueue(CrCommandQueueType::Graphics), 1, &submitInfo, m_swapchainRecreationFence);
	CrAssert(result == VK_SUCCESS);
}
