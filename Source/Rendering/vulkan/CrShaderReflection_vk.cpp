#include "CrRendering_pch.h"
#include "CrShaderReflection_vk.h"

#include "Rendering/ICrShader.h"

CrShaderReflectionVulkan::~CrShaderReflectionVulkan()
{
	for (cr3d::ShaderStage::T stage = cr3d::ShaderStage::Vertex; stage < cr3d::ShaderStage::Count; ++stage)
	{
		spvReflectDestroyShaderModule(&m_reflection[stage]);
	}
}

void CrShaderReflectionVulkan::AddBytecode(const CrShaderBytecodeSharedHandle& bytecode)
{
	cr3d::ShaderStage::T shaderStage = bytecode->GetShaderStage();

	// Perform reflection on the shader
	spvReflectCreateShaderModule(bytecode->GetBytecode().size(), bytecode->GetBytecode().data(), &m_reflection[shaderStage]);

	for (uint32_t i = 0; i < m_reflection[shaderStage].descriptor_binding_count; ++i)
	{
		spvReflectChangeDescriptorBindingNumbers(&m_reflection[shaderStage], &m_reflection[shaderStage].descriptor_bindings[i],
			m_currentBindSlot, (uint32_t)SPV_REFLECT_SET_NUMBER_DONT_CHANGE);
		m_currentBindSlot++;
	}
}

void CrShaderReflectionVulkan::ForEachResource(ShaderReflectionFn fn) const
{
	for (cr3d::ShaderStage::T stage = cr3d::ShaderStage::Vertex; stage < cr3d::ShaderStage::Count; ++stage)
	{
		const SpvReflectShaderModule& shaderModule = m_reflection[stage];
		for (uint32_t i = 0; i < shaderModule.descriptor_binding_count; ++i)
		{
			const SpvReflectDescriptorBinding& binding = shaderModule.descriptor_bindings[i];

			if (binding.accessed)
			{
				CrShaderResource resource;
				resource.name = binding.name;

				switch (binding.descriptor_type)
				{
					case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
						resource.name = binding.name;
						resource.type = cr3d::ShaderResourceType::ConstantBuffer;
						break;
					case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
						resource.type = cr3d::ShaderResourceType::Texture;
						break;
					case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
						resource.type = cr3d::ShaderResourceType::Sampler;
						break;
					case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
					{
						if (binding.resource_type == SPV_REFLECT_RESOURCE_FLAG_UAV)
						{
							resource.type = cr3d::ShaderResourceType::RWStorageBuffer;
						}
						else
						{
							resource.type = cr3d::ShaderResourceType::StorageBuffer;
						}
						break;
					}
					case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
						resource.type = cr3d::ShaderResourceType::RWTexture;
						break;
					case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
						resource.type = cr3d::ShaderResourceType::DataBuffer;
						break;
					case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
						resource.type = cr3d::ShaderResourceType::RWDataBuffer;
						break;
					default:
						break;
				}

				resource.bindPoint = (bindpoint_t)binding.binding;
				fn(stage, resource);
			}
		}
	}
}