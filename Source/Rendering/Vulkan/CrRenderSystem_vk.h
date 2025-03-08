#pragma once

#include "Rendering/ICrRenderSystem.h"

#include "Core/Containers/CrHashMap.h"

#include "crstl/string.h"
#include "crstl/vector.h"

class CrRenderSystemVulkan final : public ICrRenderSystem
{
public:

	CrRenderSystemVulkan(const CrRenderSystemDescriptor& renderSystemDescriptor);

	VkInstance GetVkInstance() const { return m_vkInstance; }

	bool IsVkInstanceExtensionSupported(const crstl::string& extension);

	bool IsVkInstanceLayerSupported(const crstl::string& layer);

	virtual ICrRenderDevice* CreateRenderDevicePS(const CrRenderDeviceDescriptor& descriptor) override;

private:

	// Global per-application state https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#VkInstance
	VkInstance m_vkInstance;

	VkDebugUtilsMessengerEXT m_vkDebugMessenger;

	CrHashSet<crstl::string> m_supportedInstanceExtensions;

	CrHashSet<crstl::string> m_supportedInstanceLayers;

	crstl::vector<const char*> m_instanceLayers;
};