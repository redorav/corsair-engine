#include "CrRendering_pch.h"
#include "CrShader_vk.h"
#include "CrRenderDevice_vk.h"
#include "CrVulkan.h"

#include "renderdoc_app.h"

#include "Rendering/CrShaderResourceMetadata.h"
#include "Rendering/ICrShader.inl"

#include "Core/Logging/ICrDebug.h"

static void CreateVkDescriptorSetLayout(VkDevice vkDevice, VkDescriptorSetLayoutBinding* layoutBindings, uint32_t layoutBindingCount, VkDescriptorSetLayout* vkDescriptorSetLayout)
{
	VkDescriptorSetLayoutCreateInfo descriptorLayout;
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.pNext = nullptr;
	descriptorLayout.flags = 0;
	descriptorLayout.bindingCount = layoutBindingCount;
	descriptorLayout.pBindings = layoutBindings;

	VkResult vkResult = vkCreateDescriptorSetLayout(vkDevice, &descriptorLayout, nullptr, vkDescriptorSetLayout);
	CrAssert(vkResult == VK_SUCCESS);
}

CrGraphicsShaderVulkan::CrGraphicsShaderVulkan(ICrRenderDevice* renderDevice, const CrGraphicsShaderDescriptor& graphicsShaderDescriptor)
	: ICrGraphicsShader(renderDevice, graphicsShaderDescriptor)
{
	m_vkDevice = static_cast<const CrRenderDeviceVulkan*>(renderDevice)->GetVkDevice();

	CrVector<VkDescriptorSetLayoutBinding> layoutBindings;
	CrShaderBindingLayoutResources resources;

	// Create the shader modules and parse reflection information
	for (const CrShaderBytecodeSharedHandle& shaderBytecode : graphicsShaderDescriptor.m_bytecodes)
	{
		// Modify the reflection and the bytecode itself. We need to do this to get consecutive
		// binding points once different shader stages are brought together
		const CrShaderReflectionHeader& reflectionHeader = shaderBytecode->GetReflection();

		// Copy bytecode too. The bytecode gets discarded later as the shader module takes ownership
		const CrVector<uint8_t>& bytecode = shaderBytecode->GetBytecode();

		// Create shader modules from the modified bytecode
		VkShaderModuleCreateInfo moduleCreateInfo;
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.pNext = nullptr;
		moduleCreateInfo.codeSize = bytecode.size();
		moduleCreateInfo.pCode = (uint32_t*)bytecode.data();
		moduleCreateInfo.flags = 0;

		VkShaderModule vkShaderModule;
		VkResult vkResult = vkCreateShaderModule(m_vkDevice, &moduleCreateInfo, nullptr, &vkShaderModule);
		CrAssert(vkResult == VK_SUCCESS);

		if (vkSetDebugUtilsObjectTag)
		{
			VkDebugUtilsObjectTagInfoEXT tagInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_TAG_INFO_EXT };
			tagInfo.objectType = VK_OBJECT_TYPE_SHADER_MODULE;
			tagInfo.objectHandle = (uint64_t)vkShaderModule;
			tagInfo.tagName = RENDERDOC_ShaderDebugMagicValue_truncated;

			CrString tagName = eastl::to_string(reflectionHeader.bytecodeHash) + ".pdb";

			tagInfo.pTag = tagName.c_str();
			tagInfo.tagSize = tagName.length();

			vkSetDebugUtilsObjectTag(m_vkDevice, &tagInfo);
		}

		m_vkShaderModules.push_back(vkShaderModule);

		ICrShaderBindingLayout::AddResources(reflectionHeader, resources, [&layoutBindings](cr3d::ShaderStage::T stage, const CrShaderReflectionResource& resource)
		{
			VkDescriptorSetLayoutBinding layoutBinding;
			layoutBinding.binding = resource.bindPoint;
			layoutBinding.descriptorType = crvk::GetVkDescriptorType(resource.type);
			layoutBinding.descriptorCount = 1; // TODO Get array size from reflection
			layoutBinding.stageFlags = crvk::GetVkShaderStage(stage);
			layoutBinding.pImmutableSamplers = nullptr;

			layoutBindings.push_back(layoutBinding);
		});

		if (reflectionHeader.shaderStage == cr3d::ShaderStage::Vertex)
		{
			for (uint32_t i = 0; i < reflectionHeader.stageInputs.size(); ++i)
			{
				const CrShaderInterfaceVariable& interfaceVariable = reflectionHeader.stageInputs[i];

				// We don't want to register built-ins
				if (interfaceVariable.type == cr3d::ShaderInterfaceBuiltinType::None)
				{
					CrVertexInput& input = m_inputSignature.inputs.push_back();
					input.semantic = CrVertexSemantic::FromString(interfaceVariable.name.c_str());
				}
			}
		}
	}

	CreateVkDescriptorSetLayout(static_cast<CrRenderDeviceVulkan*>(renderDevice)->GetVkDevice(), layoutBindings.data(), (uint32_t)layoutBindings.size(), &m_vkDescriptorSetLayout);

	m_bindingLayout = CrUniquePtr<ICrShaderBindingLayout>(new ICrShaderBindingLayout(resources));
}

CrGraphicsShaderVulkan::~CrGraphicsShaderVulkan()
{
	for (VkShaderModule vkShaderModule : m_vkShaderModules)
	{
		vkDestroyShaderModule(m_vkDevice, vkShaderModule, nullptr);
	}

	vkDestroyDescriptorSetLayout(m_vkDevice, m_vkDescriptorSetLayout, nullptr);
}

CrComputeShaderVulkan::CrComputeShaderVulkan(ICrRenderDevice* renderDevice, const CrComputeShaderDescriptor& computeShaderDescriptor)
	: ICrComputeShader(renderDevice, computeShaderDescriptor)
{
	m_vkDevice = static_cast<const CrRenderDeviceVulkan*>(renderDevice)->GetVkDevice();

	const CrShaderReflectionHeader& reflectionHeader = computeShaderDescriptor.m_bytecode->GetReflection();

	VkShaderModuleCreateInfo moduleCreateInfo;
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = nullptr;
	moduleCreateInfo.codeSize = computeShaderDescriptor.m_bytecode->GetBytecode().size();
	moduleCreateInfo.pCode = (uint32_t*)computeShaderDescriptor.m_bytecode->GetBytecode().data();
	moduleCreateInfo.flags = 0;

	VkResult vkResult = vkCreateShaderModule(m_vkDevice, &moduleCreateInfo, nullptr, &m_vkShaderModule);
	CrAssert(vkResult == VK_SUCCESS);

	CrVector<VkDescriptorSetLayoutBinding> layoutBindings;
	CrShaderBindingLayoutResources resources;

	ICrShaderBindingLayout::AddResources(reflectionHeader, resources, [&layoutBindings](cr3d::ShaderStage::T stage, const CrShaderReflectionResource& resource)
	{
		VkDescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding = resource.bindPoint;
		layoutBinding.descriptorType = crvk::GetVkDescriptorType(resource.type);
		layoutBinding.descriptorCount = 1; // TODO Get array size from reflection
		layoutBinding.stageFlags = crvk::GetVkShaderStage(stage);
		layoutBinding.pImmutableSamplers = nullptr;

		layoutBindings.push_back(layoutBinding);
	});

	CreateVkDescriptorSetLayout(static_cast<CrRenderDeviceVulkan*>(renderDevice)->GetVkDevice(), layoutBindings.data(), (uint32_t)layoutBindings.size(), &m_vkDescriptorSetLayout);

	// Create the optimized shader resource table
	m_bindingLayout = CrUniquePtr<ICrShaderBindingLayout>(new ICrShaderBindingLayout(resources));
}

CrComputeShaderVulkan::~CrComputeShaderVulkan()
{
	vkDestroyShaderModule(m_vkDevice, m_vkShaderModule, nullptr);

	vkDestroyDescriptorSetLayout(m_vkDevice, m_vkDescriptorSetLayout, nullptr);
}