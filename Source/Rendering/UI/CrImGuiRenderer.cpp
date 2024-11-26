#include "Rendering/CrRendering_pch.h"

#include "CrImGuiRenderer.h"

#include "Core/Input/CrInputManager.h"
#include "Core/CrPlatform.h"
#include "Core/CrFrameTime.h"

#include "Rendering/CrGPUBuffer.h"
#include "Rendering/ICrShader.h"
#include "Rendering/CrShaderManager.h"
#include "Rendering/CrPipelineStateManager.h"
#include "Rendering/ICrTexture.h"
#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/ICrCommandBuffer.h"
#include "Rendering/ICrSampler.h"
#include "Rendering/CrRenderPassDescriptor.h"
#include "Rendering/CrRenderGraph.h"
#include "Rendering/CrRenderingResources.h"
#include "Rendering/CrRendering.h"

#include "GeneratedShaders/ShaderMetadata.h"

#include "imgui.h"

#include "Core/CrGlobalPaths.h"
#include "GeneratedShaders/BuiltinShaders.h"

#include "Math/CrHlslppMatrixFloat.h"

// Based on ImDrawVert
struct UIVertex
{
	CrVertexElement<float, cr3d::DataFormat::RG32_Float> position;
	CrVertexElement<float, cr3d::DataFormat::RG32_Float> uv;
	CrVertexElement<uint8_t, cr3d::DataFormat::RGBA8_Unorm> color;
};

CrVertexDescriptor UIVertexDescriptor =
{
	CrVertexAttribute(CrVertexSemantic::Position, cr3d::DataFormat::RG32_Float, 0),
	CrVertexAttribute(CrVertexSemantic::TexCoord0, cr3d::DataFormat::RG32_Float, 0),
	CrVertexAttribute(CrVertexSemantic::Color, cr3d::DataFormat::RGBA8_Unorm, 0),
};

float4x4 ComputeProjectionMatrix(ImDrawData* data)
{
	float L = data->DisplayPos.x;
	float R = data->DisplayPos.x + data->DisplaySize.x;
	float T = data->DisplayPos.y;
	float B = data->DisplayPos.y + data->DisplaySize.y;
	return float4x4(
		float4(2.0f / (R - L), 0.0f, 0.0f, 0.0f),
		float4(0.0f, 2.0f / (T - B), 0.0f, 0.0f),
		float4(0.0f, 0.0f, 0.5f, 0.0f),
		float4((R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f)
	);
}

CrImGuiRenderer* ImGuiRenderer = nullptr;

CrImGuiRenderer::CrImGuiRenderer()
{
}

void CrImGuiRenderer::Initialize(const CrImGuiRendererInitParams& initParams)
{
	CrAssert(ImGuiRenderer == nullptr);
	ImGuiRenderer = new CrImGuiRenderer;
	ImGuiRenderer->Initialize(initParams);
}

void CrImGuiRenderer::Destroy()
{
	CrAssert(ImGuiRenderer != nullptr);
	delete ImGuiRenderer;
}

CrImGuiRenderer& CrImGuiRenderer::Get()
{
	CrAssert(ImGuiRenderer != nullptr);
	return *ImGuiRenderer;
}

static ImGuiKey s_ImguiKeys[KeyboardKey::Code::Count] = {};

void CrImGuiRenderer::Initialize(const CrImGuiRendererInitParams& initParams)
{
	m_initParams = initParams;

	// Generic ImGui setup:
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	static_assert(sizeof(ImDrawVert) == sizeof(UIVertex), "ImGui vertex declaration doesn't match");

	CrRenderDeviceHandle renderDevice = ICrRenderSystem::GetRenderDevice();

	// Pipeline description:
	{
		CrGraphicsPipelineDescriptor psoDescriptor;
		psoDescriptor.depthStencilState.depthTestEnable = false;
		psoDescriptor.depthStencilState.depthWriteEnable = false;
		psoDescriptor.blendState.renderTargetBlends[0].enable = true;
		psoDescriptor.blendState.renderTargetBlends[0].srcColorBlendFactor = cr3d::BlendFactor::SrcAlpha;
		psoDescriptor.blendState.renderTargetBlends[0].dstColorBlendFactor = cr3d::BlendFactor::OneMinusSrcAlpha;
		psoDescriptor.blendState.renderTargetBlends[0].colorBlendOp = cr3d::BlendOp::Add;
		psoDescriptor.blendState.renderTargetBlends[0].srcAlphaBlendFactor = cr3d::BlendFactor::OneMinusSrcAlpha;
		psoDescriptor.blendState.renderTargetBlends[0].dstAlphaBlendFactor = cr3d::BlendFactor::Zero;
		psoDescriptor.blendState.renderTargetBlends[0].alphaBlendOp = cr3d::BlendOp::Add;

		psoDescriptor.renderTargets.colorFormats[0] = initParams.m_swapchainFormat;

		psoDescriptor.rasterizerState.cullMode = cr3d::PolygonCullMode::None;

		m_imguiGraphicsPipeline = BuiltinPipelines->GetGraphicsPipeline(psoDescriptor, UIVertexDescriptor, CrBuiltinShaders::ImguiVS, CrBuiltinShaders::ImguiPS);
	}

	// Font atlas:
	{
		unsigned char* fontData = nullptr;
		int fontWidth, fontHeight;
		io.Fonts->GetTexDataAsRGBA32(&fontData, &fontWidth, &fontHeight);

		CrTextureDescriptor fontParams;
		fontParams.width = (uint32_t)fontWidth;
		fontParams.height = (uint32_t)fontHeight;
		fontParams.format = cr3d::DataFormat::RGBA8_Unorm;
		fontParams.name = "ImGui Font Atlas";
		fontParams.initialData = fontData;
		fontParams.initialDataSize = 4 * fontWidth * fontHeight; // Can't this be computed internally from texture params?

		m_fontAtlas = renderDevice->CreateTexture(fontParams);
		CrAssertMsg(m_fontAtlas.get(), "Failed to create the ImGui font atlas");
		io.Fonts->TexID = (ImTextureID)m_fontAtlas.get();
	}

	{
		s_ImguiKeys[KeyboardKey::Tab]         = ImGuiKey_Tab;
		s_ImguiKeys[KeyboardKey::LeftArrow]   = ImGuiKey_LeftArrow;
		s_ImguiKeys[KeyboardKey::RightArrow]  = ImGuiKey_RightArrow;
		s_ImguiKeys[KeyboardKey::UpArrow]     = ImGuiKey_UpArrow;
		s_ImguiKeys[KeyboardKey::DownArrow]   = ImGuiKey_DownArrow;
		s_ImguiKeys[KeyboardKey::PageUp]      = ImGuiKey_PageUp;
		s_ImguiKeys[KeyboardKey::PageDown]    = ImGuiKey_PageDown;
		s_ImguiKeys[KeyboardKey::Home]        = ImGuiKey_Home;
		s_ImguiKeys[KeyboardKey::End]         = ImGuiKey_End;
		s_ImguiKeys[KeyboardKey::Insert]      = ImGuiKey_Insert;
		s_ImguiKeys[KeyboardKey::Delete]      = ImGuiKey_Delete;
		s_ImguiKeys[KeyboardKey::Backspace]   = ImGuiKey_Backspace;
		s_ImguiKeys[KeyboardKey::Space]       = ImGuiKey_Space;
		s_ImguiKeys[KeyboardKey::Intro]       = ImGuiKey_Enter;
		s_ImguiKeys[KeyboardKey::Escape]      = ImGuiKey_Escape;
		s_ImguiKeys[KeyboardKey::KeypadEnter] = ImGuiKey_KeypadEnter;
		s_ImguiKeys[KeyboardKey::A]           = ImGuiKey_A;
		s_ImguiKeys[KeyboardKey::C]           = ImGuiKey_C;
		s_ImguiKeys[KeyboardKey::V]           = ImGuiKey_V;
		s_ImguiKeys[KeyboardKey::X]           = ImGuiKey_X;
		s_ImguiKeys[KeyboardKey::Y]           = ImGuiKey_Y;
		s_ImguiKeys[KeyboardKey::Z]           = ImGuiKey_Z;
	}
	
	// Default resolution for the first frame, we need to query the real viewport during NewFrame()
	io.DisplaySize = ImVec2(1920.0f, 1080.0f);
}

void CrImGuiRenderer::NewFrame(uint32_t width, uint32_t height)
{
	ImGuiIO& io = ImGui::GetIO();

	// Generic io:
	io.DisplaySize = ImVec2((float)width, (float)height);
	io.DeltaTime = (float)CrFrameTime::GetFrameDelta().AsSeconds();
		
	// Update input:
	const MouseState& mouseState = CrInput.GetMouseState();
	io.MouseDown[0] = mouseState.buttonHeld[MouseButton::Left];
	io.MouseDown[1] = mouseState.buttonHeld[MouseButton::Right];
	io.MouseDown[2] = mouseState.buttonHeld[MouseButton::Middle];
	io.MousePos = ImVec2((float)mouseState.position.x, (float)mouseState.position.y);
	io.MouseWheel = (float)mouseState.mouseWheel.y;

	const KeyboardState& keyboardState = CrInput.GetKeyboardState();

	for (uint32_t k = 0; k < KeyboardKey::Count; ++k)
	{
		KeyboardKey::Code key = (KeyboardKey::Code)keyboardState.keyHeld[k];
		io.KeysData[s_ImguiKeys[key]].Down = true;
	}

	ImGui::NewFrame();
}

void CrImGuiRenderer::Render(CrRenderGraph& renderGraph, const CrTextureHandle& swapchainTexture)
{
	renderGraph.AddRenderPass(CrRenderGraphString("ImGui Render"), float4(0.3f, 0.3f, 0.6f, 1.0f), CrRenderGraphPassType::Graphics,
	[&](CrRenderGraph& renderGraph)
	{
		renderGraph.BindRenderTarget(swapchainTexture.get());
	},
	[this](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
	{
		ImGui::Render();

		// Query the draw data for this frame:
		ImDrawData* data = ImGui::GetDrawData();
		if (!data || !data->Valid || !data->CmdListsCount || (data->DisplaySize.x * data->DisplaySize.y) <= 0.0f)
		{
			return;
		}

		// Check index buffer size. By default indices are unsigned shorts (ImDrawIdx):
		CrGPUBufferView indexBuffer = commandBuffer->AllocateIndexBuffer(data->TotalIdxCount, cr3d::DataFormat::R16_Uint);
		CrGPUBufferView vertexBuffer = commandBuffer->AllocateVertexBuffer(data->TotalVtxCount, sizeof(UIVertex));

		// Update contents:
		ImDrawIdx* pIdx = (ImDrawIdx*)indexBuffer.GetData();
		ImDrawVert* pVtx = (ImDrawVert*)vertexBuffer.GetData();
		for (int i = 0; i < data->CmdListsCount; ++i)
		{
			ImDrawList* drawList = data->CmdLists[i];

			memcpy(pIdx, drawList->IdxBuffer.Data, drawList->IdxBuffer.Size * sizeof(ImDrawIdx));
			memcpy(pVtx, drawList->VtxBuffer.Data, drawList->VtxBuffer.Size * sizeof(ImDrawVert));

			pIdx += drawList->IdxBuffer.Size;
			pVtx += drawList->VtxBuffer.Size;
		}

		// Setup global config:
		commandBuffer->BindGraphicsPipelineState(m_imguiGraphicsPipeline.get());
		commandBuffer->BindIndexBuffer(indexBuffer);
		commandBuffer->BindVertexBuffer(vertexBuffer, 0);
		commandBuffer->BindSampler(Samplers::UISampleState, CrRenderingResources::Get().AllLinearClampSampler.get());

		// Projection matrix. TODO: this could be cached.
		CrGPUBufferViewT<UIData> uiDataBuffer = commandBuffer->AllocateConstantBuffer<UIData>();
		UIData* uiData = uiDataBuffer.GetData();
		{
			uiData->projection = ComputeProjectionMatrix(data);
		}
		commandBuffer->BindConstantBuffer(uiDataBuffer);

		// Iterate over each draw list -> draw command: 
		ImVec2 clipOffset = data->DisplayPos;
		uint32_t acumVtxOffset = 0;
		uint32_t acumIdxOffset = 0;
		for (int listIdx = 0; listIdx < data->CmdListsCount; ++listIdx)
		{
			const ImDrawList* drawList = data->CmdLists[listIdx];
			for (int cmdIdx = 0; cmdIdx < drawList->CmdBuffer.Size; ++cmdIdx)
			{
				const ImDrawCmd* drawCmd = &drawList->CmdBuffer[cmdIdx];

				uint32_t x = (uint32_t)(drawCmd->ClipRect.x - clipOffset.x);
				uint32_t y = (uint32_t)(drawCmd->ClipRect.y - clipOffset.y);
				uint32_t width = (uint32_t)(drawCmd->ClipRect.z - x);
				uint32_t height = (uint32_t)(drawCmd->ClipRect.w - y);

				if (!drawCmd->UserCallback)
				{
					// Generic rendering.
					ICrTexture* texture = (ICrTexture*)drawCmd->TextureId;
					commandBuffer->BindTexture(Textures::UITexture, texture);
					commandBuffer->SetScissor(CrRectangle(x, y, width, height));
					commandBuffer->DrawIndexed(drawCmd->ElemCount, 1, drawCmd->IdxOffset + acumIdxOffset, drawCmd->VtxOffset + acumVtxOffset, 0);
				}
				else
				{
					// Handle user callback:
					if (drawCmd->UserCallback == ImDrawCallback_ResetRenderState)
					{
						// This is a special case to reset the render state.. What should we do here?
						CrAssertMsg(false, "Not implemented");
					}
					else
					{
						drawCmd->UserCallback(drawList, drawCmd);
					}
				}
			}
			acumIdxOffset += drawList->IdxBuffer.Size;
			acumVtxOffset += drawList->VtxBuffer.Size;
		}
	});
}