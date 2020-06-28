#include "CrRendering_pch.h"
#include "CrShaderReflection_vk.h"

#pragma warning (push, 0)
// Glslang
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>

// SPIR-V
#include <spirv_cross.hpp>
#pragma warning (pop)

const spirv_cross::Resource CrShaderReflectionVulkan::defaultResource = { 0, 0, 0, "" };

void CrShaderReflectionVulkan::AddShaderStagePS(cr3d::ShaderStage::T stage, const CrVector<unsigned char>& bytecode)
{
	// Perform reflection on the shader
	// SPIRV-Cross has several classes like CompilerGLSL that can translate from SPIR-V to other high level languages. For reflection we don't need them.
	reflection[stage] = CrMakeUnique<spirv_cross::Compiler>(reinterpret_cast<const uint32_t*>(bytecode.data()), bytecode.size() / 4);
	resources[stage] = reflection[stage]->get_shader_resources(reflection[stage]->get_active_interface_variables());
}

const spirv_cross::Resource& CrShaderReflectionVulkan::GetSpvResource(cr3d::ShaderStage::T stage, cr3d::ShaderResourceType::T resourceType, uint32_t index) const
{
	switch (resourceType)
	{
		case cr3d::ShaderResourceType::ConstantBuffer:
			return resources[stage].uniform_buffers[index];
		case cr3d::ShaderResourceType::Texture:
			return resources[stage].separate_images[index];
		case cr3d::ShaderResourceType::Sampler:
			return resources[stage].separate_samplers[index];
		case cr3d::ShaderResourceType::ROStructuredBuffer:
		case cr3d::ShaderResourceType::RWStructuredBuffer:
			return resources[stage].storage_buffers[index];
		case cr3d::ShaderResourceType::RWTexture:
			return resources[stage].storage_images[index];
		default:
			return defaultResource;
	}
}

CrShaderResource CrShaderReflectionVulkan::GetResourcePS(cr3d::ShaderStage::T stage, cr3d::ShaderResourceType::T resourceType, uint32_t index) const
{
	CrShaderResource resource = CrShaderResource::Invalid;
	const spirv_cross::Resource& spvResource = GetSpvResource(stage, resourceType, index);

	resource.name = spvResource.name.c_str();
	resource.bindPoint = (bindpoint_t)reflection[stage]->get_decoration(spvResource.id, spv::DecorationBinding);

	return resource;
}

uint32_t CrShaderReflectionVulkan::GetResourceCountPS(cr3d::ShaderStage::T stage, cr3d::ShaderResourceType::T resourceType) const
{
	switch (resourceType)
	{
		case cr3d::ShaderResourceType::ConstantBuffer:
			return (uint32_t)resources[stage].uniform_buffers.size();
		case cr3d::ShaderResourceType::Texture:
			return (uint32_t)resources[stage].separate_images.size();
		case cr3d::ShaderResourceType::Sampler:
			return (uint32_t)resources[stage].separate_samplers.size();
		case cr3d::ShaderResourceType::ROStructuredBuffer:
		case cr3d::ShaderResourceType::RWStructuredBuffer:
			return (uint32_t)resources[stage].storage_buffers.size();
		case cr3d::ShaderResourceType::RWTexture:
			return (uint32_t)resources[stage].storage_images.size();
		default:
			return 0;
	}
}
