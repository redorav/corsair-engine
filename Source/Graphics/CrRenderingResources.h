#pragma once

#include "Graphics/CrRenderingForwardDeclarations.h"

#include "crstl/intrusive_ptr.h"

// Rendering resources that are used as default
class CrRenderingResources
{
public:

	static void Initialize(crgfx::ICrRenderDevice* renderDevice);

	static void Deinitialize();

	// Global samplers

	CrSamplerHandle AllLinearClampSampler;

	CrSamplerHandle AllLinearWrapSampler;

	CrSamplerHandle AllPointClampSampler;

	CrSamplerHandle AllPointWrapSampler;

	CrTextureHandle WhiteSmallTexture;

	CrTextureHandle BlackSmallTexture;

	// Points up in tangent space
	CrTextureHandle NormalsSmallTexture;

private:

	CrRenderingResources(crgfx::ICrRenderDevice* renderDevice);
};

extern CrRenderingResources* RenderingResources;