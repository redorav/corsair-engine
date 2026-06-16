#pragma once

#include "Graphics/IShader.h"
#include <vulkan/vulkan.h>

namespace crgfx
{
	class GraphicsShaderVulkan final : public IGraphicsShader
	{
	public:

		GraphicsShaderVulkan(crgfx::IDevice* renderDevice, const crgfx::GraphicsShaderDescriptor& graphicsShaderDescriptor);

		~GraphicsShaderVulkan();

		const crstl::vector<VkShaderModule>& GetVkShaderModules() const { return m_vkShaderModules; }

		VkDescriptorSetLayout GetVkDescriptorSetLayout() const { return m_vkDescriptorSetLayout; }

	private:

		VkDevice m_vkDevice;

		crstl::vector<VkShaderModule> m_vkShaderModules;

		// We store the descriptor set layout to connect it later on to the pipeline resource layout when creating it.
		// The layout is also needed when allocating descriptor sets from a pool.
		VkDescriptorSetLayout m_vkDescriptorSetLayout;
	};

	class ComputeShaderVulkan final : public IComputeShader
	{
	public:

		ComputeShaderVulkan(crgfx::IDevice* renderDevice, const crgfx::ComputeShaderDescriptor& computeShaderDescriptor);

		~ComputeShaderVulkan();

		VkShaderModule GetVkShaderModule() const { return m_vkShaderModule; }

		VkDescriptorSetLayout GetVkDescriptorSetLayout() const { return m_vkDescriptorSetLayout; }

	private:

		VkDevice m_vkDevice;

		VkShaderModule m_vkShaderModule;

		VkDescriptorSetLayout m_vkDescriptorSetLayout;
	};
};