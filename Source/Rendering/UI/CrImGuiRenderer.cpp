#include "CrRendering_pch.h"

#include "CrImGuiRenderer.h"

#include "CrInputManager.h"
#include "Core/CrPlatform.h"
#include "Core/CrFrameTime.h"
#include "Rendering/CrGPUBuffer.h"
#include "Rendering/ICrRenderPass.h"
#include "Rendering/ICrShader.h"
#include "Rendering/ICrShaderManager.h"
#include "Rendering/ICrPipelineStateManager.h"
#include "Rendering/ICrTexture.h"
#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/ICrCommandBuffer.h"
#include "Rendering/ICrSampler.h"
#include "GeneratedShaders/ShaderMetadata.h"

#include "imgui.h"

// Based on ImDrawVert
struct UIVertex
{
	CrVertexElement<float, cr3d::DataFormat::RG32_Float> position;
	CrVertexElement<float, cr3d::DataFormat::RG32_Float> uv;
	CrVertexElement<uint8_t, cr3d::DataFormat::RGBA8_Unorm> color;

	static CrVertexDescriptor GetVertexDescriptor()
	{
		return { decltype(position)::GetFormat(), decltype(uv)::GetFormat(), decltype(color)::GetFormat() };
	}
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

void CrImGuiRenderer::Init(const CrImGuiRendererInitParams& initParams)
{
	m_initParams = initParams;

	// Generic ImGui setup:
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	static_assert(sizeof(ImDrawVert) == sizeof(UIVertex), "ImGui vertex declaration doesn't match");

	CrRenderDeviceSharedHandle renderDevice = ICrRenderSystem::GetRenderDevice();

	// Setup render pass used to blit the UI:
	CrRenderPassDescriptor renderPassDescriptor;

	renderPassDescriptor.m_colorAttachments[0] = CrAttachmentDescriptor(
		initParams.m_swapchainFormat, initParams.m_sampleCount,
		CrAttachmentLoadOp::Load , CrAttachmentStoreOp::Store,
		CrAttachmentLoadOp::DontCare, CrAttachmentStoreOp::DontCare,
		cr3d::ResourceState::Present, cr3d::ResourceState::Present
	);

	m_renderPass = renderDevice->CreateRenderPass(renderPassDescriptor);

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
		psoDescriptor.Hash();

		// TODO Rework shader paths
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
		m_uiGraphicsPipeline = ICrPipelineStateManager::Get()->GetGraphicsPipeline(psoDescriptor, shaders, UIVertex::GetVertexDescriptor(), renderPassDescriptor);
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
	
	// Default linear clamp sampler state:
	CrSamplerDescriptor descriptor;
	m_uiSamplerState = renderDevice->CreateSampler(descriptor);

	// Default resolution for the first frame, we need to query the real viewport during NewFrame()
	io.DisplaySize = ImVec2(1920.0f, 1080.0f); 
}

void CrImGuiRenderer::NewFrame(uint32_t width, uint32_t height)
{
	ImGuiIO& io = ImGui::GetIO();

	// Generic io:
	io.DisplaySize = ImVec2((float)width, (float)height);
	io.DeltaTime = CrFrameTime::GetFrameDelta();
		
	// Update input:
	io.MouseDown[0] = CrInput.GetKey(KeyCode::MouseLeft);
	io.MouseDown[1] = CrInput.GetKey(KeyCode::MouseRight);
	io.MouseDown[2] = CrInput.GetKey(KeyCode::MouseMiddle);
	float mx = CrInput.GetAxis(AxisCode::MouseX);
	float my = CrInput.GetAxis(AxisCode::MouseY);
	io.MousePos = ImVec2(mx * io.DisplaySize.x, my * io.DisplaySize.y);

	ImGui::NewFrame();
}

void CrImGuiRenderer::Render(ICrCommandBuffer* cmdBuffer, const ICrFramebuffer* output)
{
	ImGui::Render();

	// Query the draw data for this frame:
	ImDrawData* data = ImGui::GetDrawData();
	if (!data || !data->Valid || !data->CmdListsCount || (data->DisplaySize.x * data->DisplaySize.y) <= 0.0f)
	{
		return;
	}

	const ImGuiIO& io = ImGui::GetIO();

	UpdateBuffers(data);

	// Begin rendering the draw lists:
	CrRenderPassBeginParams passParams = {};
	passParams.clear = false;
	passParams.drawArea = { 0, 0, (uint32_t)io.DisplaySize.x, (uint32_t)io.DisplaySize.y };

	cmdBuffer->BeginDebugEvent("ImGui Render", float4(0.3f, 0.3f, 0.6f, 1.0f));
	{
		cmdBuffer->BeginRenderPass(m_renderPass.get(), output, passParams);
		{
			// Setup global config:
			cmdBuffer->BindGraphicsPipelineState(m_uiGraphicsPipeline.get());
			cmdBuffer->BindIndexBuffer(m_indexBuffer.get());
			cmdBuffer->BindVertexBuffer(m_vertexBuffer.get(), 0);
			cmdBuffer->BindSampler(cr3d::ShaderStage::Pixel, Samplers::UISampleState, m_uiSamplerState.get());

			// Projection matrix. TODO: this could be cached.
			CrGPUBufferType<UIData> uiDataBuffer = cmdBuffer->AllocateConstantBuffer<UIData>();
			UIDataData* uiData = uiDataBuffer.Lock();
			{
				uiData->projection = ComputeProjectionMatrix(data);
			}
			uiDataBuffer.Unlock();
			cmdBuffer->BindConstantBuffer(&uiDataBuffer);

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
					CrScissor scissorRect = CrScissor(
						(uint32_t)(drawCmd->ClipRect.x - clipOffset.x), (uint32_t)(drawCmd->ClipRect.y - clipOffset.y),
						(uint32_t)(drawCmd->ClipRect.z - clipOffset.x), (uint32_t)(drawCmd->ClipRect.w - clipOffset.y)
					);

					if (!drawCmd->UserCallback)
					{
						// Generic rendering.
						ICrTexture* texture = (ICrTexture*)drawCmd->TextureId;
						cmdBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::UITexture, texture);
						cmdBuffer->SetScissor(scissorRect);
						cmdBuffer->DrawIndexed(drawCmd->ElemCount, 1, drawCmd->IdxOffset + acumIdxOffset, drawCmd->VtxOffset + acumVtxOffset, 0);
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
		cmdBuffer->EndRenderPass();
	}
	cmdBuffer->EndDebugEvent();
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

void CrImGuiRenderer::UpdateBuffers(ImDrawData* data)
{
	// TODO: I don't think the reseting the buffer is safe? Its deleted inline.

	// Check index buffer size. By default indices are unsigned shorts (ImDrawIdx):
	uint32_t currentIndexCount = data->TotalIdxCount;
	if (!m_indexBuffer || currentIndexCount > m_currentMaxIndexCount)
	{
		m_currentMaxIndexCount = currentIndexCount * 2;
		m_indexBuffer = ICrRenderSystem::GetRenderDevice()->CreateIndexBuffer(cr3d::DataFormat::R16_Uint, m_currentMaxIndexCount);
	}

	// Check vertex buffer size:
	uint32_t currentVertexCount = data->TotalVtxCount;
	if (!m_vertexBuffer || currentVertexCount > m_currentMaxVertexCount)
	{
		m_currentMaxVertexCount = currentVertexCount * 2;
		m_vertexBuffer = ICrRenderSystem::GetRenderDevice()->CreateVertexBuffer<UIVertex>(m_currentMaxVertexCount);
	}

	// Update contents:
	ImDrawIdx* pIdx = (ImDrawIdx*)m_indexBuffer->Lock();
	ImDrawVert* pVtx = (ImDrawVert*)m_vertexBuffer->Lock();
	for (int i = 0; i < data->CmdListsCount; ++i)
	{
		ImDrawList* drawList = data->CmdLists[i];

		memcpy(pIdx, drawList->IdxBuffer.Data, drawList->IdxBuffer.Size * sizeof(ImDrawIdx));
		memcpy(pVtx, drawList->VtxBuffer.Data, drawList->VtxBuffer.Size * sizeof(ImDrawVert));

		pIdx += drawList->IdxBuffer.Size;
		pVtx += drawList->VtxBuffer.Size;
	}
	m_indexBuffer->Unlock();
	m_vertexBuffer->Unlock();
}