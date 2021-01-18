#include "CrRendering_pch.h"

#include "Rendering/ICrShader.h" // TODO remove
#include "CrShaderManager_vk.h"
#include "CrShaderReflection_vk.h"
#include "CrRenderDevice_vk.h"
#include "ShaderResources.h"
#include "CrVulkan.h"

#include "Core/Logging/ICrDebug.h"

#pragma warning (push, 0)

// SPIRV-Cross
#include <spirv_cross.hpp>
#include <spirv_cpp.hpp>

#pragma warning (pop)

void CrShaderManagerVulkan::InitPS()
{

}

void CrShaderManagerVulkan::CreateShaderResourceTablePS
(
	const CrShaderReflectionVulkan& reflection, 
	CrShaderResourceTable& resourceTable
) const
{
	VkDevice vkDevice = static_cast<const CrRenderDeviceVulkan*>(m_renderDevice)->GetVkDevice();

	CrVector<VkDescriptorSetLayoutBinding> layoutBindings;

	reflection.ForEachConstantBuffer([&layoutBindings](cr3d::ShaderStage::T stage, const CrShaderResource& constantBuffer)
	{
		VkDescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding = constantBuffer.bindPoint;
		layoutBinding.descriptorType = crvk::GetVkDescriptorType(cr3d::ShaderResourceType::ConstantBuffer);
		layoutBinding.descriptorCount = 1; // TODO Get array size from reflection
		layoutBinding.stageFlags = crvk::GetVkShaderStage(stage);
		layoutBinding.pImmutableSamplers = nullptr;
		layoutBindings.push_back(layoutBinding);
	});

	reflection.ForEachTexture([&layoutBindings](cr3d::ShaderStage::T stage, const CrShaderResource& texture)
	{
		VkDescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding = texture.bindPoint;
		layoutBinding.descriptorType = crvk::GetVkDescriptorType(cr3d::ShaderResourceType::Texture);
		layoutBinding.descriptorCount = 1; // TODO Get array size from reflection
		layoutBinding.stageFlags = crvk::GetVkShaderStage(stage);
		layoutBinding.pImmutableSamplers = nullptr;
		layoutBindings.push_back(layoutBinding);
	});

	reflection.ForEachSampler([&layoutBindings](cr3d::ShaderStage::T stage, const CrShaderResource& sampler)
	{
		VkDescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding = sampler.bindPoint;
		layoutBinding.descriptorType = crvk::GetVkDescriptorType(cr3d::ShaderResourceType::Sampler);
		layoutBinding.descriptorCount = 1; // TODO Get array size from reflection
		layoutBinding.stageFlags = crvk::GetVkShaderStage(stage);
		layoutBinding.pImmutableSamplers = nullptr;
		layoutBindings.push_back(layoutBinding);
	});

	VkDescriptorSetLayoutCreateInfo descriptorLayout;
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.pNext = nullptr;
	descriptorLayout.flags = 0;
	descriptorLayout.bindingCount = (uint32_t) layoutBindings.size();
	descriptorLayout.pBindings = layoutBindings.data();

	VkResult result = vkCreateDescriptorSetLayout(vkDevice, &descriptorLayout, nullptr, &resourceTable.m_vkDescriptorSetLayout);
	CrAssert(result == VK_SUCCESS);
}
