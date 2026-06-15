#include "Graphics/CrRendering_pch.h"
#include "SamplerVulkan.h"
#include "DeviceVulkan.h"
#include "CrVulkan.h"

#include "Math/CrMath.h"

namespace crgfx
{
	// See https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkSamplerCreateInfo.html
	SamplerVulkan::SamplerVulkan(crgfx::IDevice* renderDevice, const crgfx::SamplerDescriptor& descriptor) : ISampler(renderDevice)
	{
		crgfx::DeviceVulkan* vulkanRenderDevice = static_cast<crgfx::DeviceVulkan*>(renderDevice);

		VkDevice vkDevice = vulkanRenderDevice->GetVkDevice();

		VkSamplerCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

		createInfo.minFilter = crvk::GetVkFilter(descriptor.minFilter);
		createInfo.magFilter = crvk::GetVkFilter(descriptor.magFilter);

		createInfo.mipmapMode = crvk::GetVkMipmapMode(descriptor.mipmapFilter);

		createInfo.addressModeU = crvk::GetVkAddressMode(descriptor.addressModeU);
		createInfo.addressModeV = crvk::GetVkAddressMode(descriptor.addressModeV);
		createInfo.addressModeW = crvk::GetVkAddressMode(descriptor.addressModeW);

		createInfo.anisotropyEnable = descriptor.enableAnisotropy;

		if (descriptor.enableAnisotropy)
		{
			createInfo.maxAnisotropy = (float)CrMax(1u, descriptor.maxAnisotropy); // TODO Clamp to hardware max limit
		}

		createInfo.compareEnable = descriptor.enableCompare;
		createInfo.compareOp = crvk::GetVkCompareOp(descriptor.compareOp);

		createInfo.borderColor = crvk::GetVkBorderColor(descriptor.borderColor);

		createInfo.mipLodBias = descriptor.mipLodBias;
		createInfo.minLod = descriptor.minLod;
		createInfo.maxLod = descriptor.maxLod;

		createInfo.unnormalizedCoordinates = false; // Not all APIs support it and it has many restrictions in Vulkan (e.g. no anisotropy)

		vkCreateSampler(vkDevice, &createInfo, nullptr, &m_vkSampler);

		vulkanRenderDevice->SetVkObjectName((uint64_t)m_vkSampler, VK_OBJECT_TYPE_SAMPLER, descriptor.name.c_str());
	}

	SamplerVulkan::~SamplerVulkan()
	{
		vkDestroySampler(static_cast<crgfx::DeviceVulkan*>(m_renderDevice)->GetVkDevice(), m_vkSampler, nullptr);
	}
};