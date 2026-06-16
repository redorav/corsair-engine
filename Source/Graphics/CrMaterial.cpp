#include "Graphics/CrRendering_pch.h"

#include "Graphics/CrMaterial.h"
#include "Graphics/CrRendererConfig.h"
#include "Graphics/IShader.h"
#include "Graphics/CrRenderMesh.h"

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

	crgfx::DataFormat::T mainDepthFormat = CrRendererConfig::DepthBufferFormat;
	
	if (mesh->GetIsDoubleSided())
	{
		materialPassProperties.pipelineDescriptor.rasterizerState.cullMode = crgfx::PolygonCullMode::None;
	}

	if (pipelineVariant == CrMaterialPipelineVariant::Depth)
	{
		materialPassProperties.pipelineDescriptor.renderTargets.depthFormat = mainDepthFormat;
		materialPassProperties.shaderVariant = CrMaterialShaderVariant::Depth;
	}
	else if (pipelineVariant == CrMaterialPipelineVariant::Shadow)
	{
		materialPassProperties.pipelineDescriptor.renderTargets.depthFormat = crgfx::DataFormat::D16_Unorm;
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
		materialPassProperties.pipelineDescriptor.renderTargets.colorFormats[0] = CrRendererConfig::LightingFormat;
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
		materialPassProperties.pipelineDescriptor.renderTargets.colorFormats[0] != crgfx::DataFormat::Invalid ||
		materialPassProperties.pipelineDescriptor.renderTargets.depthFormat != crgfx::DataFormat::Invalid);
	CrAssert(materialPassProperties.shaderVariant != CrMaterialShaderVariant::Count);

	return materialPassProperties;
}

void CrMaterial::AddTexture(const crgfx::TextureHandle& texture, Textures::T semantic)
{
	TextureBinding binding;
	binding.texture = texture;
	binding.semantic = semantic;
	m_textures.push_back(binding);
}