#pragma once

#include "Graphics/CrGraphicsForwardDeclarations.h"

struct CrRendererConfig
{
	static crgfx::DataFormat::T DepthBufferFormat;

	static crgfx::DataFormat::T GBufferAlbedoAOFormat;
	static crgfx::DataFormat::T GBufferNormalsFormat;
	static crgfx::DataFormat::T GBufferMaterialFormat;

	static crgfx::DataFormat::T LightingFormat;

	static crgfx::DataFormat::T DebugShaderFormat;

	static crgfx::DataFormat::T SwapchainFormat;
};