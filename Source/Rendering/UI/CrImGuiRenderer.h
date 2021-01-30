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

	static CrImGuiRenderer* k_Instance;

	CrRenderPassSharedHandle m_RenderPass;
	ICrGraphicsPipeline* m_UIGfxPipeline;
	CrIndexBufferSharedHandle m_IndexBuffer;
	CrVertexBufferSharedHandle m_VertexBuffer;
	CrTextureSharedHandle m_FontAtlas;
	CrSamplerSharedHandle m_UISamplerState;

	uint32_t m_CurMaxIndexCount;
	uint32_t m_CurMaxVertexCount;

	CrImGuiRendererInitParams m_InitParams;
};