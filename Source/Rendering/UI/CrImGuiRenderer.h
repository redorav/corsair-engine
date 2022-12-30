#pragma once

#include "Core/SmartPointers/CrIntrusivePtr.h"

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrBuiltinPipeline.h"
#include "Rendering/ICrTexture.h"

struct ImDrawData;

struct CrImGuiRendererInitParams
{
	cr3d::DataFormat::T m_swapchainFormat;
	cr3d::SampleCount m_sampleCount;
};

class CrImGuiRenderer
{
public:

	static void Create(const CrImGuiRendererInitParams& initParams);

	static void Destroy();

	static CrImGuiRenderer& Get();

	void Initialize(const CrImGuiRendererInitParams& initParams);

	void NewFrame(uint32_t width, uint32_t height);

	void Render(CrRenderGraph& renderGraph, CrRenderGraphTextureId swapchainTextureId);

private:

	CrImGuiRenderer();

	CrImGuiRenderer(const CrImGuiRenderer& other) = delete;

	CrBuiltinGraphicsPipeline m_imguiGraphicsPipeline;

	CrTextureHandle m_fontAtlas;

	CrImGuiRendererInitParams m_initParams;
};