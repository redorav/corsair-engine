#include "CrRendering_pch.h"

#include "ICrShader.h" // TODO remove
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

void CrShaderManagerVulkan::CreateShaderResourceSetPS
(
	const CrVector<CrShaderStageInfo>& shaderStageInfo, 
	const CrShaderReflectionVulkan& reflection, 
	CrShaderResourceSet& resourceSet
) const
{
	VkDevice vkDevice = static_cast<const CrRenderDeviceVulkan*>(m_renderDevice)->GetVkDevice();

	CrVector<VkDescriptorSetLayoutBinding> layoutBindings;

	for (const CrShaderStageInfo& stageInfo : shaderStageInfo)
	{
		cr3d::ShaderStage::T stage = stageInfo.stage;

		for (cr3d::ShaderResourceType::T resourceType = cr3d::ShaderResourceType::Start; resourceType < cr3d::ShaderResourceType::Count; ++resourceType)
		{
			for (uint32_t i = 0; i < reflection.GetResourceCount(stage, resourceType); ++i)
			{
				CrShaderResource resource = reflection.GetResource(stage, resourceType, i);

				VkDescriptorSetLayoutBinding layoutBinding;
				layoutBinding.binding = resource.bindPoint;
				layoutBinding.descriptorType = crvk::GetVkDescriptorType(resourceType);
				layoutBinding.descriptorCount = 1; // TODO Get array size from reflection
				layoutBinding.stageFlags = crvk::GetVkShaderStage(stage);
				layoutBinding.pImmutableSamplers = nullptr;
				layoutBindings.push_back(layoutBinding);
			}
		}
	}

	VkDescriptorSetLayoutCreateInfo descriptorLayout;
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.pNext = nullptr;
	descriptorLayout.flags = 0;
	descriptorLayout.bindingCount = (uint32_t) layoutBindings.size();
	descriptorLayout.pBindings = layoutBindings.data();

	VkResult result = vkCreateDescriptorSetLayout(vkDevice, &descriptorLayout, nullptr, &resourceSet.m_vkDescriptorSetLayout);
	CrAssert(result == VK_SUCCESS);
}
