#include "CrRendering_pch.h"

#include "CrRenderSystem_vk.h"

#include "CrRenderDevice_vk.h"

#include "Core/CrMacros.h"

#include "Core/Logging/ICrDebug.h"

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

	if (IsVkInstanceExtensionSupported(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
	{
		enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
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
