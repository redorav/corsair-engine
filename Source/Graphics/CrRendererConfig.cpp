#include "Graphics/CrRendering_pch.h"

#include "Graphics/CrRendererConfig.h"

#include "Graphics/DataFormats.h"

crgfx::DataFormat::T CrRendererConfig::DepthBufferFormat = crgfx::DataFormat::D32_Float_S8_Uint;

// GBuffer
crgfx::DataFormat::T CrRendererConfig::GBufferAlbedoAOFormat = crgfx::DataFormat::RGBA8_Unorm;
crgfx::DataFormat::T CrRendererConfig::GBufferNormalsFormat = crgfx::DataFormat::RGBA8_Unorm;
crgfx::DataFormat::T CrRendererConfig::GBufferMaterialFormat = crgfx::DataFormat::RGBA8_Unorm;

crgfx::DataFormat::T CrRendererConfig::LightingFormat = crgfx::DataFormat::RG11B10_Float;

// Debug Shader
crgfx::DataFormat::T CrRendererConfig::DebugShaderFormat = crgfx::DataFormat::RGBA32_Float;

crgfx::DataFormat::T CrRendererConfig::SwapchainFormat = crgfx::DataFormat::BGRA8_Unorm;