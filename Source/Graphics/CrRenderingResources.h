#pragma once

#include "Graphics/CrRenderingForwardDeclarations.h"

#include "crstl/intrusive_ptr.h"

// Rendering resources that are used as default
class CrRenderingResources
{
public:

	static void Initialize();

	static void Deinitialize();

	// Global samplers

	CrSamplerHandle AllLinearClampSampler;

	CrSamplerHandle AllLinearWrapSampler;

	CrSamplerHandle AllPointClampSampler;

	CrSamplerHandle AllPointWrapSampler;

	crgfx::TextureHandle WhiteSmallTexture;

	crgfx::TextureHandle BlackSmallTexture;

	// Points up in tangent space
	crgfx::TextureHandle NormalsSmallTexture;

private:

	CrRenderingResources();
};

extern CrRenderingResources* RenderingResources;