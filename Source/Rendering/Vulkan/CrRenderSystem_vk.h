#pragma once

#include "Rendering/ICrRenderSystem.h"

#include "Core/String/CrString.h"

#include "Core/Containers/CrVector.h"

#include "Core/Containers/CrSet.h"

class CrRenderSystemVulkan final : public ICrRenderSystem
{
public:

	CrRenderSystemVulkan(const CrRenderSystemDescriptor& renderSystemDescriptor);

	VkInstance GetVkInstance() const { return m_vkInstance; }

	bool IsVkInstanceExtensionSupported(const CrString& extension);

	bool IsVkInstanceLayerSupported(const CrString& layer);

	virtual ICrRenderDevice* CreateRenderDevicePS(const CrRenderDeviceDescriptor& descriptor) const override;

private:

	// Global per-application state https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#VkInstance
	VkInstance m_vkInstance;

	VkDebugUtilsMessengerEXT m_vkDebugMessenger;

	CrSet<CrString> m_supportedInstanceExtensions;

	CrSet<CrString> m_supportedInstanceLayers;

	CrVector<const char*> m_instanceLayers;
};