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
#include "Rendering/CrRenderMesh.h"
#include "Rendering/CrMaterialCompiler.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/CrShaderSources.h"
#include "Rendering/CrVisibility.h"

#include "Rendering/CrRenderWorld.h"
#include "Rendering/CrRenderModelInstance.h"
#include "Rendering/CrCPUStackAllocator.h"

#include "Rendering/CrRenderGraph.h"

#include "Input/CrInputManager.h"

#include "Core/CrPlatform.h"
#include "Core/CrFrameTime.h"
#include "Core/CrGlobalPaths.h"

#include "CrResourceManager.h"

#include "imgui.h"

#include "Core/FileSystem/CrPath.h"

#include "Rendering/CrVertexDescriptor.h"
#include "Rendering/CrCommonVertexLayouts.h"

// TODO Put somewhere else
bool HashingAssert()
{
	CrGraphicsPipelineDescriptor defaultDescriptor;
	CrAssertMsg(CrHash(&defaultDescriptor) == CrHash(12342096532583984399), "Failed to hash known pipeline descriptor!");
	return true;
}

// Batches render packets
struct CrRenderPacketBatcher
{
	CrRenderPacketBatcher(ICrCommandBuffer* commandBuffer)
	{
		m_commandBuffer = commandBuffer;
	}

	void AddRenderPacket(const CrRenderPacket& renderPacket)
	{
		bool stateMismatch =
			renderPacket.pipeline != m_pipeline ||
			renderPacket.material != m_material ||
			renderPacket.renderMesh != m_renderMesh;

		bool noMoreSpace = (m_numInstances + renderPacket.numInstances) > m_matrices.size();

		if (stateMismatch || noMoreSpace)
		{
			ExecuteBatch();
			m_batchStarted = false;
		}

		if (!m_batchStarted)
		{
			// Begin batch
			m_numInstances = 0;
			m_material = renderPacket.material;
			m_renderMesh = renderPacket.renderMesh;
			m_pipeline = renderPacket.pipeline;
			m_batchStarted = true;
		}

		// Accumulate and copy matrix pointers over
		for (uint32_t i = 0; i < renderPacket.numInstances; ++i)
		{
			m_matrices[m_numInstances + i] = &renderPacket.transforms[i];
		}

		m_numInstances += renderPacket.numInstances;
	}

	void ExecuteBatch()
	{
		if (m_numInstances > 0)
		{
			// Allocate constant buffer with all transforms and copy them across
			CrGPUBufferType<Instance> transformBuffer = m_commandBuffer->AllocateConstantBuffer<Instance>(sizeof(Instance::local2World[0]) * m_numInstances);
			cr3d::float4x4* transforms = (cr3d::float4x4*)transformBuffer.Lock();
			{
				for (uint32_t i = 0; i < m_numInstances; ++i)
				{
					transforms[i] = *m_matrices[i];
				}
			}
			transformBuffer.Unlock();
			m_commandBuffer->BindConstantBuffer(&transformBuffer);

			m_commandBuffer->BindGraphicsPipelineState(m_pipeline);

			for (uint32_t t = 0; t < m_material->m_textures.size(); ++t)
			{
				CrMaterial::TextureBinding binding = m_material->m_textures[t];
				m_commandBuffer->BindTexture(cr3d::ShaderStage::Pixel, binding.semantic, binding.texture.get());
			}

			for (uint32_t vbIndex = 0; vbIndex < m_renderMesh->GetVertexBufferCount(); ++vbIndex)
			{
				m_commandBuffer->BindVertexBuffer(m_renderMesh->GetVertexBuffer(vbIndex).get(), vbIndex);
			}

			m_commandBuffer->BindIndexBuffer(m_renderMesh->GetIndexBuffer().get());

			m_commandBuffer->DrawIndexed(m_renderMesh->GetIndexBuffer()->GetNumElements(), m_numInstances, 0, 0, 0);
		}
	}

	bool m_batchStarted = false;

	uint32_t m_numInstances = 0;
	const CrMaterial* m_material = nullptr;
	const CrRenderMesh* m_renderMesh = nullptr;
	const ICrGraphicsPipeline* m_pipeline = nullptr;

	ICrCommandBuffer* m_commandBuffer = nullptr;

	// Has to match the maximum number of matrices declared in the shader
	CrArray<float4x4*, sizeof_array(Instance::local2World)> m_matrices;
};

void CrFrame::Init(void* platformHandle, void* platformWindow, uint32_t width, uint32_t height)
{
	HashingAssert();

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
		CrPipelineStateManager::Get().Initialize(renderDevice.get());
	}

	CrRenderModelSharedHandle nyraModel = CrResourceManager::LoadModel(CrResourceManager::GetFullResourcePath("nyra/nyra_pose_mod.fbx"));
	CrRenderModelSharedHandle jainaModel = CrResourceManager::LoadModel(CrResourceManager::GetFullResourcePath("jaina/storm_hero_jaina.fbx"));
	CrRenderModelSharedHandle damagedHelmet = CrResourceManager::LoadModel(CrResourceManager::GetFullResourcePath("gltf-helmet/DamagedHelmet.gltf"));

	// Create the rendering scratch. Start with 10MB
	m_renderingStream = CrSharedPtr<CrCPUStackAllocator>(new CrCPUStackAllocator());
	m_renderingStream->Initialize(10 * 1024 * 1024);

	// Create a render world
	m_renderWorld = CrMakeShared<CrRenderWorld>();

	const uint32_t numModels = 100;

	for (uint32_t i = 0; i < numModels; ++i)
	{
		CrRenderModelInstance modelInstance = m_renderWorld->CreateModelInstance();

		float angle = 1.61803f * i;
		float radius = 300.0f * i / numModels;

		float x = radius * sinf(angle);
		float z = radius * cosf(angle);

		float4x4 transformMatrix = float4x4::translation(x, 0.0f, z);

		m_renderWorld->SetTransform(modelInstance.GetId(), transformMatrix);

		int r = rand();
		CrRenderModelSharedHandle renderModel;

		if (r < RAND_MAX / 3)
		{
			renderModel = nyraModel;
		}
		else if (r < 2 * RAND_MAX / 3)
		{
			renderModel = jainaModel;
		}
		else
		{
			renderModel = damagedHelmet;
		}

		m_renderWorld->SetRenderModel(modelInstance.GetId(), renderModel);
	}

	m_camera = CrSharedPtr<CrCamera>(new CrCamera());

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

	CrGraphicsShaderHandle lineGraphicsShader = CrShaderManager::Get().CompileGraphicsShader(basicBytecodeLoadInfo);
	CrComputeShaderHandle computeShader = CrShaderManager::Get().CompileComputeShader(computeBytecodeLoadInfo);

	CrGraphicsPipelineDescriptor basicGraphicsPipelineDescriptor;
	basicGraphicsPipelineDescriptor.renderTargets.colorFormats[0] = m_swapchain->GetFormat();
	basicGraphicsPipelineDescriptor.renderTargets.depthFormat = m_depthStencilTexture->GetFormat();

	CrComputePipelineDescriptor computePipelineDescriptor;

	m_computePipelineState = CrPipelineStateManager::Get().GetComputePipeline(computePipelineDescriptor, computeShader);

	basicGraphicsPipelineDescriptor.primitiveTopology = cr3d::PrimitiveTopology::LineList;
	m_linePipelineState = CrPipelineStateManager::Get().GetGraphicsPipeline(
		basicGraphicsPipelineDescriptor, lineGraphicsShader, SimpleVertexDescriptor);

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

	CrRenderingStatistics::Reset();

	ICrCommandBuffer* drawCommandBuffer = m_drawCmdBuffers[m_swapchain->GetCurrentFrameIndex()].get();

	CrImGuiRenderer::GetImGuiRenderer()->NewFrame(m_swapchain->GetWidth(), m_swapchain->GetHeight());

	m_mainRenderGraph.commandBuffer = drawCommandBuffer; // TODO Rework

	UpdateCamera();

	m_renderWorld->BeginRendering(m_renderingStream);

	m_renderWorld->SetCamera(m_camera);

	drawCommandBuffer->Begin();

	CrRenderGraphTextureId depthTexture;
	CrRenderGraphTextureId swapchainTexture;

	{
		CrRenderGraphTextureDescriptor depthDescriptor;
		depthDescriptor.texture = m_depthStencilTexture.get();
		depthTexture = mainRenderGraph.CreateTexture("Depth", depthDescriptor);

		CrRenderGraphTextureDescriptor swapchainDescriptor;
		swapchainDescriptor.texture = m_swapchain->GetTexture(m_swapchain->GetCurrentFrameIndex()).get();
		swapchainTexture = mainRenderGraph.CreateTexture("Swapchain", swapchainDescriptor);
	}

	m_mainRenderGraph.AddRenderPass("Render Pass 1", float4(1.0f, 0.0, 1.0f, 1.0f), CrRenderGraphPassType::Graphics,
	[=](CrRenderGraph& renderGraph)
	{
		renderGraph.AddDepthStencilTarget(depthTexture, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, 0.0f);
		renderGraph.AddRenderTarget(swapchainTexture, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, float4(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f));
	},
	[this](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
	{
		commandBuffer->SetViewport(CrViewport(0.0f, 0.0f, (float)m_swapchain->GetWidth(), (float)m_swapchain->GetHeight()));
		commandBuffer->SetScissor(CrScissor(0, 0, m_swapchain->GetWidth(), m_swapchain->GetHeight()));

		commandBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::DiffuseTexture0, m_defaultWhiteTexture.get());
		commandBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::NormalTexture0, m_defaultWhiteTexture.get());
		commandBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::SpecularTexture0, m_defaultWhiteTexture.get());

		CrGPUBufferType<Color> colorBuffer = commandBuffer->AllocateConstantBuffer<Color>();
		Color* theColorData2 = colorBuffer.Lock();
		{
			theColorData2->color = float4(1.0f, 1.0f, 1.0f, 1.0f);
			theColorData2->tint2 = float4(1.0f, 1.0f, 1.0f, 1.0f);
		}
		colorBuffer.Unlock();
		commandBuffer->BindConstantBuffer(&colorBuffer);

		CrGPUBufferType<DynamicLight> dynamicLightBuffer = commandBuffer->AllocateConstantBuffer<DynamicLight>();
		DynamicLight* dynamicLightBufferData = dynamicLightBuffer.Lock();
		{
			dynamicLightBufferData->positionRadius = float4(1.0f, 1.0f, 1.0f, 0.0f);
			dynamicLightBufferData->color = float4(1.0f, 0.25f, 0.25f, 0.0f);
		}
		dynamicLightBuffer.Unlock();
		commandBuffer->BindConstantBuffer(&dynamicLightBuffer);

		CrGPUBufferType<Camera> cameraDataBuffer = commandBuffer->AllocateConstantBuffer<Camera>();
		Camera* cameraData2 = cameraDataBuffer.Lock();
		{
			*cameraData2 = m_cameraConstantData;
		}
		cameraDataBuffer.Unlock();
		commandBuffer->BindConstantBuffer(&cameraDataBuffer);

		commandBuffer->BindSampler(cr3d::ShaderStage::Pixel, Samplers::AllLinearClampSampler, m_linearClampSamplerHandle.get());
		commandBuffer->BindSampler(cr3d::ShaderStage::Pixel, Samplers::AllLinearWrapSampler, m_linearWrapSamplerHandle.get());

		float t0 = CrFrameTime::GetFrameCount() * 0.01f;
		float x = sinf(t0);
		float z = cosf(t0);

		float4x4 transformMatrix = float4x4::translation(x, 0.0f, z);

		m_renderWorld->ComputeVisibilityAndRenderPackets();

		const CrRenderList& mainRenderList = m_renderWorld->GetMainRenderList();

		CrRenderPacketBatcher renderPacketBatcher(commandBuffer);

		mainRenderList.ForEachRenderPacket([&](const CrRenderPacket& renderPacket)
		{
			renderPacketBatcher.AddRenderPacket(renderPacket);
		});

		renderPacketBatcher.ExecuteBatch();
	});

	CrRenderGraphBufferDescriptor structuredBufferDescriptor;
	structuredBufferDescriptor.buffer = m_structuredBuffer.get();
	CrRenderGraphBufferId structuredBuffer = mainRenderGraph.CreateBuffer("Structured Buffer", structuredBufferDescriptor);

	CrRenderGraphBufferDescriptor rwStructuredBufferDescriptor;
	rwStructuredBufferDescriptor.buffer = m_rwStructuredBuffer.get();
	CrRenderGraphBufferId rwStructuredBuffer = mainRenderGraph.CreateBuffer("RW Structured Buffer", rwStructuredBufferDescriptor);

	CrRenderGraphBufferDescriptor colorsRWDataBufferDescriptor;
	colorsRWDataBufferDescriptor.buffer = m_colorsRWDataBuffer.get();
	CrRenderGraphBufferId colorsRWDataBuffer = mainRenderGraph.CreateBuffer("Colors RW Data Buffer", colorsRWDataBufferDescriptor);

	CrRenderGraphTextureDescriptor colorsRWTextureDescriptor;
	colorsRWTextureDescriptor.texture = m_colorsRWTexture.get();
	CrRenderGraphTextureId colorsRWTexture = mainRenderGraph.CreateTexture("Colors RW Texture", colorsRWTextureDescriptor);

	CrComputePipelineHandle computePipeline = m_computePipelineState;

	m_mainRenderGraph.AddRenderPass("Compute 1", float4(0.0f, 0.0, 1.0f, 1.0f), CrRenderGraphPassType::Compute,
	[&](CrRenderGraph& renderGraph)
	{
		renderGraph.AddBuffer(structuredBuffer, cr3d::ShaderStageFlags::Compute);
		renderGraph.AddRWBuffer(rwStructuredBuffer, cr3d::ShaderStageFlags::Compute);
		renderGraph.AddRWBuffer(colorsRWDataBuffer, cr3d::ShaderStageFlags::Compute);
		renderGraph.AddRWTexture(colorsRWTexture, cr3d::ShaderStageFlags::Compute, 0, 1, 0, 1);
	},
	[=](const CrRenderGraph& renderGraph, ICrCommandBuffer* commandBuffer)
	{
		commandBuffer->BindComputePipelineState(computePipeline.get());
		commandBuffer->BindStorageBuffer(cr3d::ShaderStage::Compute, StorageBuffers::ExampleStructuredBufferCompute, renderGraph.GetPhysicalBuffer(structuredBuffer));
		commandBuffer->BindRWStorageBuffer(cr3d::ShaderStage::Compute, RWStorageBuffers::ExampleRWStructuredBufferCompute, renderGraph.GetPhysicalBuffer(rwStructuredBuffer));
		commandBuffer->BindRWDataBuffer(cr3d::ShaderStage::Compute, RWDataBuffers::ExampleDataBufferCompute, renderGraph.GetPhysicalBuffer(colorsRWDataBuffer));
		commandBuffer->BindRWTexture(cr3d::ShaderStage::Compute, RWTextures::ExampleRWTextureCompute, renderGraph.GetPhysicalTexture(colorsRWTexture), 0);
		commandBuffer->Dispatch(1, 1, 1);
	});
		commandBuffer->Dispatch(1, 1, 1);
	});

	m_mainRenderGraph.AddRenderPass("Draw Debug UI", float4(), CrRenderGraphPassType::Behavior,
	[](CrRenderGraph&)
	{},
	[this](const CrRenderGraph&, ICrCommandBuffer*)
	{
		DrawDebugUI();
	});

	// Render ImGui
	CrImGuiRenderer::GetImGuiRenderer()->Render(m_mainRenderGraph, swapchainTexture);

	// Create a render pass that transitions the frame. We need to give the render graph
	// visibility over what's going to happen with the texture, but not necessarily
	// execute the behavior inside as we may want to do further work before we end the 
	// command buffer
	m_mainRenderGraph.AddRenderPass("Present", float4(), CrRenderGraphPassType::Behavior,
	[=](CrRenderGraph& renderGraph)
	{
		renderGraph.AddSwapchain(swapchainTexture);
	},
	[this, mainCommandQueue](const CrRenderGraph&, ICrCommandBuffer* /*commandBuffer*/)
	{
		
	});

	m_mainRenderGraph.Execute();


	drawCommandBuffer->End();

	drawCommandBuffer->Submit(m_swapchain->GetCurrentPresentCompleteSemaphore().get());

	m_swapchain->Present(mainCommandQueue.get(), drawCommandBuffer->GetCompletionSemaphore().get());

	m_renderWorld->EndRendering();

	m_renderingStream->Reset();

	m_mainRenderGraph.Reset();

	renderDevice->ProcessDeletionQueue();

	CrFrameTime::IncrementFrameCount();
}

struct CrSizeUnit
{
	uint64_t smallUnit;
	const char* unit;
};

CrSizeUnit GetSizeUnit(uint64_t bytes)
{
	if (bytes > 1024 * 1024 * 1024)
	{
		return { bytes / (1024 * 1024 * 1024), "GB" };
	}
	else if (bytes > 1024 * 1024)
	{
		return { bytes / (1024 * 1024), "MB" };
	}
	else if (bytes > 1024)
	{
		return { bytes / 1024, "KB" };
	}
	else
	{
		return { bytes, "bytes" };
	}
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
		if (ImGui::Begin("Statistics", &s_ShowStats))
		{
			CrTime delta = CrFrameTime::GetFrameDelta();
			CrTime averageDelta = CrFrameTime::GetFrameDeltaAverage(); 

			const CrRenderDeviceProperties& properties = ICrRenderSystem::GetRenderDevice()->GetProperties();
			CrSizeUnit sizeUnit = GetSizeUnit(properties.gpuMemoryBytes);

			ImGui::Text("Frame: %i", CrFrameTime::GetFrameCount());
			ImGui::Text("GPU: %s (%llu%s)", properties.description.c_str(), sizeUnit.smallUnit, sizeUnit.unit);
			ImGui::Text("Delta: [Instant] %.2f ms [Average] %.2fms", delta.AsMilliseconds(), averageDelta.AsMilliseconds());
			ImGui::Text("FPS: [Instant] %.2f fps [Average] %.2f fps", delta.AsFPS(), averageDelta.AsFPS());
			ImGui::Text("Drawcalls: %i Vertices: %i", CrRenderingStatistics::GetDrawcallCount(), CrRenderingStatistics::GetVertexCount());

			ImGui::End();
		}
	}
}

void CrFrame::UpdateCamera()
{
	m_camera->SetupPerspective((float)m_swapchain->GetWidth(), (float)m_swapchain->GetHeight(), 1.0f, 1000.0f);

	float3 currentLookAt = m_camera->GetLookatVector();
	float3 currentRight = m_camera->GetRightVector();

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
		m_camera->Translate(currentRight * -translationSpeed * frameDelta);
	}
	
	if (keyboard.keyPressed[KeyboardKey::D] || gamepadState.axes[GamepadAxis::LeftX] > 0.0f)
	{
		m_camera->Translate(currentRight * translationSpeed * frameDelta);
	}
	
	if (keyboard.keyPressed[KeyboardKey::W] || gamepadState.axes[GamepadAxis::LeftY] > 0.0f)
	{
		m_camera->Translate(currentLookAt * translationSpeed * frameDelta);
	}
	
	if (keyboard.keyPressed[KeyboardKey::S] || gamepadState.axes[GamepadAxis::LeftY] < 0.0f)
	{
		m_camera->Translate(currentLookAt * -translationSpeed * frameDelta);
	}
	
	if (keyboard.keyPressed[KeyboardKey::Q] || gamepadState.axes[GamepadAxis::LeftTrigger] > 0.0f)
	{
		m_camera->Translate(float3(0.0f, -translationSpeed, 0.0f) * frameDelta);
	}
	
	if (keyboard.keyPressed[KeyboardKey::E] || gamepadState.axes[GamepadAxis::RightTrigger] > 0.0f)
	{
		m_camera->Translate(float3(0.0f, translationSpeed, 0.0f) * frameDelta);
	}

	if (gamepadState.axes[GamepadAxis::RightX] > 0.0f)
	{
		//CrLogWarning("Moving right");
		m_camera->Rotate(float3(0.0f, 2.0f, 0.0f) * frameDelta);
		//camera.RotateAround(float3::zero(), float3(0.0f, 1.0f, 0.0f), 0.1f);
		//camera.LookAt(float3::zero(), float3(0, 1, 0));
	}

	if (gamepadState.axes[GamepadAxis::RightX] < 0.0f)
	{
		//CrLogWarning("Moving left");
		m_camera->Rotate(float3(0.0f, -2.0f, 0.0f) * frameDelta);
		//camera.RotateAround(float3::zero(), float3(0.0f, 1.0f, 0.0f), -0.1f);
		//camera.LookAt(float3::zero(), float3(0, 1, 0));
	}

	if (mouseState.buttonPressed[MouseButton::Right])
	{
		m_camera->Rotate(float3(mouseState.relativePosition.y, mouseState.relativePosition.x, 0.0f) * frameDelta);
	}

	m_camera->Update();

	m_cameraConstantData.world2View = m_camera->GetWorld2ViewMatrix();
	m_cameraConstantData.view2Projection = m_camera->GetView2ProjectionMatrix();
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

CrFrame::CrFrame()
{

}

CrFrame::~CrFrame()
{
}