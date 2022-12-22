#include "Rendering/CrRendering_pch.h"

#include "Rendering/CrRendererConfig.h"

cr3d::DataFormat::T CrRendererConfig::DepthBufferFormat = cr3d::DataFormat::D32_Float_S8_Uint;

// GBuffer
cr3d::DataFormat::T CrRendererConfig::GBufferAlbedoAOFormat = cr3d::DataFormat::RGBA8_Unorm;
cr3d::DataFormat::T CrRendererConfig::GBufferNormalsFormat = cr3d::DataFormat::RGBA8_Unorm;
cr3d::DataFormat::T CrRendererConfig::GBufferMaterialFormat = cr3d::DataFormat::RGBA8_Unorm;

cr3d::DataFormat::T CrRendererConfig::LightingFormat = cr3d::DataFormat::RG11B10_Float;

// Debug Shader
cr3d::DataFormat::T CrRendererConfig::DebugShaderFormat = cr3d::DataFormat::RGBA16_Unorm;

cr3d::DataFormat::T CrRendererConfig::SwapchainFormat = cr3d::DataFormat::BGRA8_Unorm;