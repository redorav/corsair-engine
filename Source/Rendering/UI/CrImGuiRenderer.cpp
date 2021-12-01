#include "CrRendering_pch.h"

#include "CrImGuiRenderer.h"

#include "Input/CrInputManager.h"
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
#include "GeneratedShaders/ShaderMetadata.h"

#include "imgui.h"

#include "Core/CrGlobalPaths.h"

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

CrImGuiRenderer* CrImGuiRenderer::k_instance = nullptr;

CrImGuiRenderer::CrImGuiRenderer()
	: m_currentMaxIndexCount(0)
	, m_currentMaxVertexCount(0)
{
}

CrImGuiRenderer* CrImGuiRenderer::GetImGuiRenderer()
{
	if (!k_instance)
	{
		k_instance = new CrImGuiRenderer;
	}
	return k_instance;
}

void CrImGuiRenderer::Initialize(const CrImGuiRendererInitParams& initParams)
{
	m_initParams = initParams;

	// Generic ImGui setup:
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	static_assert(sizeof(ImDrawVert) == sizeof(UIVertex), "ImGui vertex declaration doesn't match");

	CrRenderDeviceSharedHandle renderDevice = ICrRenderSystem::GetRenderDevice();

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

		const CrString& ShaderSourceDirectory = CrGlobalPaths::GetShaderSourceDirectory();

		// Load shaders:
		CrShaderCompilationDescriptor bytecodeDesc;
		bytecodeDesc.AddBytecodeDescriptor(CrShaderBytecodeCompilationDescriptor(CrPath((ShaderSourceDirectory + "UI.hlsl").c_str()), 
			"ImguiVS", cr3d::ShaderStage::Vertex, cr3d::GraphicsApi::Vulkan, cr::Platform::Windows));

		bytecodeDesc.AddBytecodeDescriptor(CrShaderBytecodeCompilationDescriptor(CrPath((ShaderSourceDirectory + "UI.hlsl").c_str()), 
			"ImguiPS", cr3d::ShaderStage::Pixel, cr3d::GraphicsApi::Vulkan, cr::Platform::Windows));

		CrGraphicsShaderHandle shaders = CrShaderManager::Get().CompileGraphicsShader(bytecodeDesc);

		// Create it:
		m_uiGraphicsPipeline = CrPipelineStateManager::Get().GetGraphicsPipeline(psoDescriptor, shaders, UIVertexDescriptor);
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
		io.KeyMap[ImGuiKey_Tab]         = KeyboardKey::Tab;
		io.KeyMap[ImGuiKey_LeftArrow]   = KeyboardKey::LeftArrow;
		io.KeyMap[ImGuiKey_RightArrow]  = KeyboardKey::RightArrow;
		io.KeyMap[ImGuiKey_UpArrow]     = KeyboardKey::UpArrow;
		io.KeyMap[ImGuiKey_DownArrow]   = KeyboardKey::DownArrow;
		io.KeyMap[ImGuiKey_PageUp]      = KeyboardKey::PageUp;
		io.KeyMap[ImGuiKey_PageDown]    = KeyboardKey::PageDown;
		io.KeyMap[ImGuiKey_Home]        = KeyboardKey::Home;
		io.KeyMap[ImGuiKey_End]         = KeyboardKey::End;
		io.KeyMap[ImGuiKey_Insert]      = KeyboardKey::Insert;
		io.KeyMap[ImGuiKey_Delete]      = KeyboardKey::Delete;
		io.KeyMap[ImGuiKey_Backspace]   = KeyboardKey::Backspace;
		io.KeyMap[ImGuiKey_Space]       = KeyboardKey::Space;
		io.KeyMap[ImGuiKey_Enter]       = KeyboardKey::Intro;
		io.KeyMap[ImGuiKey_Escape]      = KeyboardKey::Escape;
		io.KeyMap[ImGuiKey_KeyPadEnter] = KeyboardKey::KeypadEnter;
		io.KeyMap[ImGuiKey_A]           = KeyboardKey::A;
		io.KeyMap[ImGuiKey_C]           = KeyboardKey::C;
		io.KeyMap[ImGuiKey_V]           = KeyboardKey::V;
		io.KeyMap[ImGuiKey_X]           = KeyboardKey::X;
		io.KeyMap[ImGuiKey_Y]           = KeyboardKey::Y;
		io.KeyMap[ImGuiKey_Z]           = KeyboardKey::Z;
	}
	
	// Default linear clamp sampler state:
	CrSamplerDescriptor descriptor;
	descriptor.name = "Imgui Sampler State";
	m_uiSamplerState = renderDevice->CreateSampler(descriptor);

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
	io.MouseDown[0] = mouseState.buttonPressed[MouseButton::Left];
	io.MouseDown[1] = mouseState.buttonPressed[MouseButton::Right];
	io.MouseDown[2] = mouseState.buttonPressed[MouseButton::Middle];
	io.MousePos = ImVec2((float)mouseState.position.x, (float)mouseState.position.y);
	io.MouseWheel = (float)mouseState.mouseWheel.y;

	const KeyboardState& keyboardState = CrInput.GetKeyboardState();

	for (uint32_t k = 0; k < KeyboardKey::Count; ++k)
	{
		io.KeysDown[k] = keyboardState.keyPressed[k];
	}

	ImGui::NewFrame();
}

void CrImGuiRenderer::Render(ICrCommandBuffer* commandBuffer, const ICrTexture* swapchainTexture)
{
	ImGui::Render();

	// Query the draw data for this frame:
	ImDrawData* data = ImGui::GetDrawData();
	if (!data || !data->Valid || !data->CmdListsCount || (data->DisplaySize.x * data->DisplaySize.y) <= 0.0f)
	{
		return;
	}

	// Check index buffer size. By default indices are unsigned shorts (ImDrawIdx):
	CrGPUBuffer indexBuffer = commandBuffer->AllocateIndexBuffer(data->TotalIdxCount);
	CrGPUBuffer vertexBuffer = commandBuffer->AllocateVertexBuffer(data->TotalVtxCount);

	// Update contents:
	ImDrawIdx* pIdx = (ImDrawIdx*)indexBuffer.Lock();
	ImDrawVert* pVtx = (ImDrawVert*)vertexBuffer.Lock();
	for (int i = 0; i < data->CmdListsCount; ++i)
	{
		ImDrawList* drawList = data->CmdLists[i];

		memcpy(pIdx, drawList->IdxBuffer.Data, drawList->IdxBuffer.Size * sizeof(ImDrawIdx));
		memcpy(pVtx, drawList->VtxBuffer.Data, drawList->VtxBuffer.Size * sizeof(ImDrawVert));

		pIdx += drawList->IdxBuffer.Size;
		pVtx += drawList->VtxBuffer.Size;
	}
	indexBuffer.Unlock();
	vertexBuffer.Unlock();

	CrRenderPassDescriptor graphicsRenderPass;

	CrRenderTargetDescriptor swapchainAttachment;
	swapchainAttachment.texture = swapchainTexture;
	swapchainAttachment.loadOp = CrRenderTargetLoadOp::Load;
	swapchainAttachment.initialState = cr3d::TextureState::RenderTarget;
	swapchainAttachment.finalState = cr3d::TextureState::Present;

	graphicsRenderPass.color.push_back(swapchainAttachment);

	commandBuffer->BeginDebugEvent("ImGui Render", float4(0.3f, 0.3f, 0.6f, 1.0f));
	{
		commandBuffer->BeginRenderPass(graphicsRenderPass);
		{
			// Setup global config:
			commandBuffer->BindGraphicsPipelineState(m_uiGraphicsPipeline.get());
			commandBuffer->BindIndexBuffer(&indexBuffer);
			commandBuffer->BindVertexBuffer(&vertexBuffer, 0);
			commandBuffer->BindSampler(cr3d::ShaderStage::Pixel, Samplers::UISampleState, m_uiSamplerState.get());

			// Projection matrix. TODO: this could be cached.
			CrGPUBufferType<UIData> uiDataBuffer = commandBuffer->AllocateConstantBuffer<UIData>();
			UIDataData* uiData = uiDataBuffer.Lock();
			{
				uiData->projection = ComputeProjectionMatrix(data);
			}
			uiDataBuffer.Unlock();
			commandBuffer->BindConstantBuffer(&uiDataBuffer);

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
						commandBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::UITexture, texture);
						commandBuffer->SetScissor(CrScissor(x, y, width, height));
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
		}
		commandBuffer->EndRenderPass();
	}
	commandBuffer->EndDebugEvent();
}

float4x4 CrImGuiRenderer::ComputeProjectionMatrix(ImDrawData* data)
{
	float L = data->DisplayPos.x;
	float R = data->DisplayPos.x + data->DisplaySize.x;
	float T = data->DisplayPos.y;
	float B = data->DisplayPos.y + data->DisplaySize.y;
	return float4x4(
		float4(2.0f / (R - L),		0.0f,				0.0f, 0.0f),
		float4(0.0f,				2.0f / (T - B),		0.0f, 0.0f),
		float4(0.0f,				0.0f,				0.5f, 0.0f),
		float4((R + L) / (L - R), (T + B) / (B - T),	0.5f, 1.0f)
	);
}