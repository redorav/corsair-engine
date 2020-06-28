#include "CrRendering_pch.h"

#include "CrMaterial.h"

CrMaterial::CrMaterial(CrGraphicsShader* shader)
{
	m_shader = shader;
}

void CrMaterial::Create(CrMaterialSharedHandle& material)
{
	material = CrMakeShared<CrMaterial>();
}

CrGraphicsShader* CrMaterial::GetShader()
{
	return m_shader;
}

void CrMaterial::AddTexture(const CrTextureSharedHandle& texture, Textures::T semantic)
{
	TextureBinding binding;
	binding.texture = texture;
	binding.semantic = semantic;
	m_textures.push_back(binding);
}
