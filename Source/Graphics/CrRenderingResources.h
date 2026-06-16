#pragma once

#include "Graphics/CrGraphicsForwardDeclarations.h"

#include "crstl/intrusive_ptr.h"

// Rendering resources that are used as default
class CrRenderingResources
{
public:

	static void Initialize();

	static void Deinitialize();

	// Global samplers

	crgfx::SamplerHandle AllLinearClampSampler;

	crgfx::SamplerHandle AllLinearWrapSampler;

	crgfx::SamplerHandle AllPointClampSampler;

	crgfx::SamplerHandle AllPointWrapSampler;

	crgfx::TextureHandle WhiteSmallTexture;

	crgfx::TextureHandle BlackSmallTexture;

	// Points up in tangent space
	crgfx::TextureHandle NormalsSmallTexture;

private:

	CrRenderingResources();
};

extern CrRenderingResources* RenderingResources;