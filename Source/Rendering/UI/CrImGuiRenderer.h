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
	void Render(ImDrawData* data);

private:
	static CrImGuiRenderer* k_Instance;

	ICrGraphicsPipeline* m_UIGfxPipeline;
	CrIndexBufferSharedHandle m_IndexBuffer;
	CrVertexBufferSharedHandle m_VertexBuffer;
};