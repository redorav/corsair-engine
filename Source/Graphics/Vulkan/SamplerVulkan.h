#pragma once

#include "Graphics/ISampler.h"
#include <vulkan/vulkan.h>

class IDevice;

namespace crgfx
{
	class SamplerVulkan final : public ISampler
	{
	public:

		SamplerVulkan(crgfx::IDevice* renderDevice, const crgfx::SamplerDescriptor& descriptor);

		~SamplerVulkan();

		VkSampler GetVkSampler() const { return m_vkSampler; }

	private:

		VkSampler m_vkSampler;
	};
};