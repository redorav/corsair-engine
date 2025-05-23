#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrBuiltinPipeline.h"
#include "Rendering/ICrTexture.h"

#include "crstl/intrusive_ptr.h"

struct ImDrawData;

struct CrImGuiRendererInitParams
{
	cr3d::DataFormat::T m_swapchainFormat;
	cr3d::SampleCount m_sampleCount;
};

class CrOSWindow;

class CrImGuiRenderer
{
public:

	static void Initialize(const CrImGuiRendererInitParams& initParams);

	static void Deinitialize();

	void NewFrame(const crstl::intrusive_ptr<CrOSWindow>& mainWindow);

	void AddRenderPass(CrRenderGraph& renderGraph, const CrTextureHandle& swapchainTexture);

private:

	CrImGuiRenderer(const CrImGuiRendererInitParams& initParams);

	CrImGuiRenderer(const CrImGuiRenderer& other) = delete;

	CrGraphicsPipelineHandle m_imguiGraphicsPipeline;

	CrTextureHandle m_fontAtlas;

	CrImGuiRendererInitParams m_initParams;
};

extern CrImGuiRenderer* ImGuiRenderer;