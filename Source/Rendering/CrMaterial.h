#pragma once

#include "Core/Containers/CrVector.h"
#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/CrHash.h"
#include "Core/CrPlatform.h"

#include "Rendering/ICrPipeline.h"
#include "Rendering/CrRenderingForwardDeclarations.h"

class CrMaterial;
using CrMaterialSharedHandle = CrSharedPtr<CrMaterial>;

namespace CrMaterialBlendMode
{
	enum T
	{
		Opaque = 0,
		AlphaBlend,
		Transparency,
		Additive
	};
};

namespace CrAlphaTestMode
{
	enum T
	{
		Always = 0,
		Equal,
		Greater,
		Less,
		GreaterEqual,
		LessEqual,
		Count
	};
};

namespace CrRefraction
{
	enum T
	{
		Disabled = 0,
		Enabled = 1,
		Count
	};
};

// This enum contains the number of shader variations for this shader
// It says nothing about the state of the shader
namespace CrMaterialShaderVariant
{
	enum T
	{
		Depth,   // Used for Z prepass + shadows
		GBuffer, // Used in GBuffer pass
		Forward, // Used for transparency/UI
		Debug,   // Used for debug visualization or tools
		Count,
		First = Depth
	};

	inline T& operator ++ (T& e) { e = static_cast<T>(static_cast<uint32_t>(e) + 1u); return e; }
	inline T  operator ++ (T& e, int) { T temp = e; e = static_cast<T>(static_cast<uint32_t>(e) + 1u); return temp; }
	inline T& operator -- (T& e) { e = static_cast<T>(static_cast<uint32_t>(e) - 1u); return e; }
	inline T  operator -- (T& e, int) { T temp = e; e = static_cast<T>(static_cast<uint32_t>(e) - 1u); return temp; }
};

// Expresses the combination of shader variant + render state
namespace CrMaterialPipelineVariant
{
	enum T
	{
		Depth,
		Shadow,
		GBuffer,
		Transparency,
		UI,
		Debug,
		Count,
		First = Depth
	};

	inline T& operator ++ (T& e) { e = static_cast<T>(static_cast<uint32_t>(e) + 1u); return e; }
	inline T  operator ++ (T& e, int) { T temp = e; e = static_cast<T>(static_cast<uint32_t>(e) + 1u); return temp; }
	inline T& operator -- (T& e) { e = static_cast<T>(static_cast<uint32_t>(e) - 1u); return e; }
	inline T  operator -- (T& e, int) { T temp = e; e = static_cast<T>(static_cast<uint32_t>(e) - 1u); return temp; }
};

struct CrMaterialPassProperties
{
	static CrMaterialPassProperties GetMaterialPassProperties(CrMaterialPipelineVariant::T pipelineVariant);

	CrRenderTargetFormatDescriptor renderTargets;
	CrMaterialShaderVariant::T shaderVariant = CrMaterialShaderVariant::Count;
};

// High-level description of a material
struct CrMaterialDescriptor
{

};

// High-level description of a shader
struct CrMaterialShaderDescriptor
{
	CrMaterialShaderDescriptor() {}

	CrHash ComputeHash() const
	{
		return CrHash(this, sizeof(*this));
	}

	CrMaterialBlendMode::T blendMode = CrMaterialBlendMode::Opaque;

	CrAlphaTestMode::T alphaTestMode = CrAlphaTestMode::Always;

	CrRefraction::T refraction = CrRefraction::Disabled;

	// Per-pass data
	CrMaterialShaderVariant::T shaderVariant = CrMaterialShaderVariant::GBuffer;

	// Global material properties

	cr3d::ShaderStage::T shaderStage = cr3d::ShaderStage::Count;

	cr::Platform::T platform = cr::Platform::Count;

	cr3d::GraphicsApi::T graphicsApi = cr3d::GraphicsApi::Count;
};

static_assert(sizeof(CrMaterialShaderDescriptor) == 28, "CrMaterialShaderDescriptor size mismatch");

class CrMaterial
{
public:

	const CrGraphicsShaderHandle& GetShader(CrMaterialShaderVariant::T variant) const { return m_shaders[variant]; }

	void AddTexture(const CrTextureSharedHandle& texture, Textures::T semantic);

//private: TODO Fix

	struct TextureBinding
	{
		CrTextureSharedHandle texture;
		Textures::T semantic;
	};

	CrVector<TextureBinding> m_textures;

	CrGraphicsShaderHandle m_shaders[CrMaterialShaderVariant::Count];
};
