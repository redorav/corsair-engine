#pragma once

#include "Core/SmartPointers/CrSharedPtr.h"
#include "Rendering/CrRendering.h"

#include "Rendering/CrRenderingForwardDeclarations.h"
class ICrGraphicsPipeline;
struct CrRenderPassDescriptor;
struct ImDrawData;

struct CrImGuiRendererInitParams
{
	cr3d::DataFormat::T m_Format;
	cr3d::SampleCount m_SampleCount;
};

class CrImGuiRenderer
{
private:
	CrImGuiRenderer();
	CrImGuiRenderer(const CrImGuiRenderer& other) = delete;
	~CrImGuiRenderer() = delete;

public:
	static CrImGuiRenderer* GetImGuiRenderer();
	void Init(const CrImGuiRendererInitParams& initParams);
	void NewFrame(uint32_t width, uint32_t height);
	void Render(ICrCommandBuffer* cmdBuffer, const ICrFramebuffer* output);

private:
	float4x4 GetProjection(ImDrawData* data);
	void UpdateBuffers(ImDrawData* data);

	static CrImGuiRenderer* k_instance;

	CrRenderPassSharedHandle m_renderPass;
	CrGraphicsPipelineHandle m_uiGfxPipeline;
	CrIndexBufferSharedHandle m_indexBuffer;
	CrVertexBufferSharedHandle m_vertexBuffer;
	CrTextureSharedHandle m_fontAtlas;
	CrSamplerSharedHandle m_uiSamplerState;

	uint32_t m_curMaxIndexCount;
	uint32_t m_curMaxVertexCount;

	CrImGuiRendererInitParams m_initParams;
};