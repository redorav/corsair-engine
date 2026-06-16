#pragma once

#include "Graphics/IShader.h"
#include <vulkan/vulkan.h>

class IDevice;

class CrGraphicsShaderVulkan final : public ICrGraphicsShader
{
public:

	CrGraphicsShaderVulkan(crgfx::IDevice* renderDevice, const CrGraphicsShaderDescriptor& graphicsShaderDescriptor);

	~CrGraphicsShaderVulkan();

	const crstl::vector<VkShaderModule>& GetVkShaderModules() const { return m_vkShaderModules; }

	VkDescriptorSetLayout GetVkDescriptorSetLayout() const { return m_vkDescriptorSetLayout; }

private:

	VkDevice m_vkDevice;

	crstl::vector<VkShaderModule> m_vkShaderModules;

	// We store the descriptor set layout to connect it later on to the pipeline resource layout when creating it.
	// The layout is also needed when allocating descriptor sets from a pool.
	VkDescriptorSetLayout m_vkDescriptorSetLayout;
};

class CrComputeShaderVulkan final : public ICrComputeShader
{
public:

	CrComputeShaderVulkan(crgfx::IDevice* renderDevice, const CrComputeShaderDescriptor& computeShaderDescriptor);

	~CrComputeShaderVulkan();

	VkShaderModule GetVkShaderModule() const { return m_vkShaderModule; }

	VkDescriptorSetLayout GetVkDescriptorSetLayout() const { return m_vkDescriptorSetLayout; }

private:

	VkDevice m_vkDevice;

	VkShaderModule m_vkShaderModule;

	VkDescriptorSetLayout m_vkDescriptorSetLayout;
};