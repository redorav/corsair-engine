#pragma once

#include "Core/Containers/CrVector.h"

#include "Core/SmartPointers/CrSharedPtr.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

class CrMaterial;
using CrMaterialSharedHandle = CrSharedPtr<CrMaterial>;


class CrMaterial
{
public:

	CrMaterial()
	{

	}

	CrMaterial(ICrGraphicsShader* shader);

	static void Create(CrMaterialSharedHandle& material);

	ICrGraphicsShader* GetShader();

	void AddTexture(const CrTextureSharedHandle& texture, Textures::T semantic);

//private: TODO Fix

	struct TextureBinding
	{
		CrTextureSharedHandle texture;
		Textures::T semantic;
	};

	ICrGraphicsShader* m_shader;
	CrVector<TextureBinding> m_textures;
};
