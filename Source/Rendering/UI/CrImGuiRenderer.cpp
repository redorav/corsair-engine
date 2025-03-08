#include "Rendering/CrRendering_pch.h"

#include "CrImGuiRenderer.h"

#include "Core/Input/CrInputManager.h"
#include "Core/CrPlatform.h"
#include "Core/CrFrameTime.h"

#include "Rendering/CrGPUBuffer.h"
#include "Rendering/ICrShader.h"
#include "Rendering/CrShaderManager.h"
#include "Rendering/ICrTexture.h"
#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/ICrCommandBuffer.h"
#include "Rendering/ICrSampler.h"
#include "Rendering/CrRenderPassDescriptor.h"
#include "Rendering/CrRenderGraph.h"
#include "Rendering/CrRenderingResources.h"
#include "Rendering/CrRendering.h"

#include "CrOSWindow.h"

#include "Editor/CrImGuiViewports.h"

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

void CrImGuiRenderer::Initialize(const CrImGuiRendererInitParams& initParams)
{
	CrAssert(ImGuiRenderer == nullptr);
	ImGuiRenderer = new CrImGuiRenderer(initParams);
}

void CrImGuiRenderer::Deinitialize()
{
	CrAssert(ImGuiRenderer != nullptr);
	delete ImGuiRenderer;
}

CrImGuiRenderer::CrImGuiRenderer(const CrImGuiRendererInitParams& initParams)
{
	m_initParams = initParams;

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	static_assert(sizeof(ImDrawVert) == sizeof(UIVertex), "ImGui vertex declaration doesn't match");

	CrRenderDeviceHandle renderDevice = RenderSystem->GetRenderDevice();

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
	
	// Set up Imgui style
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Default resolution for the first frame, we need to query the real viewport during NewFrame()
	io.DisplaySize = ImVec2(1920.0f, 1080.0f);
}

void CrImGuiRenderer::NewFrame(const crstl::intrusive_ptr<CrOSWindow>& mainWindow)
{
	ImGuiIO& io = ImGui::GetIO();

	uint32_t windowWidth, windowHeight;
	mainWindow->GetSizePixels(windowWidth, windowHeight);

	io.DisplaySize = ImVec2((float)windowWidth, (float)windowHeight);
	io.DeltaTime = (float)CrFrameTime::GetFrameDelta().seconds();
	if (io.DeltaTime == 0.0f)
	{
		io.DeltaTime = (float)(1.0f / 60.0f);
	}

	// Update Mouse Cursor (perhaps this needs to be in ImguiViewports?)

	if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0)
	{
		static CursorType::T prevCursorType = CursorType::Count;

		ImGuiMouseCursor imguiMouseCursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();

		CursorType::T cursorType = CursorType::Count;

		switch (imguiMouseCursor)
		{
			case ImGuiMouseCursor_None:       cursorType = CursorType::None; break;
			case ImGuiMouseCursor_Arrow:      cursorType = CursorType::Arrow; break;
			case ImGuiMouseCursor_TextInput:  cursorType = CursorType::TextInput; break;
			case ImGuiMouseCursor_ResizeAll:  cursorType = CursorType::ResizeAll; break;
			case ImGuiMouseCursor_ResizeEW:   cursorType = CursorType::ResizeEW; break;
			case ImGuiMouseCursor_ResizeNS:   cursorType = CursorType::ResizeNS; break;
			case ImGuiMouseCursor_ResizeNESW: cursorType = CursorType::ResizeNESW; break;
			case ImGuiMouseCursor_ResizeNWSE: cursorType = CursorType::ResizeNWSE; break;
			case ImGuiMouseCursor_Hand:       cursorType = CursorType::Hand; break;
			case ImGuiMouseCursor_NotAllowed: cursorType = CursorType::NotAllowed; break;
		}

		CrOSWindow::SetCursor(cursorType);
	}

	ImGui::NewFrame();
}

void CrImGuiRenderer::AddRenderPass(CrRenderGraph& renderGraph, const CrTextureHandle&)
{
	renderGraph.AddRenderPass(CrRenderGraphString("ImGui Render Viewports"), float4(0.3f, 0.3f, 0.6f, 1.0f), CrRenderGraphPassType::Behavior,
	[&](CrRenderGraph&)
	{
	},
	[this](const CrRenderGraph&, ICrCommandBuffer*)
	{
		ImGui::Render();
	});

	const ImGuiPlatformIO& imguiPlatformIO = ImGui::GetPlatformIO();

	for (int i = 0; i < imguiPlatformIO.Viewports.Size; ++i)
	{
		ImGuiViewport* imguiViewport = imguiPlatformIO.Viewports[i];
		ImGuiViewportsData* viewportData = (ImGuiViewportsData*)imguiViewport->PlatformUserData;
		CrOSWindow* osWindow = viewportData->osWindow;
		CrSwapchainHandle swapchain = osWindow->GetSwapchain();

		CrRenderGraphString imguiPassString;
		imguiPassString.append_sprintf("ImGui Render Viewport %i", i);

		renderGraph.AddRenderPass(imguiPassString, float4(0.3f, 0.3f, 0.6f, 1.0f), CrRenderGraphPassType::Graphics,
		[swapchain](CrRenderGraph& renderGraph)
		{
			renderGraph.BindRenderTarget(swapchain->GetCurrentTexture().get());
		},
		[this, imguiViewport, osWindow, swapchain](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
		{
			// Query the draw data for this frame:
			ImDrawData* data = imguiViewport->DrawData;
			if (!data || !data->Valid || !data->CmdListsCount || (data->DisplaySize.x * data->DisplaySize.y) <= 0.0f)
			{
				return;
			}

			// Check index buffer size. By default indices are unsigned shorts (ImDrawIdx):
			CrGPUBufferView indexBuffer = commandBuffer->AllocateIndexBuffer(data->TotalIdxCount, cr3d::DataFormat::R16_Uint);
			CrGPUBufferView vertexBuffer = commandBuffer->AllocateVertexBuffer(data->TotalVtxCount, sizeof(UIVertex));

			// Update contents:
			ImDrawIdx* indexBufferData = (ImDrawIdx*)indexBuffer.GetData();
			ImDrawVert* vertexBufferData = (ImDrawVert*)vertexBuffer.GetData();
			for (int cmdListIndex = 0; cmdListIndex < data->CmdListsCount; ++cmdListIndex)
			{
				ImDrawList* imDrawList = data->CmdLists[cmdListIndex];

				memcpy(indexBufferData, imDrawList->IdxBuffer.Data, imDrawList->IdxBuffer.Size * sizeof(ImDrawIdx));
				memcpy(vertexBufferData, imDrawList->VtxBuffer.Data, imDrawList->VtxBuffer.Size * sizeof(ImDrawVert));

				indexBufferData += imDrawList->IdxBuffer.Size;
				vertexBufferData += imDrawList->VtxBuffer.Size;
			}

			commandBuffer->BindGraphicsPipelineState(m_imguiGraphicsPipeline.get());
			commandBuffer->BindIndexBuffer(indexBuffer);
			commandBuffer->BindVertexBuffer(vertexBuffer, 0);
			commandBuffer->BindSampler(Samplers::UISampleState, RenderingResources->AllLinearClampSampler.get());
			commandBuffer->SetViewport(CrViewport(0, 0, swapchain->GetWidth(), swapchain->GetHeight()));

			// Projection matrix. TODO: this could be cached.
			CrGPUBufferViewT<UIData> uiDataBuffer = commandBuffer->AllocateConstantBuffer<UIData>();
			UIData* uiData = uiDataBuffer.GetData();
			{
				uiData->projection = ComputeProjectionMatrix(data);
			}
			commandBuffer->BindConstantBuffer(uiDataBuffer);

			// Iterate over each draw list -> draw command: 
			ImVec2 clipOffset = data->DisplayPos;
			uint32_t totalVertexOffset = 0;
			uint32_t totalIndexOffset = 0;
			for (int listIdx = 0; listIdx < data->CmdListsCount; ++listIdx)
			{
				const ImDrawList* imDrawList = data->CmdLists[listIdx];
				for (int cmdIdx = 0; cmdIdx < imDrawList->CmdBuffer.Size; ++cmdIdx)
				{
					const ImDrawCmd* imDrawCmd = &imDrawList->CmdBuffer[cmdIdx];

					uint32_t x = (uint32_t)(imDrawCmd->ClipRect.x - clipOffset.x);
					uint32_t y = (uint32_t)(imDrawCmd->ClipRect.y - clipOffset.y);
					uint32_t width = (uint32_t)(imDrawCmd->ClipRect.z - x);
					uint32_t height = (uint32_t)(imDrawCmd->ClipRect.w - y);

					if (!imDrawCmd->UserCallback)
					{
						// Generic rendering.
						ICrTexture* texture = (ICrTexture*)imDrawCmd->TextureId;
						commandBuffer->BindTexture(Textures::UITexture, texture);
						commandBuffer->SetScissor(CrRectangle(x, y, width, height));
						commandBuffer->DrawIndexed(imDrawCmd->ElemCount, 1, imDrawCmd->IdxOffset + totalIndexOffset, imDrawCmd->VtxOffset + totalVertexOffset, 0);
					}
					else
					{
						// Handle user callback:
						if (imDrawCmd->UserCallback == ImDrawCallback_ResetRenderState)
						{
							// This is a special case to reset the render state.. What should we do here?
							CrAssertMsg(false, "Not implemented");
						}
						else
						{
							imDrawCmd->UserCallback(imDrawList, imDrawCmd);
						}
					}
				}

				totalIndexOffset += imDrawList->IdxBuffer.Size;
				totalVertexOffset += imDrawList->VtxBuffer.Size;
			}
		});
	}
}