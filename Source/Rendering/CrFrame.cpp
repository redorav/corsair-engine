#include "CrRendering_pch.h"

#include "CrFrame.h"

#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/ICrSampler.h"
#include "Rendering/ICrSwapchain.h"
#include "Rendering/CrShaderManager.h"
#include "Rendering/ICrShader.h"
#include "Rendering/CrPipelineStateManager.h"
#include "Rendering/ICrTexture.h"
#include "Rendering/ICrCommandBuffer.h"
#include "Rendering/ICrCommandQueue.h"
#include "Rendering/CrGPUBuffer.h"
#include "Rendering/CrRenderPassDescriptor.h"
#include "Rendering/UI/CrImGuiRenderer.h"

#include "Rendering/CrCamera.h"
#include "Rendering/CrRenderModel.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/CrMesh.h"
#include "Rendering/CrMaterialCompiler.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/CrShaderSources.h"

#include "Input/CrInputManager.h"

#include "Core/CrPlatform.h"
#include "Core/CrFrameTime.h"
#include "Core/CrGlobalPaths.h"

#include "CrResourceManager.h"

#include "imgui.h"

#include "Core/FileSystem/CrPath.h"

#include "Rendering/CrVertexDescriptor.h"
#include "Rendering/CrCommonVertexLayouts.h"

static CrSharedPtr<CrCamera> camera;
static Camera cameraConstantData;

void CrFrame::Init(void* platformHandle, void* platformWindow, uint32_t width, uint32_t height)
{
	m_platformHandle = platformHandle;
	m_platformWindow = platformWindow;

	m_width = width;
	m_height = height;

	CrRenderDeviceSharedHandle renderDevice = ICrRenderSystem::GetRenderDevice();

	// TODO Move block to rendering subsystem initialization function
	{
		CrShaderSources::Get().Initialize();
		CrShaderManager::Get().Initialize(renderDevice.get());
		CrMaterialCompiler::Get().Initialize();
		CrPipelineStateManager::Get()->Init(renderDevice.get());
	}

	m_renderModel = CrResourceManager::LoadModel(CrResourceManager::GetFullResourcePath("nyra/nyra_pose_mod.fbx"));
	//m_renderModel = CrResourceManager::LoadModel(CrResourceManager::GetFullResourcePath("Tall Building 01/TallBuilding01.fbx"));
	//m_renderModel = CrResourceManager::LoadModel(CrResourceManager::GetFullResourcePath("Tavern/Barrel/Barrel.fbx"));
	//m_renderModel = CrResourceManager::LoadModel(CrResourceManager::GetFullResourcePath("jaina/storm_hero_jaina.fbx"));

	camera = CrSharedPtr<CrCamera>(new CrCamera());

	CrSamplerDescriptor descriptor;
	descriptor.name = "Linear Clamp Sampler";
	m_linearClampSamplerHandle = renderDevice->CreateSampler(descriptor);

	descriptor.addressModeU = cr3d::AddressMode::Wrap;
	descriptor.addressModeV = cr3d::AddressMode::Wrap;
	descriptor.addressModeW = cr3d::AddressMode::Wrap;
	descriptor.name = "Linear Wrap Sampler";
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
	
	CrShaderCompilationDescriptor basicBytecodeLoadInfo;
	CrShaderCompilationDescriptor computeBytecodeLoadInfo;

	CrString ShaderSourceDirectory = CrGlobalPaths::GetShaderSourceDirectory();

	CrShaderBytecodeCompilationDescriptor basicVSDescriptor = CrShaderBytecodeCompilationDescriptor(CrPath((ShaderSourceDirectory + "Ubershader.hlsl").c_str()),
		"UbershaderVS", cr3d::ShaderStage::Vertex, cr3d::GraphicsApi::Vulkan, cr::Platform::Windows);
	basicBytecodeLoadInfo.AddBytecodeDescriptor(basicVSDescriptor);

	CrShaderBytecodeCompilationDescriptor basicPSDescriptor = CrShaderBytecodeCompilationDescriptor(CrPath((ShaderSourceDirectory + "Ubershader.hlsl").c_str()),
		"UbershaderPS", cr3d::ShaderStage::Pixel, cr3d::GraphicsApi::Vulkan, cr::Platform::Windows);

	basicBytecodeLoadInfo.AddBytecodeDescriptor(basicPSDescriptor);

	computeBytecodeLoadInfo.AddBytecodeDescriptor(CrShaderBytecodeCompilationDescriptor(CrPath((ShaderSourceDirectory + "Compute.hlsl").c_str()),
		"MainCS", cr3d::ShaderStage::Compute, cr3d::GraphicsApi::Vulkan, cr::Platform::Windows));

	CrGraphicsShaderHandle graphicsShader = CrShaderManager::Get().CompileGraphicsShader(basicBytecodeLoadInfo);
	CrComputeShaderHandle computeShader = CrShaderManager::Get().CompileComputeShader(computeBytecodeLoadInfo);

	CrGraphicsPipelineDescriptor basicGraphicsPipelineDescriptor;
	basicGraphicsPipelineDescriptor.renderTargets.colorFormats[0] = m_swapchain->GetFormat();
	basicGraphicsPipelineDescriptor.renderTargets.depthFormat = m_depthStencilTexture->GetFormat();

	// TODO Reminder for next time:
	// 1) Pass in psoDescriptor, vertexInputState (need to encapsulate) and loaded/compiled graphics shader to GetGraphicsPipeline
	// 2) Create hash for all three and combine
	// 3) Do a lookup. If not in table, call CreateGraphicsPipeline with all three again
	// 4) After creation, put in table for next time

	m_basicPipelineState = CrPipelineStateManager::Get()->GetGraphicsPipeline(
		basicGraphicsPipelineDescriptor, graphicsShader, ComplexVertexDescriptor);

	// Test caching
	m_basicPipelineState = CrPipelineStateManager::Get()->GetGraphicsPipeline(
		basicGraphicsPipelineDescriptor, graphicsShader, ComplexVertexDescriptor);

	CrComputePipelineDescriptor computePipelineDescriptor;

	m_computePipelineState = CrPipelineStateManager::Get()->GetComputePipeline(computePipelineDescriptor, computeShader);

	basicGraphicsPipelineDescriptor.primitiveTopology = cr3d::PrimitiveTopology::LineList;
	m_linePipelineState = CrPipelineStateManager::Get()->GetGraphicsPipeline(
		basicGraphicsPipelineDescriptor, graphicsShader, SimpleVertexDescriptor);

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
	CrImGuiRenderer::GetImGuiRenderer()->Initialize(imguiInitParams);
}

void CrFrame::Process()
{
	const CrRenderDeviceSharedHandle& renderDevice = ICrRenderSystem::GetRenderDevice();
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

	UpdateCamera();

	{
		drawCommandBuffer->Begin();
		drawCommandBuffer->SetViewport(CrViewport(0.0f, 0.0f, (float)m_swapchain->GetWidth(), (float)m_swapchain->GetHeight()));
		drawCommandBuffer->SetScissor(CrScissor(0, 0, m_swapchain->GetWidth(), m_swapchain->GetHeight()));

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
				float t0 = CrFrameTime::GetFrameCount() * 0.01f;
				float x = sinf(t0);
				float z = cosf(t0);

				float4x4 transformMatrix = float4x4::translation(x, 0.0f, z);

				CrGPUBufferType<Instance> transformBuffer = drawCommandBuffer->AllocateConstantBuffer<Instance>();
				Instance* transformData = transformBuffer.Lock();
				{
					//store(transformMatrix, &transformData->local2World.m00);
					transformData->local2World[0] = transformMatrix;
				}
				transformBuffer.Unlock();
				drawCommandBuffer->BindConstantBuffer(&transformBuffer);

				for (uint32_t meshIndex = 0; meshIndex < m_renderModel->GetRenderMeshCount(); ++meshIndex)
				{
					CrPair<const CrRenderMeshSharedHandle&, const CrMaterialSharedHandle&> meshMaterial = m_renderModel->GetRenderMeshMaterial(meshIndex);
					const CrRenderMeshSharedHandle& renderMesh = meshMaterial.first;
					const CrMaterialSharedHandle& material = meshMaterial.second;

					drawCommandBuffer->BindGraphicsPipelineState(m_renderModel->GetPipeline(meshIndex, CrMaterialPipelineVariant::Transparency).get());

					for (uint32_t t = 0; t < material->m_textures.size(); ++t)
					{
						CrMaterial::TextureBinding binding = material->m_textures[t];
						drawCommandBuffer->BindTexture(cr3d::ShaderStage::Pixel, binding.semantic, binding.texture.get());
					}

					for (uint32_t vbIndex = 0; vbIndex < renderMesh->GetVertexBufferCount(); ++vbIndex)
					{
						drawCommandBuffer->BindVertexBuffer(renderMesh->GetVertexBuffer(vbIndex).get(), vbIndex);
					}

					drawCommandBuffer->BindIndexBuffer(renderMesh->GetIndexBuffer().get());
					drawCommandBuffer->DrawIndexed(renderMesh->GetIndexBuffer()->GetNumElements(), 1, 0, 0, 0);
				}
			}
			drawCommandBuffer->EndRenderPass();
		}
		drawCommandBuffer->EndDebugEvent();

		//drawCommandBuffer->BeginDebugEvent("Compute Shader 1", float4(0.0f, 0.0, 1.0f, 1.0f));
		//{
		//	drawCommandBuffer->BindComputePipelineState(m_computePipelineState.get());
		//
		//	drawCommandBuffer->BindRWStorageBuffer(cr3d::ShaderStage::Compute, RWStorageBuffers::ExampleRWStructuredBufferCompute, m_rwStructuredBuffer.get());
		//
		//	drawCommandBuffer->BindStorageBuffer(cr3d::ShaderStage::Compute, StorageBuffers::ExampleStructuredBufferCompute, m_structuredBuffer.get());
		//
		//	drawCommandBuffer->BindRWDataBuffer(cr3d::ShaderStage::Compute, RWDataBuffers::ExampleDataBufferCompute, m_colorsRWDataBuffer.get());
		//
		//	drawCommandBuffer->BindRWTexture(cr3d::ShaderStage::Compute, RWTextures::ExampleRWTextureCompute, m_colorsRWTexture.get(), 0);
		//
		//	drawCommandBuffer->Dispatch(1, 1, 1);
		//}
		//drawCommandBuffer->EndDebugEvent();

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
	camera->SetupPerspective((float)m_swapchain->GetWidth(), (float)m_swapchain->GetHeight(), 1.0f, 1000.0f);

	float3 currentLookAt = camera->GetLookatVector();
	float3 currentRight = camera->GetRightVector();

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
		camera->Translate(currentRight * -translationSpeed * frameDelta);
	}
	
	if (keyboard.keyPressed[KeyboardKey::D] || gamepadState.axes[GamepadAxis::LeftX] > 0.0f)
	{
		camera->Translate(currentRight * translationSpeed * frameDelta);
	}
	
	if (keyboard.keyPressed[KeyboardKey::W] || gamepadState.axes[GamepadAxis::LeftY] > 0.0f)
	{
		camera->Translate(currentLookAt * translationSpeed * frameDelta);
	}
	
	if (keyboard.keyPressed[KeyboardKey::S] || gamepadState.axes[GamepadAxis::LeftY] < 0.0f)
	{
		camera->Translate(currentLookAt * -translationSpeed * frameDelta);
	}
	
	if (keyboard.keyPressed[KeyboardKey::Q] || gamepadState.axes[GamepadAxis::LeftTrigger] > 0.0f)
	{
		camera->Translate(float3(0.0f, -translationSpeed, 0.0f) * frameDelta);
	}
	
	if (keyboard.keyPressed[KeyboardKey::E] || gamepadState.axes[GamepadAxis::RightTrigger] > 0.0f)
	{
		camera->Translate(float3(0.0f, translationSpeed, 0.0f) * frameDelta);
	}

	if (gamepadState.axes[GamepadAxis::RightX] > 0.0f)
	{
		//CrLogWarning("Moving right");
		camera->Rotate(float3(0.0f, 2.0f, 0.0f) * frameDelta);
		//camera.RotateAround(float3::zero(), float3(0.0f, 1.0f, 0.0f), 0.1f);
		//camera.LookAt(float3::zero(), float3(0, 1, 0));
	}

	if (gamepadState.axes[GamepadAxis::RightX] < 0.0f)
	{
		//CrLogWarning("Moving left");
		camera->Rotate(float3(0.0f, -2.0f, 0.0f) * frameDelta);
		//camera.RotateAround(float3::zero(), float3(0.0f, 1.0f, 0.0f), -0.1f);
		//camera.LookAt(float3::zero(), float3(0, 1, 0));
	}

	if (mouseState.buttonPressed[MouseButton::Right])
	{
		camera->Rotate(float3(mouseState.relativePosition.y, mouseState.relativePosition.x, 0.0f) * frameDelta);
	}

	camera->Update();

	cameraConstantData.world2View = camera->GetWorld2ViewMatrix();
	cameraConstantData.view2Projection = camera->GetView2ProjectionMatrix();
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
	depthTexParams.name = "Depth Texture D32S8";

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
