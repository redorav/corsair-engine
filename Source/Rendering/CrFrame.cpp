#include "CrRendering_pch.h"

#include "CrFrame.h"

#include "Core/CrPlatform.h"

#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/ICrSampler.h"
#include "Rendering/ICrSwapchain.h"
#include "Rendering/ICrShaderManager.h"
#include "Rendering/ICrShader.h"
#include "Rendering/ICrPipelineStateManager.h"
#include "Rendering/ICrTexture.h"
#include "Rendering/ICrCommandBuffer.h"
#include "Rendering/ICrCommandQueue.h"
#include "Rendering/CrGPUBuffer.h"
#include "Rendering/CrRenderPassDescriptor.h"

#include "Rendering/CrCamera.h"
#include "Rendering/CrRenderModel.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/CrMesh.h"

#include "Input/CrInputManager.h"

#include "Core/CrFrameTime.h"

#include "CrResourceManager.h"

#include "Rendering/UI/CrImGuiRenderer.h"
#include "imgui.h"

struct SimpleVertex
{
	CrVertexElement<half, cr3d::DataFormat::RGBA16_Float> position;
	CrVertexElement<uint8_t, cr3d::DataFormat::RGBA8_Unorm> normal;
	CrVertexElement<uint8_t, cr3d::DataFormat::RGBA8_Unorm> tangent;
	CrVertexElement<half, cr3d::DataFormat::RG16_Float> uv;

	static CrVertexDescriptor GetVertexDescriptor()
	{
		return { decltype(position)::GetFormat(), decltype(normal)::GetFormat(), decltype(tangent)::GetFormat(), decltype(uv)::GetFormat() };
	}
};

static CrCamera camera;
static Camera cameraConstantData;

void CrFrame::Init(void* platformHandle, void* platformWindow, uint32_t width, uint32_t height)
{
	m_platformHandle = platformHandle;
	m_platformWindow = platformWindow;

	m_width = width;
	m_height = height;

	CrRenderDeviceSharedHandle renderDevice = ICrRenderSystem::GetRenderDevice();

	m_renderModel = CrResourceManager::LoadModel("nyra/nyra_pose_mod.fbx");
	//m_renderModel = CrResourceManager::LoadModel("jaina/storm_hero_jaina.fbx");

	CrString SHADER_PATH = IN_SRC_PATH;
	SHADER_PATH = SHADER_PATH + "Rendering/Shaders/";

	CrSamplerDescriptor descriptor;
	m_linearClampSamplerHandle = renderDevice->CreateSampler(descriptor);

	descriptor.addressModeU = cr3d::AddressMode::Wrap;
	descriptor.addressModeV = cr3d::AddressMode::Wrap;
	descriptor.addressModeW = cr3d::AddressMode::Wrap;
	m_linearWrapSamplerHandle = renderDevice->CreateSampler(descriptor);

	CrTextureDescriptor rwTextureParams;
	rwTextureParams.width = 64;
	rwTextureParams.height = 64;
	rwTextureParams.format = cr3d::DataFormat::RGBA16_Unorm;
	rwTextureParams.usage = cr3d::TextureUsage::UnorderedAccess;
	rwTextureParams.name = "Colors RW Texture";
	m_colorsRWTexture = renderDevice->CreateTexture(rwTextureParams);

	m_colorsRWDataBuffer = renderDevice->CreateDataBuffer(cr3d::BufferAccess::GPUWrite, cr3d::DataFormat::RGBA8_Unorm, 128);

	RecreateSwapchainAndDepth();
	
	CrBytecodeLoadDescriptor bytecodeLoadInfo;
	CrBytecodeLoadDescriptor computeBytecodeLoadInfo;

#define USE_HLSL

#if defined(USE_HLSL)

	bytecodeLoadInfo.AddBytecodeDescriptor(CrShaderBytecodeDescriptor(CrPath((SHADER_PATH + "triangle.hlsl").c_str()), 
		"BasicVS", cr3d::ShaderStage::Vertex, cr3d::ShaderCodeFormat::SourceHLSL, cr3d::GraphicsApi::Vulkan, cr::Platform::Windows));

	bytecodeLoadInfo.AddBytecodeDescriptor(CrShaderBytecodeDescriptor(CrPath((SHADER_PATH + "triangle.hlsl").c_str()), 
		"BasicPS", cr3d::ShaderStage::Pixel, cr3d::ShaderCodeFormat::SourceHLSL, cr3d::GraphicsApi::Vulkan, cr::Platform::Windows));

	computeBytecodeLoadInfo.AddBytecodeDescriptor(CrShaderBytecodeDescriptor(CrPath((SHADER_PATH + "compute.hlsl").c_str()),
		"MainCS", cr3d::ShaderStage::Compute, cr3d::ShaderCodeFormat::SourceHLSL, cr3d::GraphicsApi::Vulkan, cr::Platform::Windows));

#else

	bytecodeLoadInfo.AddBytecodeDescriptor(CrShaderBytecodeDescriptor(CrPath((SHADER_PATH + "triangle.vert.spv").c_str()),
		"BasicVS", cr3d::ShaderStage::Vertex, cr3d::ShaderCodeFormat::Binary, cr3d::GraphicsApi::Vulkan, cr::Platform::Windows));

	bytecodeLoadInfo.AddBytecodeDescriptor(CrShaderBytecodeDescriptor(CrPath((SHADER_PATH + "triangle.frag.spv").c_str()),
		"BasicPS", cr3d::ShaderStage::Vertex, cr3d::ShaderCodeFormat::Binary, cr3d::GraphicsApi::Vulkan, cr::Platform::Windows));

#endif

	// TODO Move block to rendering subsystem initialization function
	{
		ICrShaderManager::Get()->Init(renderDevice.get());
		ICrPipelineStateManager::Get()->Init(renderDevice.get());
	}

	CrGraphicsShaderHandle graphicsShader = ICrShaderManager::Get()->LoadGraphicsShader(bytecodeLoadInfo);

	CrComputeShaderHandle computeShader = ICrShaderManager::Get()->LoadComputeShader(computeBytecodeLoadInfo);

	CrGraphicsPipelineDescriptor graphicsPipelineDescriptor;
	graphicsPipelineDescriptor.renderTargets.colorFormats[0] = m_swapchain->GetFormat();
	graphicsPipelineDescriptor.renderTargets.depthFormat = m_depthStencilTexture->GetFormat();
	graphicsPipelineDescriptor.renderTargets.sampleCount = cr3d::SampleCount::S1;

	graphicsPipelineDescriptor.Hash();

	// TODO Reminder for next time:
	// 1) Pass in psoDescriptor, vertexInputState (need to encapsulate) and loaded/compiled graphics shader to GetGraphicsPipeline
	// 2) Create hash for all three and combine
	// 3) Do a lookup. If not in table, call CreateGraphicsPipeline with all three again
	// 4) After creation, put in table for next time

	m_pipelineTriangleState = ICrPipelineStateManager::Get()->GetGraphicsPipeline(graphicsPipelineDescriptor, graphicsShader, SimpleVertex::GetVertexDescriptor());

	// Test caching
	m_pipelineTriangleState = ICrPipelineStateManager::Get()->GetGraphicsPipeline(graphicsPipelineDescriptor, graphicsShader, SimpleVertex::GetVertexDescriptor());

	CrComputePipelineDescriptor computePipelineDescriptor;

	m_computePipelineState = ICrPipelineStateManager::Get()->GetComputePipeline(computePipelineDescriptor, computeShader);

	graphicsPipelineDescriptor.primitiveTopology = cr3d::PrimitiveTopology::LineList;
	graphicsPipelineDescriptor.Hash();
	m_pipelineLineState = ICrPipelineStateManager::Get()->GetGraphicsPipeline(graphicsPipelineDescriptor, graphicsShader, SimpleVertex::GetVertexDescriptor());

	uint8_t whiteTextureInitialData[4 * 4 * 4];
	memset(whiteTextureInitialData, 0xff, sizeof(whiteTextureInitialData));

	CrTextureDescriptor whiteTextureParams;
	whiteTextureParams.width = 4;
	whiteTextureParams.height = 4;
	whiteTextureParams.initialData = whiteTextureInitialData;
	whiteTextureParams.initialDataSize = sizeof(whiteTextureInitialData);
	m_defaultWhiteTexture = renderDevice->CreateTexture(whiteTextureParams);

	m_rwStructuredBuffer = renderDevice->CreateStructuredBuffer<ExampleRWStructuredBufferCompute>(cr3d::BufferAccess::GPUWrite, 32);

	m_structuredBuffer = renderDevice->CreateStructuredBuffer<ExampleStructuredBufferCompute>(cr3d::BufferAccess::CPUWrite, 32);

	// Initialize ImGui renderer
	CrImGuiRendererInitParams imguiInitParams = {};
	imguiInitParams.m_swapchainFormat = m_swapchain->GetFormat();
	imguiInitParams.m_sampleCount = cr3d::SampleCount::S1;
	CrImGuiRenderer::GetImGuiRenderer()->Initialize(imguiInitParams);
}

void CrFrame::Process()
{
	CrRenderDeviceSharedHandle renderDevice = ICrRenderSystem::GetRenderDevice();
	const CrCommandQueueSharedHandle& mainCommandQueue = renderDevice->GetMainCommandQueue();

	CrSwapchainResult swapchainResult = m_swapchain->AcquireNextImage(UINT64_MAX);

	if (swapchainResult == CrSwapchainResult::Invalid)
	{
		RecreateSwapchainAndDepth();
		swapchainResult = m_swapchain->AcquireNextImage(UINT64_MAX);
	}

	ICrCommandBuffer* drawCommandBuffer = m_drawCmdBuffers[m_swapchain->GetCurrentFrameIndex()].get();

	CrImGuiRenderer::GetImGuiRenderer()->NewFrame(m_swapchain->GetWidth(), m_swapchain->GetHeight());
	DrawDebugUI();

	{
		drawCommandBuffer->Begin();
		drawCommandBuffer->SetViewport(CrViewport(0.0f, 0.0f, (float)m_swapchain->GetWidth(), (float)m_swapchain->GetHeight()));
		drawCommandBuffer->SetScissor(CrScissor(0, 0, m_swapchain->GetWidth(), m_swapchain->GetHeight()));

		CrRenderPassDescriptor renderPassDescriptor;
		{
			CrRenderTargetDescriptor swapchainAttachment;
			swapchainAttachment.texture = m_swapchain->GetTexture(m_swapchain->GetCurrentFrameIndex()).get();
			swapchainAttachment.clearColor = float4(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f);
			swapchainAttachment.loadOp = CrRenderTargetLoadOp::Clear;
			swapchainAttachment.initialState = cr3d::TextureState::Undefined;
			swapchainAttachment.finalState = cr3d::TextureState::Present;

			CrRenderTargetDescriptor depthAttachment;
			depthAttachment.texture = m_depthStencilTexture.get();
			depthAttachment.loadOp = CrRenderTargetLoadOp::Clear;
			depthAttachment.initialState = cr3d::TextureState::Undefined;
			depthAttachment.finalState = cr3d::TextureState::DepthStencilWrite;

			renderPassDescriptor.color.push_back(swapchainAttachment);
			renderPassDescriptor.depth = depthAttachment;
		}

		drawCommandBuffer->BeginDebugEvent("RenderPass 1", float4(1.0f, 0.0, 1.0f, 1.0f));
		{
			drawCommandBuffer->BeginRenderPass(renderPassDescriptor);
			{
				drawCommandBuffer->BindGraphicsPipelineState(m_pipelineTriangleState.get());
	
				UpdateCamera();

				drawCommandBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::DiffuseTexture0, m_defaultWhiteTexture.get());
				drawCommandBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::NormalTexture0, m_defaultWhiteTexture.get());
				drawCommandBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::SpecularTexture0, m_defaultWhiteTexture.get());

				CrGPUBufferType<Color> colorBuffer = drawCommandBuffer->AllocateConstantBuffer<Color>();
				Color* theColorData2 = colorBuffer.Lock();
				{
					theColorData2->color = float4(1.0f, 1.0f, 1.0f, 1.0f);
					theColorData2->tint2 = float4(1.0f, 1.0f, 1.0f, 1.0f);
				}
				colorBuffer.Unlock();
				drawCommandBuffer->BindConstantBuffer(&colorBuffer);
	
				CrGPUBufferType<DynamicLight> dynamicLightBuffer = drawCommandBuffer->AllocateConstantBuffer<DynamicLight>();
				DynamicLight* dynamicLightBufferData = dynamicLightBuffer.Lock();
				{
					dynamicLightBufferData->positionRadius = float4(1.0f, 1.0f, 1.0f, 0.0f);
					dynamicLightBufferData->color = float4(1.0f, 0.25f, 0.25f, 0.0f);
				}
				dynamicLightBuffer.Unlock();
				drawCommandBuffer->BindConstantBuffer(&dynamicLightBuffer);
	
				CrGPUBufferType<Camera> cameraDataBuffer = drawCommandBuffer->AllocateConstantBuffer<Camera>();
				Camera* cameraData2 = cameraDataBuffer.Lock();
				{
					*cameraData2 = cameraConstantData;
				}
				cameraDataBuffer.Unlock();
				drawCommandBuffer->BindConstantBuffer(&cameraDataBuffer);
	
				drawCommandBuffer->BindSampler(cr3d::ShaderStage::Pixel, Samplers::AllLinearClampSampler, m_linearClampSamplerHandle.get());
				drawCommandBuffer->BindSampler(cr3d::ShaderStage::Pixel, Samplers::AllLinearWrapSampler, m_linearWrapSamplerHandle.get());
	
				for (uint32_t m = 0; m < m_renderModel->m_renderMeshes.size(); ++m)
				{
					const CrRenderMeshSharedHandle& renderMesh = m_renderModel->m_renderMeshes[m];
					uint32_t materialIndex = (*m_renderModel->m_materialMap.find(renderMesh.get())).second;
					const CrMaterialSharedHandle& material = m_renderModel->m_materials[materialIndex];
	
					for (uint32_t t = 0; t < material->m_textures.size(); ++t)
					{
						CrMaterial::TextureBinding binding = material->m_textures[t];
						drawCommandBuffer->BindTexture(cr3d::ShaderStage::Pixel, binding.semantic, binding.texture.get());
					}
	
					drawCommandBuffer->BindVertexBuffer(renderMesh->m_vertexBuffer.get(), 0);
					drawCommandBuffer->BindIndexBuffer(renderMesh->m_indexBuffer.get());
					drawCommandBuffer->DrawIndexed(renderMesh->m_indexBuffer->GetNumElements(), 1, 0, 0, 0);
				}
			}
			drawCommandBuffer->EndRenderPass();
		}
		drawCommandBuffer->EndDebugEvent();

		drawCommandBuffer->BeginDebugEvent("Compute Shader 1", float4(0.0f, 0.0, 1.0f, 1.0f));
		{
			drawCommandBuffer->BindComputePipelineState(m_computePipelineState.get());

			drawCommandBuffer->BindRWStorageBuffer(cr3d::ShaderStage::Compute, RWStorageBuffers::ExampleRWStructuredBufferCompute, m_rwStructuredBuffer.get());
		
			drawCommandBuffer->BindStorageBuffer(cr3d::ShaderStage::Compute, StorageBuffers::ExampleStructuredBufferCompute, m_structuredBuffer.get());
		
			drawCommandBuffer->BindRWDataBuffer(cr3d::ShaderStage::Compute, RWDataBuffers::ExampleDataBufferCompute, m_colorsRWDataBuffer.get());
		
			drawCommandBuffer->BindRWTexture(cr3d::ShaderStage::Compute, RWTextures::ExampleRWTextureCompute, m_colorsRWTexture.get(), 0);
		
			drawCommandBuffer->Dispatch(1, 1, 1);
		}
		drawCommandBuffer->EndDebugEvent();

		// Render ImGui
		CrImGuiRenderer::GetImGuiRenderer()->Render(drawCommandBuffer, m_swapchain->GetTexture(m_swapchain->GetCurrentFrameIndex()).get());

		drawCommandBuffer->End();
	}

	drawCommandBuffer->Submit(m_swapchain->GetCurrentPresentCompleteSemaphore().get());

	m_swapchain->Present(mainCommandQueue.get(), drawCommandBuffer->GetCompletionSemaphore().get());

	renderDevice->ProcessDeletionQueue();

	CrFrameTime::IncrementFrameCount();
}

void CrFrame::DrawDebugUI()
{
	static bool s_DemoOpen = true;
	static bool s_ShowStats = true;
	if (s_DemoOpen)
	{
		ImGui::ShowDemoWindow(&s_DemoOpen);
	}

	if (s_ShowStats)
	{
		if (ImGui::Begin("Stats", &s_ShowStats)) {

			CrTime delta = CrFrameTime::GetFrameDelta();
			ImGui::Text("Frame: %.2f ms", delta.AsMilliseconds());
			ImGui::Text("FPS: %.2f", delta.AsFPS());

			ImGui::End();
		}

	}
}

void CrFrame::UpdateCamera()
{
	camera.SetupPerspective((float)m_swapchain->GetWidth(), (float)m_swapchain->GetHeight(), 1.0f, 1000.0f);

	float3 currentLookAt = camera.GetLookatVector();
	float3 currentRight = camera.GetRightVector();

	const MouseState& mouseState = CrInput.GetMouseState();
	const KeyboardState& keyboard = CrInput.GetKeyboardState();
	const GamepadState& gamepadState = CrInput.GetGamepadState(0);

	float frameDelta = (float)CrFrameTime::GetFrameDelta().AsSeconds();

	float translationSpeed = 5.0f;

	if (keyboard.keyPressed[KeyboardKey::LeftShift])
	{
		translationSpeed *= 3.0f;
	}

	// TODO Hack to get a bit of movement on the camera
	if (keyboard.keyPressed[KeyboardKey::A] || gamepadState.axes[GamepadAxis::LeftX] < 0.0f)
	{
		camera.Translate(currentRight * -translationSpeed * frameDelta);
	}
	
	if (keyboard.keyPressed[KeyboardKey::D] || gamepadState.axes[GamepadAxis::LeftX] > 0.0f)
	{
		camera.Translate(currentRight * translationSpeed * frameDelta);
	}
	
	if (keyboard.keyPressed[KeyboardKey::W] || gamepadState.axes[GamepadAxis::LeftY] > 0.0f)
	{
		camera.Translate(currentLookAt * translationSpeed * frameDelta);
	}
	
	if (keyboard.keyPressed[KeyboardKey::S] || gamepadState.axes[GamepadAxis::LeftY] < 0.0f)
	{
		camera.Translate(currentLookAt * -translationSpeed * frameDelta);
	}
	
	if (keyboard.keyPressed[KeyboardKey::Q] || gamepadState.axes[GamepadAxis::LeftTrigger] > 0.0f)
	{
		camera.Translate(float3(0.0f, -translationSpeed, 0.0f) * frameDelta);
	}
	
	if (keyboard.keyPressed[KeyboardKey::E] || gamepadState.axes[GamepadAxis::RightTrigger] > 0.0f)
	{
		camera.Translate(float3(0.0f, translationSpeed, 0.0f) * frameDelta);
	}

	if (gamepadState.axes[GamepadAxis::RightX] > 0.0f)
	{
		//CrLogWarning("Moving right");
		camera.Rotate(float3(0.0f, 2.0f, 0.0f) * frameDelta);
		//camera.RotateAround(float3::zero(), float3(0.0f, 1.0f, 0.0f), 0.1f);
		//camera.LookAt(float3::zero(), float3(0, 1, 0));
	}

	if (gamepadState.axes[GamepadAxis::RightX] < 0.0f)
	{
		//CrLogWarning("Moving left");
		camera.Rotate(float3(0.0f, -2.0f, 0.0f) * frameDelta);
		//camera.RotateAround(float3::zero(), float3(0.0f, 1.0f, 0.0f), -0.1f);
		//camera.LookAt(float3::zero(), float3(0, 1, 0));
	}

	if (mouseState.buttonPressed[MouseButton::Right])
	{
		camera.Rotate(float3(mouseState.relativePosition.y, mouseState.relativePosition.x, 0.0f) * frameDelta);
	}

	camera.Update();

	cameraConstantData.world2View = camera.GetWorld2ViewMatrix();
	cameraConstantData.view2Projection = camera.GetView2ProjectionMatrix();
}

void CrFrame::RecreateSwapchainAndDepth()
{
	CrRenderDeviceSharedHandle renderDevice = ICrRenderSystem::GetRenderDevice();
	const CrCommandQueueSharedHandle& mainCommandQueue = renderDevice->GetMainCommandQueue();

	// Ensure all operations on the device have been finished before destroying resources
	renderDevice->WaitIdle();

	// 1. Destroy the old swapchain before creating the new one. Otherwise the API will fail trying to create a resource
	// that becomes available after (once the pointer assignment happens and the resource is destroyed). Right after, create
	// the new swapchain
	m_swapchain = nullptr;

	CrSwapchainDescriptor swapchainDescriptor = {};
	swapchainDescriptor.platformWindow = m_platformWindow;
	swapchainDescriptor.platformHandle = m_platformHandle;
	swapchainDescriptor.requestedWidth = m_width;
	swapchainDescriptor.requestedHeight = m_height;
	swapchainDescriptor.format = cr3d::DataFormat::BGRA8_Unorm;
	swapchainDescriptor.requestedBufferCount = 3;
	m_swapchain = renderDevice->CreateSwapchain(swapchainDescriptor);

	// 2. Recreate depth stencil texture

	CrTextureDescriptor depthTexParams;
	depthTexParams.width = m_swapchain->GetWidth();
	depthTexParams.height = m_swapchain->GetHeight();
	depthTexParams.format = cr3d::DataFormat::D32_Float_S8_Uint;
	depthTexParams.usage = cr3d::TextureUsage::DepthStencil;

	m_depthStencilTexture = renderDevice->CreateTexture(depthTexParams); // Create the depth buffer

	// 4. Recreate command buffers
	m_drawCmdBuffers.resize(m_swapchain->GetImageCount());
	for (uint32_t i = 0; i < m_drawCmdBuffers.size(); ++i)
	{
		m_drawCmdBuffers[i] = mainCommandQueue->CreateCommandBuffer();
	}

	// Make sure all of this work is finished
	renderDevice->WaitIdle();
}
