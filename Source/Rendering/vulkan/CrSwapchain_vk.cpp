#include "CrRendering_pch.h"

#include "CrSwapchain_vk.h"
#include "CrCommandQueue_vk.h"
#include "CrTexture_vk.h"
#include "CrRenderDevice_vk.h"
#include "CrGPUSynchronization_vk.h"
#include "CrVulkan.h"

#include "Core/Logging/ICrDebug.h"

CrSwapchainVulkan::CrSwapchainVulkan(ICrRenderDevice* renderDevice, const CrSwapchainDescriptor& swapchainDescriptor)
	: ICrSwapchain(renderDevice, swapchainDescriptor)
{
	CrRenderDeviceVulkan* vulkanDevice = static_cast<CrRenderDeviceVulkan*>(renderDevice);

	m_vkInstance = vulkanDevice->GetVkInstance();
	m_vkDevice = vulkanDevice->GetVkDevice();
	m_vkPhysicalDevice = vulkanDevice->GetVkPhysicalDevice();

	VkResult result;

	// Create a surface for the supplied window handle
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = (HINSTANCE)swapchainDescriptor.platformHandle;
	surfaceCreateInfo.hwnd = (HWND)swapchainDescriptor.platformWindow;
	result = vkCreateWin32SurfaceKHR(m_vkInstance, &surfaceCreateInfo, nullptr, &m_vkSurface);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.window = (ANativeWindow*)swapchainDescriptor.platformWindow;
	result = vkCreateAndroidSurfaceKHR(m_vkInstance, &surfaceCreateInfo, nullptr, &m_vkSurface);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.connection = connection;
	surfaceCreateInfo.window = *(xcb_window_t*)swapchainDescriptor.platformWindow;
	surfaceCreateInfo.connection = (xcb_connection_t*)swapchainDescriptor.platformHandle;
	result = vkCreateXcbSurfaceKHR(m_vkInstance, &surfaceCreateInfo, nullptr, &m_vkSurface);
#elif defined(VK_USE_PLATFORM_VI_NN) // Nintendo Switch
	VkViSurfaceCreateInfoNN surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_VI_SURFACE_CREATE_INFO_NN;
	surfaceCreateInfo.window = *(nn::vi::NativeWindowHandle*)swapchainDescriptor.platformWindow;
	result = vkCreateViSurfaceNN(m_vkInstance, &surfaceCreateInfo, nullptr, &m_vkSurface);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
	VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
	surfaceCreateInfo.pView = swapchainDescriptor.platformWindow;
	result = vkCreateMacOSSurfaceMVK(m_vkInstance, &surfaceCreateInfo, nullptr, &m_vkSurface);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
	VkIOSSurfaceCreateInfoMVK surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
	surfaceCreateInfo.pView = swapchainDescriptor.platformWindow;
	result = vkCreateIOSSurfaceMVK(m_vkInstance, &surfaceCreateInfo, nullptr, &m_vkSurface);
#endif

	CrAssertMsg(result == VK_SUCCESS, "Surface creation failed");

	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueFamilyCount, nullptr);

	// We make an assumption here that Graphics queues can always present. The reason is that
	// in Vulkan it's awkward to query whether a queue can present without creating a surface first.
	// https://vulkan.gpuinfo.org/ seems to validate this assumption, but the validation layers will complain if we don't call this.
	CrVector<VkBool32> supportsPresent(queueFamilyCount);
	for (uint32_t i = 0; i < queueFamilyCount; i++)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(m_vkPhysicalDevice, i, m_vkSurface, supportsPresent.data());
	}

	// Get list of supported color formats and spaces for the surface (backbuffer)
	{
		uint32_t formatCount;
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_vkPhysicalDevice, m_vkSurface, &formatCount, nullptr);
		CrVector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_vkPhysicalDevice, m_vkSurface, &formatCount, surfaceFormats.data());
		CrAssertMsg(result == VK_SUCCESS, "Could not retrieve surface formats");

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

		if (!foundMatchingFormat)
		{
			m_vkFormat = surfaceFormats[0].format;
			m_format = crvk::GetDataFormat(m_vkFormat);
			m_vkColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		}
	}

	VkSwapchainKHR oldSwapchain = m_vkSwapchain;

	// Get physical device surface properties and formats
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_vkPhysicalDevice, m_vkSurface, &surfaceCapabilities);
	CrAssertMsg(result == VK_SUCCESS, "Could not retrieve surface capabilities");

	uint32_t presentModeCount;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_vkPhysicalDevice, m_vkSurface, &presentModeCount, nullptr);

	CrVector<VkPresentModeKHR> presentModes(presentModeCount);
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_vkPhysicalDevice, m_vkSurface, &presentModeCount, presentModes.data());
	CrAssertMsg(result == VK_SUCCESS, "Could not retrieve surface present modes");

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

	CrAssertMsg(m_width > 0, "Must have a width greater than 0!");
	CrAssertMsg(m_height > 0, "Must have a height greater than 0!");

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

	VkSwapchainCreateInfoKHR swapchainInfo;
	swapchainInfo.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.pNext                 = nullptr;
	swapchainInfo.flags                 = 0;
	swapchainInfo.surface               = m_vkSurface;
	swapchainInfo.minImageCount         = desiredNumberOfSwapchainImages;
	swapchainInfo.imageFormat           = m_vkFormat;
	swapchainInfo.imageColorSpace       = m_vkColorSpace;
	swapchainInfo.imageExtent           = { swapchainExtent.width, swapchainExtent.height };
	swapchainInfo.imageArrayLayers      = 1;
	swapchainInfo.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
	swapchainInfo.queueFamilyIndexCount = 0;
	swapchainInfo.pQueueFamilyIndices   = nullptr;
	swapchainInfo.preTransform          = (VkSurfaceTransformFlagBitsKHR)preTransform;
	swapchainInfo.presentMode           = swapchainPresentMode;
	swapchainInfo.oldSwapchain          = oldSwapchain;
	swapchainInfo.clipped               = true;
	swapchainInfo.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	result = vkCreateSwapchainKHR(m_vkDevice, &swapchainInfo, nullptr, &m_vkSwapchain);
	CrAssertMsg(result == VK_SUCCESS, "Swapchain creation failed");

	// If we just re-created an existing swapchain, we should destroy the old swapchain at this point.
	// Note: destroying the swapchain also cleans up all its associated presentable images once the platform is done with them.
	if (oldSwapchain != nullptr)
	{
		vkDestroySwapchainKHR(m_vkDevice, oldSwapchain, nullptr);
	}

	result = vkGetSwapchainImagesKHR(m_vkDevice, m_vkSwapchain, &m_imageCount, nullptr);

	CrVector<VkImage> images(m_imageCount);
	result = vkGetSwapchainImagesKHR(m_vkDevice, m_vkSwapchain, &m_imageCount, images.data());

	m_textures = CrVector<CrTextureSharedHandle>(m_imageCount);

	CrTextureDescriptor swapchainTextureParams;
	swapchainTextureParams.width = m_width;
	swapchainTextureParams.height = m_height;
	swapchainTextureParams.format = m_format;
	swapchainTextureParams.name = "Swapchain Texture";
	swapchainTextureParams.usage = cr3d::TextureUsage::SwapChain;

	for (uint32_t i = 0; i < m_imageCount; i++)
	{
		swapchainTextureParams.extraDataPtr = images[i]; // Swapchain texture
		m_textures[i] = renderDevice->CreateTexture(swapchainTextureParams);
	}

	CreatePresentSemaphores(m_imageCount);
}

CrSwapchainVulkan::~CrSwapchainVulkan()
{
	vkDestroySwapchainKHR(m_vkDevice, m_vkSwapchain, nullptr);

	vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, nullptr);
}

CrSwapchainResult CrSwapchainVulkan::AcquireNextImagePS(const ICrGPUSemaphore* signalSemaphore, uint64_t timeoutNanoseconds)
{
	VkResult res = vkAcquireNextImageKHR(m_vkDevice, m_vkSwapchain, timeoutNanoseconds, static_cast<const CrGPUSemaphoreVulkan*>(signalSemaphore)->GetVkSemaphore(), (VkFence)nullptr, &m_currentBufferIndex);

	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
	{
		return CrSwapchainResult::Invalid;
	}

	return CrSwapchainResult::Success;
}

void CrSwapchainVulkan::PresentPS(ICrCommandQueue* queue, const ICrGPUSemaphore* waitSemaphore)
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_vkSwapchain;
	presentInfo.pImageIndices = &m_currentBufferIndex;

	if (waitSemaphore)
	{
		presentInfo.pWaitSemaphores = &static_cast<const CrGPUSemaphoreVulkan*>(waitSemaphore)->GetVkSemaphore();
		presentInfo.waitSemaphoreCount = 1;
	}

	vkQueuePresentKHR(static_cast<CrCommandQueueVulkan*>(queue)->GetVkQueue(), &presentInfo);
}