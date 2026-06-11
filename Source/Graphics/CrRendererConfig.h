#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"

struct CrRendererConfig
{
	static cr3d::DataFormat::T DepthBufferFormat;

	static cr3d::DataFormat::T GBufferAlbedoAOFormat;
	static cr3d::DataFormat::T GBufferNormalsFormat;
	static cr3d::DataFormat::T GBufferMaterialFormat;

	static cr3d::DataFormat::T LightingFormat;

	static cr3d::DataFormat::T DebugShaderFormat;

	static cr3d::DataFormat::T SwapchainFormat;
};