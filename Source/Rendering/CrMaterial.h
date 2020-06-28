#pragma once

#include "Core/Containers/CrVector.h"

#include "Core/SmartPointers/CrSharedPtr.h"

class ICrTexture;
using CrTextureSharedHandle = CrSharedPtr<ICrTexture>;
class CrGraphicsShader;

class CrMaterial;
using CrMaterialSharedHandle = CrSharedPtr<CrMaterial>;

namespace Textures { enum T : uint8_t; }

class CrMaterial
{
public:

	CrMaterial()
	{

	}

	CrMaterial(CrGraphicsShader* shader);

	static void Create(CrMaterialSharedHandle& material);

	CrGraphicsShader* GetShader();

	void AddTexture(const CrTextureSharedHandle& texture, Textures::T semantic);

//private: TODO Fix

	struct TextureBinding
	{
		CrTextureSharedHandle texture;
		Textures::T semantic;
	};

	CrGraphicsShader* m_shader;
	CrVector<TextureBinding> m_textures;
};
