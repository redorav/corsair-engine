#pragma once

#include "Graphics/ICrSampler.h"
#include <vulkan/vulkan.h>

class ICrRenderDevice;

class CrSamplerVulkan final : public ICrSampler
{
public:

	CrSamplerVulkan(crgfx::ICrRenderDevice* renderDevice, const CrSamplerDescriptor& descriptor);

	~CrSamplerVulkan();

	VkSampler GetVkSampler() const { return m_vkSampler; }

private:

	VkSampler m_vkSampler;
};