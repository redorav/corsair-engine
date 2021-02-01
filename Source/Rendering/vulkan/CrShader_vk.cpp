#include "CrRendering_pch.h"
#include "CrShader_vk.h"
#include "CrRenderDevice_vk.h"
#include "CrShaderReflection_vk.h"

#include "Rendering/CrShaderResourceMetadata.h"

#include "Core/Logging/ICrDebug.h"

static ICrShaderBindingTable* CreateBindingTable(const ICrRenderDevice* renderDevice, const CrShaderReflectionVulkan& vulkanReflection)
{
	CrVector<VkDescriptorSetLayoutBinding> layoutBindings;
	CrShaderBindingTableResources resources;

	vulkanReflection.ForEachResource([&resources, &layoutBindings](cr3d::ShaderStage::T stage, const CrShaderResource& resource)
	{
		VkDescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding = resource.bindPoint;
		layoutBinding.descriptorType = crvk::GetVkDescriptorType(resource.type);
		layoutBinding.descriptorCount = 1; // TODO Get array size from reflection
		layoutBinding.stageFlags = crvk::GetVkShaderStage(stage);
		layoutBinding.pImmutableSamplers = nullptr;
	
		switch(resource.type)
		{
			case cr3d::ShaderResourceType::ConstantBuffer:
			{
				const ConstantBufferMetadata& metadata = CrShaderMetadata::GetConstantBuffer(resource.name);
				resources.constantBuffers.push_back(CrShaderBinding(resource.bindPoint, stage, metadata.id));
				break;
			}
			case cr3d::ShaderResourceType::Sampler:
			{
				const SamplerMetadata& metadata = CrShaderMetadata::GetSampler(resource.name);
				resources.samplers.push_back(CrShaderBinding(resource.bindPoint, stage, metadata.id));
				break;
			}
			case cr3d::ShaderResourceType::Texture:
			{
				const TextureMetadata& metadata = CrShaderMetadata::GetTexture(resource.name);
				resources.textures.push_back(CrShaderBinding(resource.bindPoint, stage, metadata.id));
				break;
			}
			case cr3d::ShaderResourceType::RWTexture:
			{
				const RWTextureMetadata& metadata = CrShaderMetadata::GetRWTexture(resource.name);
				resources.rwTextures.push_back(CrShaderBinding(resource.bindPoint, stage, metadata.id));
				break;
			}
			case cr3d::ShaderResourceType::RWDataBuffer:
			{
				const RWDataBufferMetadata& metadata = CrShaderMetadata::GetRWDataBuffer(resource.name);
				resources.rwDataBuffers.push_back(CrShaderBinding(resource.bindPoint, stage, metadata.id));
				break;
			}
		};
	
		layoutBindings.push_back(layoutBinding);
	});

	CrShaderBindingTableVulkan* vulkanBindingTable = new CrShaderBindingTableVulkan(resources);

	VkDescriptorSetLayoutCreateInfo descriptorLayout;
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.pNext = nullptr;
	descriptorLayout.flags = 0;
	descriptorLayout.bindingCount = (uint32_t)layoutBindings.size();
	descriptorLayout.pBindings = layoutBindings.data();

	VkResult result = vkCreateDescriptorSetLayout(
		static_cast<const CrRenderDeviceVulkan*>(renderDevice)->GetVkDevice(), &descriptorLayout, nullptr, 
		&static_cast<CrShaderBindingTableVulkan*>(vulkanBindingTable)->m_vkDescriptorSetLayout);

	CrAssert(result == VK_SUCCESS);

	return vulkanBindingTable;
}

CrGraphicsShaderVulkan::CrGraphicsShaderVulkan(const ICrRenderDevice* renderDevice, const CrGraphicsShaderDescriptor& graphicsShaderDescriptor)
	: ICrGraphicsShader(renderDevice, graphicsShaderDescriptor)
{
	m_vkDevice = static_cast<const CrRenderDeviceVulkan*>(renderDevice)->GetVkDevice();

	CrShaderReflectionVulkan vulkanReflection;

	// Create the shader modules and parse reflection information
	for (const CrShaderBytecodeSharedHandle& shaderBytecode : graphicsShaderDescriptor.m_bytecodes)
	{
		VkShaderModuleCreateInfo moduleCreateInfo;
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.pNext = nullptr;
		moduleCreateInfo.codeSize = shaderBytecode->GetBytecode().size();
		moduleCreateInfo.pCode = (uint32_t*)shaderBytecode->GetBytecode().data();
		moduleCreateInfo.flags = 0;

		VkShaderModule vkShaderModule;
		VkResult vkResult = vkCreateShaderModule(m_vkDevice, &moduleCreateInfo, nullptr, &vkShaderModule);
		CrAssert(vkResult == VK_SUCCESS);

		m_vkShaderModules.push_back(vkShaderModule);

		vulkanReflection.AddBytecode(shaderBytecode);
	}

	// Create the optimized shader resource table
	m_bindingTable = CrUniquePtr<ICrShaderBindingTable>(CreateBindingTable(renderDevice, vulkanReflection));
}

CrGraphicsShaderVulkan::~CrGraphicsShaderVulkan()
{
	for (VkShaderModule vkShaderModule : m_vkShaderModules)
	{
		vkDestroyShaderModule(m_vkDevice, vkShaderModule, nullptr);
	}
}

CrComputeShaderVulkan::CrComputeShaderVulkan(const ICrRenderDevice* renderDevice, const CrComputeShaderDescriptor& computeShaderDescriptor)
	: ICrComputeShader(renderDevice, computeShaderDescriptor)
{
	m_vkDevice = static_cast<const CrRenderDeviceVulkan*>(renderDevice)->GetVkDevice();

	VkShaderModuleCreateInfo moduleCreateInfo;
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = nullptr;
	moduleCreateInfo.codeSize = computeShaderDescriptor.m_bytecode->GetBytecode().size();
	moduleCreateInfo.pCode = (uint32_t*)computeShaderDescriptor.m_bytecode->GetBytecode().data();
	moduleCreateInfo.flags = 0;

	VkResult vkResult = vkCreateShaderModule(m_vkDevice, &moduleCreateInfo, nullptr, &m_vkShaderModule);
	CrAssert(vkResult == VK_SUCCESS);

	CrShaderReflectionVulkan vulkanReflection;
	vulkanReflection.AddBytecode(computeShaderDescriptor.m_bytecode);

	// Create the optimized shader resource table
	m_bindingTable = CrUniquePtr<ICrShaderBindingTable>(CreateBindingTable(renderDevice, vulkanReflection));
}
