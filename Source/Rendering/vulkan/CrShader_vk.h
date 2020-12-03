#pragma once

#include "Rendering/ICrShader.h"
#include <vulkan/vulkan.h>

class ICrRenderDevice;

class CrGraphicsShaderVulkan final : public ICrGraphicsShader
{
public:

	CrGraphicsShaderVulkan(const ICrRenderDevice* renderDevice, const CrGraphicsShaderDescriptor& graphicsShaderDescriptor);

	~CrGraphicsShaderVulkan();

	const CrVector<VkShaderModule>& GetVkShaderModules() const;

private:

	VkDevice m_vkDevice;

	CrVector<VkShaderModule> m_vkShaderModules;

	VkDescriptorSetLayout m_vkDescriptorSetLayout;

	VkPipelineLayout m_vkPipelineLayout;
};

inline const CrVector<VkShaderModule>& CrGraphicsShaderVulkan::GetVkShaderModules() const
{
	return m_vkShaderModules;
}

class CrComputeShaderVulkan final : public ICrComputeShader
{

	VkShaderModule m_vkShaderModule;

	VkDescriptorSetLayout m_vkDescriptorSetLayout;

	VkPipelineLayout m_vkPipelineLayout;
};