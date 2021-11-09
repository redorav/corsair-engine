#include "CrRendering_pch.h"
#include "CrShader_vk.h"
#include "CrRenderDevice_vk.h"

#include "Rendering/CrShaderResourceMetadata.h"

#include "Core/Logging/ICrDebug.h"

static ICrShaderBindingTable* CreateBindingTable
(
	const ICrRenderDevice* renderDevice, 
	const CrFixedVector<CrShaderReflectionHeader, cr3d::ShaderStage::Count>& reflectionHeaders
)
{
	CrVector<VkDescriptorSetLayoutBinding> layoutBindings;
	CrShaderBindingTableResources resources;

	for (uint32_t k = 0; k < reflectionHeaders.size(); ++k)
	{
		const CrShaderReflectionHeader& reflectionHeader = reflectionHeaders[k];

		for (uint32_t i = 0; i < reflectionHeader.resources.size(); ++i)
		{
			const CrShaderReflectionResource& resource = reflectionHeader.resources[i];

			cr3d::ShaderStage::T stage = reflectionHeader.shaderStage;

			VkDescriptorSetLayoutBinding layoutBinding;
			layoutBinding.binding = resource.bindPoint;
			layoutBinding.descriptorType = crvk::GetVkDescriptorType(resource.type);
			layoutBinding.descriptorCount = 1; // TODO Get array size from reflection
			layoutBinding.stageFlags = crvk::GetVkShaderStage(stage);
			layoutBinding.pImmutableSamplers = nullptr;

			switch (resource.type)
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
				case cr3d::ShaderResourceType::StorageBuffer:
				{
					const StorageBufferMetadata& metadata = CrShaderMetadata::GetStorageBuffer(resource.name);
					resources.storageBuffers.push_back(CrShaderBinding(resource.bindPoint, stage, metadata.id));
					break;
				}
				case cr3d::ShaderResourceType::RWStorageBuffer:
				{
					const RWStorageBufferMetadata& metadata = CrShaderMetadata::GetRWStorageBuffer(resource.name);
					resources.rwStorageBuffers.push_back(CrShaderBinding(resource.bindPoint, stage, metadata.id));
					break;
				}
				case cr3d::ShaderResourceType::RWDataBuffer:
				{
					const RWDataBufferMetadata& metadata = CrShaderMetadata::GetRWDataBuffer(resource.name);
					resources.rwDataBuffers.push_back(CrShaderBinding(resource.bindPoint, stage, metadata.id));
					break;
				}
			}

			layoutBindings.push_back(layoutBinding);
		}
	}

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

	uint32_t currentBindingPoint = 0;
	CrFixedVector<CrShaderReflectionHeader, cr3d::ShaderStage::Count> reflectionHeaders;

	// Create the shader modules and parse reflection information
	for (const CrShaderBytecodeSharedHandle& shaderBytecode : graphicsShaderDescriptor.m_bytecodes)
	{
		// Modify the reflection and the bytecode itself. We need to do this to get consecutive
		// binding points once different shader stages are brought together
		reflectionHeaders.push_back(shaderBytecode->GetReflection());
		CrShaderReflectionHeader& reflectionModified = reflectionHeaders.back();

		// Copy bytecode too. The bytecode gets discarded later as the shader module takes ownership
		CrVector<unsigned char> bytecodeModified = shaderBytecode->GetBytecode();

		for (CrShaderReflectionResource& resource : reflectionModified.resources)
		{
			resource.bindPoint = (uint8_t)currentBindingPoint;
			*((uint32_t*)&(bytecodeModified.data()[resource.bytecodeOffset])) = currentBindingPoint;
			currentBindingPoint++;
		}

		// Create shader modules from the modified bytecode
		VkShaderModuleCreateInfo moduleCreateInfo;
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.pNext = nullptr;
		moduleCreateInfo.codeSize = bytecodeModified.size();
		moduleCreateInfo.pCode = (uint32_t*)bytecodeModified.data();
		moduleCreateInfo.flags = 0;

		VkShaderModule vkShaderModule;
		VkResult vkResult = vkCreateShaderModule(m_vkDevice, &moduleCreateInfo, nullptr, &vkShaderModule);
		CrAssert(vkResult == VK_SUCCESS);

		m_vkShaderModules.push_back(vkShaderModule);
	}

	const CrShaderReflectionHeader& vertexReflection = reflectionHeaders[0];

	if (vertexReflection.shaderStage == cr3d::ShaderStage::Vertex)
	{
		for (uint32_t i = 0; i < vertexReflection.stageInputs.size(); ++i)
		{
			const CrShaderInterfaceVariable& interfaceVariable = vertexReflection.stageInputs[i];

			// We don't want to register built-ins
			if (interfaceVariable.type == cr3d::ShaderInterfaceBuiltinType::None)
			{
				CrVertexInput& input = m_inputSignature.inputs.push_back();
				input.semantic = CrVertexSemantic::FromString(interfaceVariable.name.c_str());
			}
		}
	}

	// Create the optimized shader resource table
	m_bindingTable = CrUniquePtr<ICrShaderBindingTable>(CreateBindingTable(renderDevice, reflectionHeaders));
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

	CrFixedVector<CrShaderReflectionHeader, cr3d::ShaderStage::Count> reflectionHeaders;
	reflectionHeaders.push_back(computeShaderDescriptor.m_bytecode->GetReflection());

	VkShaderModuleCreateInfo moduleCreateInfo;
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.pNext = nullptr;
	moduleCreateInfo.codeSize = computeShaderDescriptor.m_bytecode->GetBytecode().size();
	moduleCreateInfo.pCode = (uint32_t*)computeShaderDescriptor.m_bytecode->GetBytecode().data();
	moduleCreateInfo.flags = 0;

	VkResult vkResult = vkCreateShaderModule(m_vkDevice, &moduleCreateInfo, nullptr, &m_vkShaderModule);
	CrAssert(vkResult == VK_SUCCESS);

	// Create the optimized shader resource table
	m_bindingTable = CrUniquePtr<ICrShaderBindingTable>(CreateBindingTable(renderDevice, reflectionHeaders));
}
