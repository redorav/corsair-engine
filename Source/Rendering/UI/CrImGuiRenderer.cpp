#include "CrRendering_pch.h"

#include "Core/CrPlatform.h"

#include "CrImGuiRenderer.h"
#include "Rendering/CrGPUBuffer.h"
#include "Rendering/ICrRenderPass.h"
#include "Rendering/ICrShader.h"
#include "Rendering/ICrShaderManager.h"
#include "Rendering/ICrPipelineStateManager.h"
#include "Rendering/ICrTexture.h"
#include "Rendering/ICrRenderDevice.h"

#include "imgui.h"

// Based on ImDrawVert
struct UIVertex
{
	CrVertexElement<float, cr3d::DataFormat::RG16_Float> m_Position;
	CrVertexElement<float, cr3d::DataFormat::RG16_Float> m_UV;
	CrVertexElement<uint32_t, cr3d::DataFormat::R32_Uint> m_Color;

	static CrVertexDescriptor GetVertexDescriptor()
	{
		return { decltype(m_Position)::GetFormat(), decltype(m_UV)::GetFormat(),decltype(m_Color)::GetFormat() };
	}
};

CrImGuiRenderer* CrImGuiRenderer::k_Instance = nullptr;

CrImGuiRenderer::CrImGuiRenderer()
{
}

CrImGuiRenderer* CrImGuiRenderer::GetImGuiRenderer()
{
	if (!k_Instance)
	{
		k_Instance = new CrImGuiRenderer;
	}
	return k_Instance;
}

void CrImGuiRenderer::Init(CrRenderPassDescriptor* renderPassDesc)
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	// Pipeline desc:
	CrGraphicsPipelineDescriptor psoDescriptor;
	psoDescriptor.depthStencilState.depthTestEnable = false;
	psoDescriptor.depthStencilState.depthWriteEnable = false;
	psoDescriptor.Hash();

	// TODO: This should be defined in one place only
	CrString SHADER_PATH = IN_SRC_PATH;
	SHADER_PATH = SHADER_PATH + "Rendering/Shaders/";

	// Load shaders:
	CrBytecodeLoadDescriptor bytecodeDesc;
	bytecodeDesc.AddBytecodeDescriptor(CrShaderBytecodeDescriptor(
		CrPath((SHADER_PATH + "ui.hlsl").c_str()), "main_vs", cr3d::ShaderStage::Vertex, cr3d::ShaderCodeFormat::SourceHLSL, cr3d::GraphicsApi::Vulkan, cr::Platform::Windows
	));
	bytecodeDesc.AddBytecodeDescriptor(CrShaderBytecodeDescriptor(
		CrPath((SHADER_PATH + "ui.hlsl").c_str()), "main_ps", cr3d::ShaderStage::Pixel, cr3d::ShaderCodeFormat::SourceHLSL, cr3d::GraphicsApi::Vulkan, cr::Platform::Windows
	));
	CrGraphicsShaderHandle shaders = ICrShaderManager::Get()->LoadGraphicsShader(bytecodeDesc);

	// Create it:
	m_UIGfxPipeline = ICrPipelineStateManager::Get()->GetGraphicsPipeline(
		psoDescriptor, shaders, UIVertex::GetVertexDescriptor(), *renderPassDesc
	);

	// Font atlas:
	unsigned char* fontData = nullptr;
	int fontWidth, fontHeight;
	io.Fonts->GetTexDataAsRGBA32(&fontData, &fontWidth, &fontHeight);
	
	CrTextureCreateParams fontParams;
	fontParams.width = (uint32_t)fontWidth;
	fontParams.height = (uint32_t)fontHeight;
	fontParams.format = cr3d::DataFormat::RGBA8_Unorm;
	fontParams.name = "ImGui Font Atlas";
	fontParams.initialData = fontData;
	fontParams.initialDataSize = 4 * fontWidth * fontHeight; // Can't this be computed internally from tex params?

	m_FontAtlas = ICrRenderDevice::GetRenderDevice()->CreateTexture(fontParams);
	CrAssertMsg( m_FontAtlas.get(), "Failed to create the ImGui font atlas");
	
	io.Fonts->TexID = (ImTextureID)m_FontAtlas.get();

	// Default res for the first frame, we need to query the real viewport
	// during NewFrame()
	io.DisplaySize = ImVec2(1920.0f, 1080.0f); 
}

void CrImGuiRenderer::NewFrame(uint32_t width, uint32_t height)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)width, (float)height);

	ImGui::NewFrame();
}

void CrImGuiRenderer::Render()
{
	ImGui::Render();
	ImDrawData* data = ImGui::GetDrawData();
	if (!data || !data->CmdListsCount || (data->DisplaySize.x * data->DisplaySize.y) <= 0.0f)
	{
		return;
	}
}
