#pragma once

#include "Core/CrHash.h"
#include "Core/CrPlatform.h"

#include "Graphics/ICrPipeline.h"
#include "Graphics/ITexture.h"
#include "Graphics/CrRenderingForwardDeclarations.h"

#include "Math/CrHlslppVectorFloatType.h"

#include "crstl/intrusive_ptr.h"
#include "crstl/vector.h"

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
	static CrMaterialPassProperties GetMaterialPassProperties(const CrRenderMeshHandle& renderMesh, CrMaterialPipelineVariant::T pipelineVariant);

	// We'll modify the pipeline descriptor as we need based on the pipeline variant
	CrGraphicsPipelineDescriptor pipelineDescriptor;

	CrMaterialShaderVariant::T shaderVariant = CrMaterialShaderVariant::Count;
};

// High-level description of a material
struct CrMaterialDescriptor
{
	float4 color;

	float4 emissive;
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

	crgfx::ShaderStage::T shaderStage = crgfx::ShaderStage::Count;

	cr::Platform::T platform = cr::Platform::Count;

	crgfx::GraphicsApi::T graphicsApi = crgfx::GraphicsApi::Count;
};

static_assert(sizeof(CrMaterialShaderDescriptor) == 28, "CrMaterialShaderDescriptor size mismatch");

class CrMaterial final : public crstl::intrusive_ptr_interface_delete
{
public:

	CrMaterial();

	~CrMaterial();

	const CrGraphicsShaderHandle& GetShader(CrMaterialShaderVariant::T variant) const { return m_shaders[variant]; }

	void AddTexture(const crgfx::TextureHandle& texture, Textures::T semantic);

//private: TODO Fix

	struct TextureBinding
	{
		crgfx::TextureHandle texture;
		Textures::T semantic;
	};

	crstl::vector<TextureBinding> m_textures;

	CrGraphicsShaderHandle m_shaders[CrMaterialShaderVariant::Count];

	float4 m_color;

	float4 m_emissive;
};
