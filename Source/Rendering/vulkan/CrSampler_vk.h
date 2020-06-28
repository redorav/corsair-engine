#pragma once

#include "ICrSampler.h"
#include <vulkan/vulkan.h>

class ICrRenderDevice;

class CrSamplerVulkan final : public ICrSampler
{
public:

	~CrSamplerVulkan();

	VkSampler GetVkSampler() const { return m_sampler; }

	CrSamplerVulkan(ICrRenderDevice* renderDevice, const CrSamplerDescriptor& descriptor);

private:

	VkDevice m_vkDevice;

	VkSampler m_sampler;
};