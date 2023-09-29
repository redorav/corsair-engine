#include "Rendering/CrRendering_pch.h"

#include "Rendering/CrMaterial.h"
#include "Rendering/CrRendererConfig.h"
#include "Rendering/ICrShader.h"
#include "Rendering/CrRenderMesh.h"

#include "Core/Logging/ICrDebug.h"

CrMaterial::CrMaterial()
	: m_color(1.0f, 1.0f, 1.0f, 1.0f)
{

}

CrMaterial::~CrMaterial()
{

}

CrMaterialPassProperties CrMaterialPassProperties::GetMaterialPassProperties(const CrRenderMeshHandle& mesh, CrMaterialPipelineVariant::T pipelineVariant)
{
	CrMaterialPassProperties materialPassProperties;

	cr3d::DataFormat::T mainDepthFormat = CrRendererConfig::DepthBufferFormat;
	
	if (mesh->GetIsDoubleSided())
	{
		materialPassProperties.pipelineDescriptor.rasterizerState.cullMode = cr3d::PolygonCullMode::None;
	}

	if (pipelineVariant == CrMaterialPipelineVariant::Depth)
	{
		materialPassProperties.pipelineDescriptor.renderTargets.depthFormat = mainDepthFormat;
		materialPassProperties.shaderVariant = CrMaterialShaderVariant::Depth;
	}
	else if (pipelineVariant == CrMaterialPipelineVariant::Shadow)
	{
		materialPassProperties.pipelineDescriptor.renderTargets.depthFormat = cr3d::DataFormat::D16_Unorm;
		materialPassProperties.shaderVariant = CrMaterialShaderVariant::Depth;
	}
	else if (pipelineVariant == CrMaterialPipelineVariant::GBuffer)
	{
		materialPassProperties.pipelineDescriptor.renderTargets.colorFormats[0] = CrRendererConfig::GBufferAlbedoAOFormat;
		materialPassProperties.pipelineDescriptor.renderTargets.colorFormats[1] = CrRendererConfig::GBufferNormalsFormat;
		materialPassProperties.pipelineDescriptor.renderTargets.colorFormats[2] = CrRendererConfig::GBufferMaterialFormat;
		materialPassProperties.pipelineDescriptor.renderTargets.depthFormat = mainDepthFormat;
		materialPassProperties.shaderVariant = CrMaterialShaderVariant::GBuffer;
	}
	else if (pipelineVariant == CrMaterialPipelineVariant::Transparency)
	{
		// TODO Change to cr3d::DataFormat::RG11B10_Float
		materialPassProperties.pipelineDescriptor.renderTargets.colorFormats[0] = CrRendererConfig::SwapchainFormat;
		materialPassProperties.pipelineDescriptor.blendState.renderTargetBlends[0] = CrStandardPipelineStates::AlphaBlend;
		materialPassProperties.pipelineDescriptor.renderTargets.depthFormat = mainDepthFormat;
		materialPassProperties.shaderVariant = CrMaterialShaderVariant::Forward;
	}
	else if (pipelineVariant == CrMaterialPipelineVariant::UI)
	{
		materialPassProperties.pipelineDescriptor.renderTargets.colorFormats[0] = CrRendererConfig::SwapchainFormat;
		materialPassProperties.pipelineDescriptor.renderTargets.depthFormat = mainDepthFormat;
		materialPassProperties.shaderVariant = CrMaterialShaderVariant::Forward;
	}
	else if (pipelineVariant == CrMaterialPipelineVariant::Debug)
	{
		materialPassProperties.pipelineDescriptor.renderTargets.colorFormats[0] = CrRendererConfig::DebugShaderFormat;
		materialPassProperties.pipelineDescriptor.renderTargets.depthFormat = mainDepthFormat;
		materialPassProperties.shaderVariant = CrMaterialShaderVariant::Debug;
	}

	CrAssert(
		materialPassProperties.pipelineDescriptor.renderTargets.colorFormats[0] != cr3d::DataFormat::Invalid ||
		materialPassProperties.pipelineDescriptor.renderTargets.depthFormat != cr3d::DataFormat::Invalid);
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