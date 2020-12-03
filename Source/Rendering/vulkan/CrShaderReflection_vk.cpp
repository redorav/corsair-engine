#include "CrRendering_pch.h"
#include "CrShaderReflection_vk.h"

#include "Rendering/ICrShader.h"

#pragma warning (push, 0)
// SPIR-V
#include <spirv_cross.hpp>
#pragma warning (pop)

void CrShaderReflectionVulkan::AddBytecodePS(const CrShaderBytecodeSharedHandle& bytecode)
{
	// Perform reflection on the shader
	// SPIRV-Cross has several classes like CompilerGLSL that can translate from SPIR-V to other high level languages. For reflection we don't need them.
	cr3d::ShaderStage::T shaderStage = bytecode->GetShaderStage();
	m_reflection[shaderStage] = CrMakeUnique<spirv_cross::Compiler>(reinterpret_cast<const uint32_t*>(bytecode->GetBytecode().data()), bytecode->GetBytecode().size() / 4);
	m_resources[shaderStage] = m_reflection[shaderStage]->get_shader_resources(m_reflection[shaderStage]->get_active_interface_variables());
}

void CrShaderReflectionVulkan::ForEachConstantBuffer(ShaderReflectionFn fn) const
{
	for (cr3d::ShaderStage::T stage = cr3d::ShaderStage::Vertex; stage < cr3d::ShaderStage::Count; ++stage)
	{
		for (uint32_t i = 0; i < m_resources[stage].uniform_buffers.size(); ++i)
		{
			const spirv_cross::Resource& constantBuffer = m_resources[stage].uniform_buffers[i];
			CrShaderResource resource;
			resource.name = constantBuffer.name.c_str();
			resource.bindPoint = (bindpoint_t)m_reflection[stage]->get_decoration(constantBuffer.id, spv::DecorationBinding);
			fn(stage, resource);
		}
	}
}

void CrShaderReflectionVulkan::ForEachTexture(ShaderReflectionFn fn) const
{
	for (cr3d::ShaderStage::T stage = cr3d::ShaderStage::Vertex; stage < cr3d::ShaderStage::Count; ++stage)
	{
		for (uint32_t i = 0; i < m_resources[stage].separate_images.size(); ++i)
		{
			const spirv_cross::Resource& texture = m_resources[stage].separate_images[i];
			CrShaderResource resource;
			resource.name = texture.name.c_str();
			resource.bindPoint = (bindpoint_t)m_reflection[stage]->get_decoration(texture.id, spv::DecorationBinding);
			fn(stage, resource);
		}
	}
}

void CrShaderReflectionVulkan::ForEachSampler(ShaderReflectionFn fn) const
{
	for (cr3d::ShaderStage::T stage = cr3d::ShaderStage::Vertex; stage < cr3d::ShaderStage::Count; ++stage)
	{
		for (uint32_t i = 0; i < m_resources[stage].separate_samplers.size(); ++i)
		{
			const spirv_cross::Resource& sampler = m_resources[stage].separate_samplers[i];
			CrShaderResource resource;
			resource.name = sampler.name.c_str();
			resource.bindPoint = (bindpoint_t)m_reflection[stage]->get_decoration(sampler.id, spv::DecorationBinding);
			fn(stage, resource);
		}
	}
}