#include "CrRendering_pch.h"

#include "CrRenderSystem_vk.h"

#include "CrRenderDevice_vk.h"

#include "Core/CrMacros.h"

#include "Core/Logging/ICrDebug.h"

PFN_vkSetDebugUtilsObjectNameEXT    vkSetDebugUtilsObjectName    = nullptr;
PFN_vkSetDebugUtilsObjectTagEXT     vkSetDebugUtilsObjectTag     = nullptr;
PFN_vkQueueBeginDebugUtilsLabelEXT  vkQueueBeginDebugUtilsLabel  = nullptr;
PFN_vkQueueEndDebugUtilsLabelEXT    vkQueueEndDebugUtilsLabel    = nullptr;
PFN_vkQueueInsertDebugUtilsLabelEXT vkQueueInsertDebugUtilsLabel = nullptr;
PFN_vkCmdBeginDebugUtilsLabelEXT    vkCmdBeginDebugUtilsLabel    = nullptr;
PFN_vkCmdEndDebugUtilsLabelEXT      vkCmdEndDebugUtilsLabel      = nullptr;
PFN_vkCmdInsertDebugUtilsLabelEXT   vkCmdInsertDebugUtilsLabel   = nullptr;
PFN_vkCreateDebugUtilsMessengerEXT  vkCreateDebugUtilsMessenger  = nullptr;
PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = nullptr;
PFN_vkSubmitDebugUtilsMessageEXT    vkSubmitDebugUtilsMessage    = nullptr;

CrRenderSystemVulkan::CrRenderSystemVulkan(const CrRenderSystemDescriptor& renderSystemDescriptor) : ICrRenderSystem(renderSystemDescriptor)
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

	if (IsVkInstanceExtensionSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
	{
		enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

	if (renderSystemDescriptor.enableValidation)
	{
		if (IsVkInstanceLayerSupported("VK_LAYER_KHRONOS_validation"))
		{
			m_instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
		}
		else
		{
			CrLog("Graphics validation layer VK_LAYER_KHRONOS_validation not found");
		}
	}

	instanceCreateInfo.enabledLayerCount = (uint32_t)m_instanceLayers.size();
	instanceCreateInfo.ppEnabledLayerNames = m_instanceLayers.data();

	VkResult res = vkCreateInstance(&instanceCreateInfo, nullptr, &m_vkInstance);

	CrAssertMsg(res == VK_SUCCESS, "Error creating vkInstance");

	if (IsVkInstanceExtensionSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
	{
		vkSetDebugUtilsObjectName    = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(m_vkInstance, "vkSetDebugUtilsObjectNameEXT");
		vkSetDebugUtilsObjectTag     = (PFN_vkSetDebugUtilsObjectTagEXT)vkGetInstanceProcAddr(m_vkInstance, "vkSetDebugUtilsObjectTagEXT");
		vkQueueBeginDebugUtilsLabel  = (PFN_vkQueueBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(m_vkInstance, "vkQueueBeginDebugUtilsLabelEXT");
		vkQueueEndDebugUtilsLabel    = (PFN_vkQueueEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(m_vkInstance, "vkQueueEndDebugUtilsLabelEXT");
		vkQueueInsertDebugUtilsLabel = (PFN_vkQueueInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(m_vkInstance, "vkQueueInsertDebugUtilsLabelEXT");
		vkCmdBeginDebugUtilsLabel    = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(m_vkInstance, "vkCmdBeginDebugUtilsLabelEXT");
		vkCmdEndDebugUtilsLabel      = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(m_vkInstance, "vkCmdEndDebugUtilsLabelEXT");
		vkCmdInsertDebugUtilsLabel   = (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(m_vkInstance, "vkCmdInsertDebugUtilsLabelEXT");
		vkCreateDebugUtilsMessenger  = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_vkInstance, "vkCreateDebugUtilsMessengerEXT");
		vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_vkInstance, "vkDestroyDebugUtilsMessengerEXT");
		vkSubmitDebugUtilsMessage    = (PFN_vkSubmitDebugUtilsMessageEXT)vkGetInstanceProcAddr(m_vkInstance, "vkSubmitDebugUtilsMessageEXT");
	}
}

bool CrRenderSystemVulkan::IsVkInstanceExtensionSupported(const CrString& extension)
{
	return m_supportedInstanceExtensions.count(extension) > 0;
}

bool CrRenderSystemVulkan::IsVkInstanceLayerSupported(const CrString& layer)
{
	return m_supportedInstanceLayers.count(layer) > 0;
}

ICrRenderDevice* CrRenderSystemVulkan::CreateRenderDevicePS() const
{
	return new CrRenderDeviceVulkan(this);
}
