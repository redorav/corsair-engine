#pragma once

#include "Core/SmartPointers/CrSharedPtr.h"
#include "Rendering/CrRendering.h"

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrBuiltinPipeline.h"

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

	void UpdateBuffers(ImDrawData* data);

	CrBuiltinGraphicsPipeline m_imguiGraphicsPipeline;

	CrTextureHandle m_fontAtlas;

	// Imgui's vertex and index buffers grow if more vertices
	// are needed, but never shrink
	uint32_t m_currentMaxIndexCount;

	uint32_t m_currentMaxVertexCount;

	CrImGuiRendererInitParams m_initParams;
};