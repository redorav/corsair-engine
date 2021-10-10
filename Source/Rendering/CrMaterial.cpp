#include "CrRendering_pch.h"

#include "CrMaterial.h"

static CrMaterialPassProperties MaterialPassProperties[CrMaterialPipelineVariant::Count];

const CrMaterialPassProperties& CrMaterialPassProperties::GetProperties(CrMaterialPipelineVariant::T pipelineVariant)
{
	return MaterialPassProperties[pipelineVariant];
}

static bool SetupGlobalUbershaderProperties()
{
	cr3d::DataFormat::T mainDepthFormat = cr3d::DataFormat::D32_Float_S8_Uint;

	MaterialPassProperties[CrMaterialPipelineVariant::Depth].renderTargets.colorFormats[0] = cr3d::DataFormat::BGRA8_Unorm;
	MaterialPassProperties[CrMaterialPipelineVariant::Depth].renderTargets.depthFormat = mainDepthFormat;
	MaterialPassProperties[CrMaterialPipelineVariant::Depth].shaderVariant = CrMaterialShaderVariant::Depth;

	MaterialPassProperties[CrMaterialPipelineVariant::Shadow].renderTargets.depthFormat = cr3d::DataFormat::D16_Unorm;
	MaterialPassProperties[CrMaterialPipelineVariant::Shadow].shaderVariant = CrMaterialShaderVariant::Depth;

	MaterialPassProperties[CrMaterialPipelineVariant::GBuffer].renderTargets.colorFormats[0] = cr3d::DataFormat::RGBA8_Unorm;
	MaterialPassProperties[CrMaterialPipelineVariant::GBuffer].renderTargets.depthFormat = mainDepthFormat;
	MaterialPassProperties[CrMaterialPipelineVariant::GBuffer].shaderVariant = CrMaterialShaderVariant::GBuffer;

	// TODO Change to cr3d::DataFormat::RG11B10_Float
	MaterialPassProperties[CrMaterialPipelineVariant::Transparency].renderTargets.colorFormats[0] = cr3d::DataFormat::BGRA8_Unorm;
	MaterialPassProperties[CrMaterialPipelineVariant::Transparency].renderTargets.depthFormat = mainDepthFormat;
	MaterialPassProperties[CrMaterialPipelineVariant::Transparency].shaderVariant = CrMaterialShaderVariant::Forward;

	return true;
}

static bool DummySetupGlobalUbershaderProperties = SetupGlobalUbershaderProperties();

void CrMaterial::AddTexture(const CrTextureSharedHandle& texture, Textures::T semantic)
{
	TextureBinding binding;
	binding.texture = texture;
	binding.semantic = semantic;
	m_textures.push_back(binding);
}