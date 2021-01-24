#include "CrRendering_pch.h"
#include "CrShader_vk.h"
#include "CrRenderDevice_vk.h"
#include "CrShaderReflection_vk.h"

#include "Rendering/CrShaderResourceMetadata.h"

#include "Core/Logging/ICrDebug.h"

static void CreateResourceTables(const ICrRenderDevice* renderDevice, const CrShaderReflectionVulkan& vulkanReflection, ICrShaderResourceTable* resourceTable)
{
	CrVector<VkDescriptorSetLayoutBinding> layoutBindings;

	vulkanReflection.ForEachConstantBuffer([&resourceTable, &layoutBindings](cr3d::ShaderStage::T stage, const CrShaderResource& constantBuffer)
	{
		const ConstantBufferMetadata& metadata = CrShaderMetadata::GetConstantBuffer(constantBuffer.name);
		resourceTable->AddConstantBuffer(stage, metadata.id, constantBuffer.bindPoint);

		VkDescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding = constantBuffer.bindPoint;
		layoutBinding.descriptorType = crvk::GetVkDescriptorType(cr3d::ShaderResourceType::ConstantBuffer);
		layoutBinding.descriptorCount = 1; // TODO Get array size from reflection
		layoutBinding.stageFlags = crvk::GetVkShaderStage(stage);
		layoutBinding.pImmutableSamplers = nullptr;
		layoutBindings.push_back(layoutBinding);
	});

	vulkanReflection.ForEachTexture([&resourceTable, &layoutBindings](cr3d::ShaderStage::T stage, const CrShaderResource& texture)
	{
		const TextureMetadata& metadata = CrShaderMetadata::GetTexture(texture.name);
		resourceTable->AddTexture(stage, metadata.id, texture.bindPoint);

		VkDescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding = texture.bindPoint;
		layoutBinding.descriptorType = crvk::GetVkDescriptorType(cr3d::ShaderResourceType::Texture);
		layoutBinding.descriptorCount = 1; // TODO Get array size from reflection
		layoutBinding.stageFlags = crvk::GetVkShaderStage(stage);
		layoutBinding.pImmutableSamplers = nullptr;
		layoutBindings.push_back(layoutBinding);
	});

	vulkanReflection.ForEachSampler([&resourceTable, &layoutBindings](cr3d::ShaderStage::T stage, const CrShaderResource& sampler)
	{
		const SamplerMetadata& metadata = CrShaderMetadata::GetSampler(sampler.name);
		resourceTable->AddSampler(stage, metadata.id, sampler.bindPoint);

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
	descriptorLayout.bindingCount = (uint32_t)layoutBindings.size();
	descriptorLayout.pBindings = layoutBindings.data();

	VkResult result = vkCreateDescriptorSetLayout(
		static_cast<const CrRenderDeviceVulkan*>(renderDevice)->GetVkDevice(), &descriptorLayout, nullptr, 
		&static_cast<CrShaderResourceTableVulkan*>(resourceTable)->m_vkDescriptorSetLayout);
	CrAssert(result == VK_SUCCESS);
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

	const CrShaderResourceCount& resourceCount = vulkanReflection.GetShaderResourceCount();

	// Create the optimized shader resource table
	m_resourceTable = CrUniquePtr<ICrShaderResourceTable>(new CrShaderResourceTableVulkan(resourceCount));

	CreateResourceTables(renderDevice, vulkanReflection, m_resourceTable.get());
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

	const CrShaderResourceCount& resourceCount = vulkanReflection.GetShaderResourceCount();

	// Create the optimized shader resource table
	m_resourceTable = CrUniquePtr<ICrShaderResourceTable>(new CrShaderResourceTableVulkan(resourceCount));

	CreateResourceTables(renderDevice, vulkanReflection, m_resourceTable.get());
}
