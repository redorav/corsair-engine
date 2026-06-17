#pragma once

#include "Graphics/CrGraphicsForwardDeclarations.h"
#include "Graphics/CrBuiltinPipeline.h"
#include "Graphics/ITexture.h"

#include "crstl/intrusive_ptr.h"

struct ImDrawData;

struct CrImGuiRendererInitParams
{
	crgfx::DataFormat::T m_swapchainFormat;
	crgfx::SampleCount m_sampleCount;
};

class CrOSWindow;

class CrImGuiRenderer
{
public:

	static void Initialize(const CrImGuiRendererInitParams& initParams);

	static void Deinitialize();

	void NewFrame(const crstl::intrusive_ptr<CrOSWindow>& mainWindow);

	void AddRenderPass(CrRenderGraph& renderGraph, const crgfx::TextureHandle& swapchainTexture);

private:

	CrImGuiRenderer(const CrImGuiRendererInitParams& initParams);

	CrImGuiRenderer(const CrImGuiRenderer& other) = delete;

	crgfx::GraphicsPipelineHandle m_imguiGraphicsPipeline;

	crgfx::TextureHandle m_fontAtlas;

	CrImGuiRendererInitParams m_initParams;
};

extern CrImGuiRenderer* ImGuiRenderer;