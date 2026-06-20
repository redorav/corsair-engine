#pragma once

#include "Graphics/CrGraphicsForwardDeclarations.h"

#include "crstl/intrusive_ptr.h"

namespace crgfx
{
	extern crgfx::SamplerHandle AllLinearClampSampler;
	extern crgfx::SamplerHandle AllLinearWrapSampler;
	extern crgfx::SamplerHandle AllPointClampSampler;
	extern crgfx::SamplerHandle AllPointWrapSampler;
	extern crgfx::TextureHandle WhiteSmallTexture;
	extern crgfx::TextureHandle BlackSmallTexture;

	// Points up in tangent space
	extern crgfx::TextureHandle NormalsSmallTexture;

	void InitializeCommonResources();

	void DeinitializeCommonResources();
};