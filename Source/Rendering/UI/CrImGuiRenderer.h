#pragma once

#include "Core/SmartPointers/CrSharedPtr.h"

#include "Rendering/CrRenderingForwardDeclarations.h"
class ICrGraphicsPipeline;
struct CrRenderPassDescriptor;
struct ImDrawData;

class CrImGuiRenderer
{
private:
	CrImGuiRenderer();
	CrImGuiRenderer(const CrImGuiRenderer& other) = delete;
	~CrImGuiRenderer() = delete;

public:
	static CrImGuiRenderer* GetImGuiRenderer();
	void Init(CrRenderPassDescriptor* renderPassDesc);
	void NewFrame(uint32_t width, uint32_t height);
	void Render(ICrCommandBuffer* cmdBuffer);

private:
	float4x4 GetProjection(ImDrawData* data);
	void UpdateBuffers(ImDrawData* data);

	static CrImGuiRenderer* k_Instance;

	ICrGraphicsPipeline* m_UIGfxPipeline;
	CrIndexBufferSharedHandle m_IndexBuffer;
	CrVertexBufferSharedHandle m_VertexBuffer;
	CrTextureSharedHandle m_FontAtlas;
	CrSamplerSharedHandle m_UISamplerState;

	uint32_t m_CurMaxIndexCount;
	uint32_t m_CurMaxVertexCount;
};