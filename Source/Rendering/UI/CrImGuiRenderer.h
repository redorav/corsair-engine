#pragma once

#include "Core/SmartPointers/CrSharedPtr.h"
#include "Rendering/CrRendering.h"

#include "Rendering/CrRenderingForwardDeclarations.h"
class ICrGraphicsPipeline;
struct CrRenderPassDescriptor;
struct ImDrawData;

struct CrImGuiRendererInitParams
{
	cr3d::DataFormat::T m_swapchainFormat;
	cr3d::SampleCount m_sampleCount;
};

class CrImGuiRenderer
{
public:

	static CrImGuiRenderer* GetImGuiRenderer();

	void Init(const CrImGuiRendererInitParams& initParams);

	void NewFrame(uint32_t width, uint32_t height);

	void Render(ICrCommandBuffer* cmdBuffer, const ICrFramebuffer* output);

private:

	CrImGuiRenderer();

	CrImGuiRenderer(const CrImGuiRenderer& other) = delete;

	~CrImGuiRenderer() = delete;

	float4x4 ComputeProjectionMatrix(ImDrawData* data);

	void UpdateBuffers(ImDrawData* data);

	static CrImGuiRenderer* k_instance;

	CrRenderPassSharedHandle m_renderPass;

	CrGraphicsPipelineHandle m_uiGraphicsPipeline;

	CrIndexBufferSharedHandle m_indexBuffer;

	CrVertexBufferSharedHandle m_vertexBuffer;

	CrTextureSharedHandle m_fontAtlas;

	CrSamplerSharedHandle m_uiSamplerState;

	// Imgui's vertex and index buffers grow if more vertices
	// are needed, but never shrink
	uint32_t m_currentMaxIndexCount;

	uint32_t m_currentMaxVertexCount;

	CrImGuiRendererInitParams m_initParams;
};