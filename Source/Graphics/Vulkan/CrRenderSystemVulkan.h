#pragma once

#include "Graphics/ICrRenderSystem.h"

#include "crstl/open_hashmap.h"
#include "crstl/string.h"
#include "crstl/vector.h"

class CrRenderSystemVulkan final : public ICrRenderSystem
{
public:

	CrRenderSystemVulkan(const crgfx::GraphicsSystemDescriptor& graphicsSystemDescriptor);

	VkInstance GetVkInstance() const { return m_vkInstance; }

	bool IsVkInstanceExtensionSupported(const crstl::string& extension);

	bool IsVkInstanceLayerSupported(const crstl::string& layer);

	virtual crgfx::IDevice* CreateRenderDevicePS(const crgfx::DeviceDescriptor& descriptor) override;

private:

	// Global per-application state https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#VkInstance
	VkInstance m_vkInstance;

	VkDebugUtilsMessengerEXT m_vkDebugMessenger;

	crstl::open_hashset<crstl::string> m_supportedInstanceExtensions;

	crstl::open_hashset<crstl::string> m_supportedInstanceLayers;

	crstl::vector<const char*> m_instanceLayers;
};