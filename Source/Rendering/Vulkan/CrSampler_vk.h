#pragma once

#include "Rendering/ICrSampler.h"
#include <vulkan/vulkan.h>

class ICrRenderDevice;

class CrSamplerVulkan final : public ICrSampler
{
public:

	CrSamplerVulkan(ICrRenderDevice* renderDevice, const CrSamplerDescriptor& descriptor);

	~CrSamplerVulkan();

	VkSampler GetVkSampler() const { return m_vkSampler; }

private:

	VkSampler m_vkSampler;
};