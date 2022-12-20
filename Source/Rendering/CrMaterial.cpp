#include "Rendering/CrRendering_pch.h"

#include "Rendering/CrMaterial.h"
#include "Rendering/CrRendererConfig.h"
#include "Core/Logging/ICrDebug.h"

CrMaterialPassProperties CrMaterialPassProperties::GetMaterialPassProperties(CrMaterialPipelineVariant::T pipelineVariant)
{
	CrMaterialPassProperties materialPassProperties;

	cr3d::DataFormat::T mainDepthFormat = CrRendererConfig::DepthBufferFormat;

	if (pipelineVariant == CrMaterialPipelineVariant::Depth)
	{
		materialPassProperties.renderTargets.depthFormat = mainDepthFormat;
		materialPassProperties.shaderVariant = CrMaterialShaderVariant::Depth;
	}
	else if (pipelineVariant == CrMaterialPipelineVariant::Shadow)
	{
		materialPassProperties.renderTargets.depthFormat = cr3d::DataFormat::D16_Unorm;
		materialPassProperties.shaderVariant = CrMaterialShaderVariant::Depth;
	}
	else if (pipelineVariant == CrMaterialPipelineVariant::GBuffer)
	{
		materialPassProperties.renderTargets.colorFormats[0] = CrRendererConfig::GBufferAlbedoAOFormat;
		materialPassProperties.renderTargets.colorFormats[1] = CrRendererConfig::GBufferNormalsFormat;
		materialPassProperties.renderTargets.colorFormats[2] = CrRendererConfig::GBufferMaterialFormat;
		materialPassProperties.renderTargets.depthFormat = mainDepthFormat;
		materialPassProperties.shaderVariant = CrMaterialShaderVariant::GBuffer;
	}
	else if (pipelineVariant == CrMaterialPipelineVariant::Transparency)
	{
		// TODO Change to cr3d::DataFormat::RG11B10_Float
		materialPassProperties.renderTargets.colorFormats[0] = CrRendererConfig::SwapchainFormat;
		materialPassProperties.renderTargets.depthFormat = mainDepthFormat;
		materialPassProperties.shaderVariant = CrMaterialShaderVariant::Forward;
	}
	else if (pipelineVariant == CrMaterialPipelineVariant::UI)
	{
		materialPassProperties.renderTargets.colorFormats[0] = CrRendererConfig::SwapchainFormat;
		materialPassProperties.renderTargets.depthFormat = mainDepthFormat;
		materialPassProperties.shaderVariant = CrMaterialShaderVariant::Forward;
	}
	else if (pipelineVariant == CrMaterialPipelineVariant::Debug)
	{
		materialPassProperties.renderTargets.colorFormats[0] = CrRendererConfig::DebugShaderFormat;
		materialPassProperties.renderTargets.depthFormat = mainDepthFormat;
		materialPassProperties.shaderVariant = CrMaterialShaderVariant::Debug;
	}

	CrAssert(materialPassProperties.renderTargets.colorFormats[0] != cr3d::DataFormat::Invalid || materialPassProperties.renderTargets.depthFormat != cr3d::DataFormat::Invalid);
	CrAssert(materialPassProperties.shaderVariant != CrMaterialShaderVariant::Count);

	return materialPassProperties;
}

void CrMaterial::AddTexture(const CrTextureHandle& texture, Textures::T semantic)
{
	TextureBinding binding;
	binding.texture = texture;
	binding.semantic = semantic;
	m_textures.push_back(binding);
}