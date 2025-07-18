#include "CrFrame.h"

#include "Editor/CrEditor.h"

#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/ICrSwapchain.h"
#include "Rendering/CrShaderManager.h"
#include "Rendering/ICrShader.h"
#include "Rendering/ICrTexture.h"
#include "Rendering/ICrCommandBuffer.h"
#include "Rendering/CrGPUBuffer.h"
#include "Rendering/UI/CrImGuiRenderer.h"
#include "Rendering/CrGPUTimingQueryTracker.h"

#include "Rendering/CrCamera.h"
#include "Rendering/CrRenderModel.h"
#include "Rendering/CrRenderMesh.h"
#include "Rendering/CrMaterialCompiler.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/CrShaderSources.h"
#include "Rendering/CrVisibility.h"
#include "Rendering/CrCPUStackAllocator.h"

#include "Rendering/RenderWorld/CrRenderWorld.h"
#include "Rendering/RenderWorld/CrModelInstance.h"

#include "Rendering/CrRendererConfig.h"
#include "Rendering/CrRenderingResources.h"

#include "Rendering/CrCommonVertexLayouts.h"

#include "Core/Input/CrInputManager.h"
#include "Core/CrPlatform.h"
#include "Core/CrFrameTime.h"
#include "Core/CrGlobalPaths.h"

#include "Resource/CrResourceManager.h"

#include "GeneratedShaders/BuiltinShaders.h"

#include "Math/CrMath.h"
#include "Math/CrHalf.h"

#include "CrOSWindow.h"
#include "Editor/CrImGuiViewports.h"

#include "Rendering/Shaders/DirectLightingShared.hlsl"

#include <imgui.h>

static const char* GBufferDebugString[] =
{
	"None",
	"Albedo",
	"World Normals",
	"Roughness",
	"F0",
	"Depth",
	"Depth Linear"
};

static_assert(sizeof_array(GBufferDebugString) == GBufferDebugMode::Count, "");

// TODO Put somewhere else
bool HashingAssert()
{
	CrGraphicsPipelineDescriptor defaultDescriptor;
	CrHash defaultDescriptorHash = CrHash(defaultDescriptor);
	CrAssertMsg(defaultDescriptorHash == CrHash(14802820647099604725u), "Failed to hash known pipeline descriptor!");
	return true;
}

// Batches render packets
struct CrRenderPacketBatcher
{
	CrRenderPacketBatcher(ICrCommandBuffer* commandBuffer)
	{
		m_commandBuffer = commandBuffer;
		m_maxBatchSize = (uint32_t)m_matrices.size();
	}

	void SetMaximumBatchSize(uint32_t batchSize)
	{
		m_maxBatchSize = CrMin(batchSize, (uint32_t)m_matrices.size());
	}

	// Adds a render packet to the batcher and tries to batch it with preceding packets
	// If unable to batch or buffer is full, flush and repeat
	void ProcessRenderPacket(const CrRenderPacket& renderPacket)
	{
		bool stateMismatch =
			renderPacket.pipeline != m_pipeline ||
			renderPacket.material != m_material ||
			renderPacket.renderMesh != m_renderMesh ||
			renderPacket.extra != m_extra;

		bool noMoreSpace = (m_numInstances + renderPacket.numInstances) > m_maxBatchSize;

		if (stateMismatch || noMoreSpace)
		{
			FlushBatch();
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

	void FlushBatch()
	{
		if (m_numInstances > 0)
		{
			// Allocate constant buffer with all transforms and copy them across
			CrGPUBufferViewT<InstanceCB> transformBuffer = m_commandBuffer->AllocateConstantBuffer<InstanceCB>(sizeof(InstanceCB::local2World[0]) * m_numInstances);
			hlslpp::interop::float4x4* transforms = (hlslpp::interop::float4x4*)transformBuffer.GetData();
			{
				for (uint32_t i = 0; i < m_numInstances; ++i)
				{
					transforms[i] = *m_matrices[i];
				}
			}
			m_commandBuffer->BindConstantBuffer(transformBuffer);

			m_commandBuffer->BindGraphicsPipelineState(m_pipeline);

			for (uint32_t t = 0; t < m_material->m_textures.size(); ++t)
			{
				CrMaterial::TextureBinding binding = m_material->m_textures[t];
				m_commandBuffer->BindTexture(binding.semantic, binding.texture.get());
			}

			CrGPUBufferViewT<MaterialCB> materialBuffer = m_commandBuffer->AllocateConstantBuffer<MaterialCB>();
			MaterialCB* materialData = materialBuffer.GetData();
			{
				materialData->color = m_material->m_color;
				materialData->emissive = m_material->m_emissive;
			}
			m_commandBuffer->BindConstantBuffer(materialBuffer);

			for (uint32_t streamIndex = 0; streamIndex < m_renderMesh->GetVertexBufferCount(); ++streamIndex)
			{
				m_commandBuffer->BindVertexBuffer(m_renderMesh->GetVertexBuffer(streamIndex).get(), streamIndex);
			}

			m_commandBuffer->BindIndexBuffer(m_renderMesh->GetIndexBuffer().get());

			m_commandBuffer->DrawIndexed(m_renderMesh->GetIndexBuffer()->GetNumElements(), m_numInstances, 0, 0, 0);

			// Clear the batch, we want to avoid rendering twice if we call this function again
			m_numInstances = 0;
		}
	}

	bool m_batchStarted = false;

	uint32_t m_numInstances = 0;
	const CrMaterial* m_material = nullptr;
	const CrRenderMesh* m_renderMesh = nullptr;
	const ICrGraphicsPipeline* m_pipeline = nullptr;
	const void* m_extra = nullptr;

	ICrCommandBuffer* m_commandBuffer = nullptr;

	uint32_t m_maxBatchSize = 0;

	// Has to match the maximum number of matrices declared in the shader
	crstl::array<float4x4*, sizeof_array(InstanceCB::local2World)> m_matrices;
};

void CrFrame::Initialize(crstl::intrusive_ptr<CrOSWindow> mainWindow)
{
	HashingAssert();

	m_mainWindow = mainWindow;

	CrRenderDeviceHandle renderDevice = RenderSystem->GetRenderDevice();

	// TODO Move block to rendering subsystem initialization function
	{
		// Initialize ImGui renderer
		CrImGuiRendererInitParams imguiInitParams = {};
		imguiInitParams.m_swapchainFormat = CrRendererConfig::SwapchainFormat; // TODO How to conform to swapchain format?
		CrImGuiRenderer::Initialize(imguiInitParams);

		CrEditor::Initialize(mainWindow);
	}

	// Create the rendering scratch. Start with 10MB
	m_renderingStream = crstl::intrusive_ptr<CrCPUStackAllocator>(new CrCPUStackAllocator());
	m_renderingStream->Initialize(10 * 1024 * 1024);

	m_camera = CrCameraHandle(new CrCamera());

	// Create a render world
	m_renderWorld = CrRenderWorldHandle(new CrRenderWorld());

	Editor->SetRenderWorld(m_renderWorld);

	bool loadData = true;

	if (loadData)
	{
		CrRenderModelHandle nyraModel = CrResourceManager::LoadModel(CrResourceManager::GetFullResourcePath("nyra/nyra_pose_mod.fbx"));
		CrRenderModelHandle jainaModel = CrResourceManager::LoadModel(CrResourceManager::GetFullResourcePath("jaina/storm_hero_jaina.fbx"));
		CrRenderModelHandle damagedHelmet = CrResourceManager::LoadModel(CrResourceManager::GetFullResourcePath("gltf-helmet/DamagedHelmet.gltf"));

		const uint32_t numModels = 100;

		for (uint32_t i = 0; i < numModels; ++i)
		{
			CrModelInstanceId modelInstanceId = m_renderWorld->CreateModelInstance();
			CrModelInstance& modelInstance = m_renderWorld->GetModelInstance(modelInstanceId);

			float angle = 2.39996322f * i;
			float radius = 30.0f * i / numModels;

			float x = radius * sinf(angle);
			float z = radius * cosf(angle);

			float4x4 transformMatrix = float4x4::translation(x, 0.0f, z);

			modelInstance.SetTransform(transformMatrix);

			int r = rand();
			CrRenderModelHandle renderModel;
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
		
			modelInstance.SetRenderModel(renderModel);
		}
	}

	m_renderWorld->SetCamera(m_camera);

	CrTextureDescriptor rwTextureParams;
	rwTextureParams.width = 64;
	rwTextureParams.height = 64;
	rwTextureParams.format = cr3d::DataFormat::RGBA16_Unorm;
	rwTextureParams.usage = cr3d::TextureUsage::UnorderedAccess;
	rwTextureParams.name = "Colors RW Texture";
	m_colorsRWTexture = renderDevice->CreateTexture(rwTextureParams);

	m_colorsRWTypedBuffer = renderDevice->CreateTypedBuffer(cr3d::MemoryAccess::GPUOnlyWrite, cr3d::DataFormat::RGBA8_Unorm, 128);
	m_exampleComputePipeline = BuiltinPipelines->GetComputePipeline(CrBuiltinShaders::ExampleCompute);
	m_depthDownsampleLinearize = BuiltinPipelines->GetComputePipeline(CrBuiltinShaders::DepthDownsampleLinearizeMinMax);
	m_mouseSelectionResolvePipeline = BuiltinPipelines->GetComputePipeline(CrBuiltinShaders::EditorMouseSelectionResolveCS);

	{
		CrGraphicsPipelineDescriptor copyTextureGraphicsPipelineDescriptor;
		copyTextureGraphicsPipelineDescriptor.renderTargets.colorFormats[0] = cr3d::DataFormat::BGRA8_Unorm;
		copyTextureGraphicsPipelineDescriptor.depthStencilState.depthTestEnable = false;
		m_copyTexturePipeline = BuiltinPipelines->GetGraphicsPipeline(copyTextureGraphicsPipelineDescriptor, NullVertexDescriptor, CrBuiltinShaders::FullscreenTriangle, CrBuiltinShaders::CopyTextureColor);
	}

	m_createIndirectArguments = BuiltinPipelines->GetComputePipeline(CrBuiltinShaders::CreateIndirectArguments);

	m_postProcessing = BuiltinPipelines->GetComputePipeline(CrBuiltinShaders::PostProcessingCS);

	{
		CrGraphicsPipelineDescriptor directionalLightPipelineDescriptor;
		directionalLightPipelineDescriptor.renderTargets.colorFormats[0] = CrRendererConfig::LightingFormat;
		directionalLightPipelineDescriptor.depthStencilState.depthTestEnable = false;
		directionalLightPipelineDescriptor.depthStencilState.depthWriteEnable = false;
		m_directionalLightPipeline = BuiltinPipelines->GetGraphicsPipeline(directionalLightPipelineDescriptor, NullVertexDescriptor, CrBuiltinShaders::FullscreenTriangle, CrBuiltinShaders::DirectionalLightPS);
	}

	{
		CrGraphicsPipelineDescriptor gbufferDebugPipelineDescriptor;
		gbufferDebugPipelineDescriptor.renderTargets.colorFormats[0] = CrRendererConfig::SwapchainFormat;
		gbufferDebugPipelineDescriptor.depthStencilState.depthTestEnable = false;
		gbufferDebugPipelineDescriptor.depthStencilState.depthWriteEnable = false;
		m_gbufferDebugPipeline = BuiltinPipelines->GetGraphicsPipeline(gbufferDebugPipelineDescriptor, NullVertexDescriptor, CrBuiltinShaders::FullscreenTriangle, CrBuiltinShaders::GBufferDebugPS);
	}

	// Editor shaders
	{
		CrGraphicsPipelineDescriptor editorEdgeSelectionPipelineDescriptor;
		editorEdgeSelectionPipelineDescriptor.renderTargets.colorFormats[0] = cr3d::DataFormat::BGRA8_Unorm;
		editorEdgeSelectionPipelineDescriptor.depthStencilState.depthTestEnable = false;
		editorEdgeSelectionPipelineDescriptor.blendState.renderTargetBlends[0].enable = true;
		editorEdgeSelectionPipelineDescriptor.blendState.renderTargetBlends[0].colorBlendOp = cr3d::BlendOp::Add;
		m_editorEdgeSelectionPipeline = BuiltinPipelines->GetGraphicsPipeline(editorEdgeSelectionPipelineDescriptor, NullVertexDescriptor, CrBuiltinShaders::FullscreenTriangle, CrBuiltinShaders::EditorEdgeSelectionPS);
	
		CrGraphicsPipelineDescriptor editorGridPipelineDescriptor;
		editorGridPipelineDescriptor.renderTargets.colorFormats[0] = cr3d::DataFormat::BGRA8_Unorm;
		editorGridPipelineDescriptor.renderTargets.depthFormat = CrRendererConfig::DepthBufferFormat;
		editorGridPipelineDescriptor.depthStencilState.depthTestEnable = true;
		editorGridPipelineDescriptor.rasterizerState.depthClipEnable = false;
		editorGridPipelineDescriptor.blendState.renderTargetBlends[0].enable = true;
		editorGridPipelineDescriptor.blendState.renderTargetBlends[0].colorBlendOp = cr3d::BlendOp::Add;
		editorGridPipelineDescriptor.blendState.renderTargetBlends[0].srcColorBlendFactor = cr3d::BlendFactor::SrcAlpha;
		editorGridPipelineDescriptor.blendState.renderTargetBlends[0].dstColorBlendFactor = cr3d::BlendFactor::OneMinusSrcAlpha;
		m_editorGridPipeline = BuiltinPipelines->GetGraphicsPipeline(editorGridPipelineDescriptor, NullVertexDescriptor, CrBuiltinShaders::EditorGridVS, CrBuiltinShaders::EditorGridPS);
	}

	{
		CrTextureDescriptor colorfulVolumeTextureDescriptor;
		colorfulVolumeTextureDescriptor.width = 4;
		colorfulVolumeTextureDescriptor.height = 4;
		colorfulVolumeTextureDescriptor.depth = 4;
		colorfulVolumeTextureDescriptor.type = cr3d::TextureType::Volume;

		uint8_t colorfulVolumeTextureInitialData[4 * 4 * 4 * 4];
		for (uint32_t z = 0; z < 4; ++z)
		{
			for (uint32_t x = 0; x < 4; ++x)
			{
				for (uint32_t y = 0; y < 4; ++y)
				{
					colorfulVolumeTextureInitialData[z * 4 * 4 * 4 + y * 4 * 4 + x * 4 + 0] = z == 0 ? 0xff : 0;
					colorfulVolumeTextureInitialData[z * 4 * 4 * 4 + y * 4 * 4 + x * 4 + 1] = z == 1 ? 0xff : 0;
					colorfulVolumeTextureInitialData[z * 4 * 4 * 4 + y * 4 * 4 + x * 4 + 2] = z == 2 ? 0xff : 0;
					colorfulVolumeTextureInitialData[z * 4 * 4 * 4 + y * 4 * 4 + x * 4 + 3] = z == 3 ? 0xff : 0;
				}
			}
		}

		colorfulVolumeTextureDescriptor.name = "Colorful Volume Texture";
		colorfulVolumeTextureDescriptor.initialData = colorfulVolumeTextureInitialData;
		colorfulVolumeTextureDescriptor.initialDataSize = sizeof(colorfulVolumeTextureInitialData);
		m_colorfulVolumeTexture = renderDevice->CreateTexture(colorfulVolumeTextureDescriptor);
	}

	{
		CrTextureDescriptor colorfulTextureArrayDescriptor;
		colorfulTextureArrayDescriptor.width = 4;
		colorfulTextureArrayDescriptor.height = 4;
		colorfulTextureArrayDescriptor.type = cr3d::TextureType::Tex2D;
		colorfulTextureArrayDescriptor.arraySize = 4;
		colorfulTextureArrayDescriptor.mipmapCount = 3;

		uint8_t colorfulTextureArrayInitialData[4 * 4 * 4 * 4];
		for (uint32_t z = 0; z < 4; ++z)
		{
			for (uint32_t x = 0; x < 4; ++x)
			{
				for (uint32_t y = 0; y < 4; ++y)
				{
					colorfulTextureArrayInitialData[z * 4 * 4 * 4 + y * 4 * 4 + x * 4 + 0] = z == 0 ? 0xff : 0;
					colorfulTextureArrayInitialData[z * 4 * 4 * 4 + y * 4 * 4 + x * 4 + 1] = z == 1 ? 0xff : 0;
					colorfulTextureArrayInitialData[z * 4 * 4 * 4 + y * 4 * 4 + x * 4 + 2] = z == 2 ? 0xff : 0;
					colorfulTextureArrayInitialData[z * 4 * 4 * 4 + y * 4 * 4 + x * 4 + 3] = z == 3 ? 0xff : 0;
				}
			}
		}

		colorfulTextureArrayDescriptor.name = "Colorful Texture Array";
		colorfulTextureArrayDescriptor.initialData = colorfulTextureArrayInitialData;
		colorfulTextureArrayDescriptor.initialDataSize = sizeof(colorfulTextureArrayInitialData);
		m_colorfulTextureArray = renderDevice->CreateTexture(colorfulTextureArrayDescriptor);
	}

	m_rwStructuredBuffer = renderDevice->CreateStructuredBuffer<ExampleRWStructuredBufferCompute>(cr3d::MemoryAccess::GPUOnlyWrite, 32);

	m_structuredBuffer = renderDevice->CreateStructuredBuffer<ExampleStructuredBufferCompute>(cr3d::MemoryAccess::GPUOnlyRead, 32);

	CrGPUBufferDescriptor argumentsDescriptor(cr3d::BufferUsage::Indirect | cr3d::BufferUsage::Byte, cr3d::MemoryAccess::GPUOnlyWrite);
	m_indirectDispatchArguments = CrGPUBufferHandle(new CrGPUBuffer(renderDevice.get(), argumentsDescriptor, 3, 4));

	uint32_t initialValue = 65535;
	CrGPUBufferDescriptor mouseSelectionBufferDescriptor(cr3d::BufferUsage::Indirect | cr3d::BufferUsage::Byte | cr3d::BufferUsage::TransferSrc | cr3d::BufferUsage::TransferDst, cr3d::MemoryAccess::GPUOnlyWrite);
	mouseSelectionBufferDescriptor.initialData = (uint8_t*)&initialValue;
	mouseSelectionBufferDescriptor.initialDataSize = sizeof(initialValue);
	mouseSelectionBufferDescriptor.name = "Mouse Selection Entity Id Buffer";
	m_mouseSelectionBuffer = CrGPUBufferHandle(new CrGPUBuffer(renderDevice.get(), mouseSelectionBufferDescriptor, 1, 4));
}

void CrFrame::Deinitialize()
{
	CrImGuiRenderer::Deinitialize();

	CrEditor::Deinitialize();

	m_drawCmdBuffers.clear();
}

void CrFrame::Process()
{
	const CrRenderDeviceHandle& renderDevice = RenderSystem->GetRenderDevice();

	uint32_t windowWidth, windowHeight;
	m_mainWindow->GetSizePixels(windowWidth, windowHeight);

	// Resource resizes are not immediately processed but deferred to the main loop. That way we have control over where it happens
	if (m_currentWindowWidth != windowWidth || m_currentWindowHeight != windowHeight)
	{
		RecreateRenderTargets();
		m_currentWindowWidth = windowWidth;
		m_currentWindowHeight = windowHeight;
	}

	const ImGuiPlatformIO& imguiPlatformIO = ImGui::GetPlatformIO();

	for (int i = 0; i < imguiPlatformIO.Viewports.Size; i++)
	{
		ImGuiViewport* imguiViewport = imguiPlatformIO.Viewports[i];
		ImGuiViewportsData* viewportData = (ImGuiViewportsData*)imguiViewport->PlatformUserData;
		CrOSWindow* osWindow = viewportData->osWindow;
		if (osWindow)
		{
			osWindow->GetSwapchain()->AcquireNextImage();
		}
	}

	CrRenderingStatistics::Reset();

	ICrCommandBuffer* drawCommandBuffer = m_drawCmdBuffers[m_currentCommandBuffer].get();

	const MouseState& mouseState = CrInput.GetMouseState();
	const KeyboardState& keyboardState = CrInput.GetKeyboardState();

	if (keyboardState.keyHeld[KeyboardKey::LeftCtrl] &&
		keyboardState.keyHeld[KeyboardKey::LeftShift] &&
		keyboardState.keyPressed[KeyboardKey::F5])
	{
		BuiltinPipelines->RecompileComputePipelines();
	}

	ImGuiRenderer->NewFrame(m_mainWindow);

	Editor->Update();

	// TODO We need to rework this once we have windows and views so that these properties are set on the camera when we change things around
	// For now keep it this way knowing it will be changed
	m_camera->SetupPerspective(m_swapchain->GetWidth(), m_swapchain->GetHeight(), 0.1f, 1000.0f);

	//--------------
	// RENDER THREAD
	//--------------

	renderDevice->ProcessQueuedCommands();

	// Set up render graph to start recording passes
	CrRenderGraphFrameParams frameRenderGraphParams;
	frameRenderGraphParams.commandBuffer = drawCommandBuffer;
	frameRenderGraphParams.timingQueryTracker = m_timingQueryTracker.get();
	frameRenderGraphParams.frameIndex = CrFrameTime::GetFrameIndex();
	m_mainRenderGraph.Begin(frameRenderGraphParams);

	m_renderingStream->Reset();

	m_renderWorld->BeginRendering(m_renderingStream);

	m_renderWorld->ComputeVisibilityAndRenderPackets();

	drawCommandBuffer->Begin();

	drawCommandBuffer->BindTexture(Textures::DiffuseTexture0, RenderingResources->WhiteSmallTexture.get());
	drawCommandBuffer->BindTexture(Textures::NormalTexture0, RenderingResources->NormalsSmallTexture.get());
	drawCommandBuffer->BindTexture(Textures::SpecularTexture0, RenderingResources->WhiteSmallTexture.get());

	drawCommandBuffer->BindSampler(Samplers::AllLinearClampSampler, RenderingResources->AllLinearClampSampler.get());
	drawCommandBuffer->BindSampler(Samplers::AllLinearWrapSampler, RenderingResources->AllLinearWrapSampler.get());
	drawCommandBuffer->BindSampler(Samplers::AllPointClampSampler, RenderingResources->AllPointClampSampler.get());
	drawCommandBuffer->BindSampler(Samplers::AllPointWrapSampler, RenderingResources->AllPointWrapSampler.get());

	// Set up default values for common constant buffers

	CrGPUBufferViewT<MaterialCB> materialBuffer = drawCommandBuffer->AllocateConstantBuffer<MaterialCB>();
	MaterialCB* materialData = materialBuffer.GetData();
	{
		materialData->color = float4(1.0f, 1.0f, 1.0f, 1.0f);
		materialData->emissive = float4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	drawCommandBuffer->BindConstantBuffer(materialBuffer);

	const float4x4 view2ProjectionMatrix = m_camera->GetView2ProjectionMatrix();

	m_cameraConstantData.world2View = m_camera->GetWorld2ViewMatrix();
	m_cameraConstantData.view2Projection = view2ProjectionMatrix;
	m_cameraConstantData.view2WorldRotation = (const float3x4&)m_camera->GetView2WorldMatrix();
	m_cameraConstantData.linearization = m_camera->ComputeLinearizationParams();
	m_cameraConstantData.backprojection = CrCamera::ComputeBackprojectionParams(view2ProjectionMatrix);
	m_cameraConstantData.screenResolution = float4
	(
		m_camera->GetResolutionWidth(),
		m_camera->GetResolutionHeight(),
		1.0f / m_camera->GetResolutionWidth(),
		1.0f / m_camera->GetResolutionHeight()
	);
	m_cameraConstantData.worldPosition = float4(m_camera->GetPosition(), m_camera->GetNearPlane());

	CrGPUBufferViewT<CameraCB> cameraDataBuffer = drawCommandBuffer->AllocateConstantBuffer<CameraCB>();
	CameraCB* cameraData = cameraDataBuffer.GetData();
	{
		*cameraData = m_cameraConstantData;
	}
	drawCommandBuffer->BindConstantBuffer(cameraDataBuffer);

	CrGPUBufferViewT<InstanceCB> identityConstantBuffer = drawCommandBuffer->AllocateConstantBuffer<InstanceCB>();
	InstanceCB* identityTransformData = identityConstantBuffer.GetData();
	{
		identityTransformData->local2World[0] = float4x4::identity();
	}

	m_timingQueryTracker->BeginFrame(drawCommandBuffer, CrFrameTime::GetFrameIndex());

	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("GBuffer Pass"), float4(160, 180, 150, 255) / 255.0f, CrRenderGraphPassType::Graphics,
	[&](CrRenderGraph& renderGraph)
	{
		renderGraph.BindDepthStencilTarget(m_depthStencilTexture.get(),
			CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, 0.0f,
			CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, 0);
		renderGraph.BindRenderTarget(m_gbufferAlbedoAOTexture.get(), CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, float4(0.0f, 0.0f, 0.0f, 0.0f));
		renderGraph.BindRenderTarget(m_gbufferNormalsTexture.get(), CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, float4(0.0f, 0.0f, 0.0f, 0.0f));
		renderGraph.BindRenderTarget(m_gbufferMaterialTexture.get(), CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, float4(0.0f, 0.0f, 0.0f, 0.0f));
	},
	[=](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
	{
		commandBuffer->SetViewport(CrViewport(0.0f, 0.0f, (float)m_gbufferAlbedoAOTexture->GetWidth(), (float)m_gbufferAlbedoAOTexture->GetHeight()));
		commandBuffer->SetScissor(CrRectangle(0, 0, m_gbufferAlbedoAOTexture->GetWidth(), m_gbufferAlbedoAOTexture->GetHeight()));

		const CrRenderList& gBufferRenderList = m_renderWorld->GetRenderList(CrRenderListUsage::GBuffer);

		CrRenderPacketBatcher renderPacketBatcher(commandBuffer);

		gBufferRenderList.ForEachRenderPacket([&](const CrRenderPacket& renderPacket)
		{
			renderPacketBatcher.ProcessRenderPacket(renderPacket);
		});

		renderPacketBatcher.FlushBatch(); // Execute the last batch
	});

	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Depth Downsample Linearize"), float4(160, 160, 160, 255) / 255.0f, CrRenderGraphPassType::Compute,
	[=](CrRenderGraph& renderGraph)
	{
		renderGraph.BindTexture(Textures::RawDepthTexture, m_depthStencilTexture.get(), cr3d::ShaderStageFlags::Compute);
		renderGraph.BindRWTexture(RWTextures::RWLinearDepthMinMaxMip1, m_linearDepth16MinMaxMipChain.get(), cr3d::ShaderStageFlags::Compute, 0);
		renderGraph.BindRWTexture(RWTextures::RWLinearDepthMinMaxMip2, m_linearDepth16MinMaxMipChain.get(), cr3d::ShaderStageFlags::Compute, 1);
		renderGraph.BindRWTexture(RWTextures::RWLinearDepthMinMaxMip3, m_linearDepth16MinMaxMipChain.get(), cr3d::ShaderStageFlags::Compute, 2);
		renderGraph.BindRWTexture(RWTextures::RWLinearDepthMinMaxMip4, m_linearDepth16MinMaxMipChain.get(), cr3d::ShaderStageFlags::Compute, 3);
	},
	[=](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
	{
		commandBuffer->BindComputePipelineState(m_depthDownsampleLinearize.get());
		commandBuffer->BindTexture(Textures::RawDepthTexture, m_depthStencilTexture.get(), CrTextureView(cr3d::TexturePlane::Depth));
		commandBuffer->BindTexture(Textures::StencilTexture, m_depthStencilTexture.get(), CrTextureView(cr3d::TexturePlane::Stencil));
		commandBuffer->BindRWTexture(RWTextures::RWLinearDepthMinMaxMip1, m_linearDepth16MinMaxMipChain.get(), 0);
		commandBuffer->BindRWTexture(RWTextures::RWLinearDepthMinMaxMip2, m_linearDepth16MinMaxMipChain.get(), 1);
		commandBuffer->BindRWTexture(RWTextures::RWLinearDepthMinMaxMip3, m_linearDepth16MinMaxMipChain.get(), 2);
		commandBuffer->BindRWTexture(RWTextures::RWLinearDepthMinMaxMip4, m_linearDepth16MinMaxMipChain.get(), 3);
		commandBuffer->Dispatch
		(
			(m_linearDepth16MinMaxMipChain->GetWidth() + 7) / 8,
			(m_linearDepth16MinMaxMipChain->GetHeight() + 7) / 8,
			1
		);
	});

	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Lighting Pass"), float4(200, 170, 220, 255) / 255.0f, CrRenderGraphPassType::Graphics,
	[this](CrRenderGraph& renderGraph)
	{
		renderGraph.BindRenderTarget(m_lightingTexture.get(), CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, 0.0f);
		renderGraph.BindTexture(Textures::GBufferDepthTexture, m_depthStencilTexture.get(), cr3d::ShaderStageFlags::Pixel);
		renderGraph.BindTexture(Textures::GBufferAlbedoAOTexture, m_gbufferAlbedoAOTexture.get(), cr3d::ShaderStageFlags::Pixel);
		renderGraph.BindTexture(Textures::GBufferNormalsTexture, m_gbufferNormalsTexture.get(), cr3d::ShaderStageFlags::Pixel);
		renderGraph.BindTexture(Textures::GBufferMaterialTexture, m_gbufferMaterialTexture.get(), cr3d::ShaderStageFlags::Pixel);
	},
	[this]
	(const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
	{
		CrGPUBufferViewT<DynamicLightCB> lightConstantBuffer = commandBuffer->AllocateConstantBuffer<DynamicLightCB>();
		DynamicLightCB* lightData = lightConstantBuffer.GetData();
		{
			lightData->positionRadius = float4(0.0f, 1.0f, 0.0f, 1.0f);
			lightData->colorIntensity = float4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		commandBuffer->SetViewport(CrViewport(0, 0, m_lightingTexture->GetWidth(), m_lightingTexture->GetHeight()));
		commandBuffer->BindConstantBuffer(lightConstantBuffer);
		commandBuffer->BindTexture(Textures::GBufferDepthTexture, m_depthStencilTexture.get());
		commandBuffer->BindTexture(Textures::GBufferAlbedoAOTexture, m_gbufferAlbedoAOTexture.get());
		commandBuffer->BindTexture(Textures::GBufferNormalsTexture, m_gbufferNormalsTexture.get());
		commandBuffer->BindTexture(Textures::GBufferMaterialTexture, m_gbufferMaterialTexture.get());
		commandBuffer->BindGraphicsPipelineState(m_directionalLightPipeline.get());
		commandBuffer->Draw(3, 1, 0, 0);
	});

	if (m_renderWorld->HasRenderList(CrRenderListUsage::Transparency))
	{
		m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Transparency Pass"), float4(180, 180, 204, 255) / 255.0f, CrRenderGraphPassType::Graphics,
		[this](CrRenderGraph& renderGraph)
		{
			renderGraph.BindRenderTarget(m_lightingTexture.get(), CrRenderTargetLoadOp::Load, CrRenderTargetStoreOp::Store);
			renderGraph.BindDepthStencilTarget(m_depthStencilTexture.get(), CrRenderTargetLoadOp::Load, CrRenderTargetStoreOp::Store, 0.0f);
		},
		[this](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
		{
			commandBuffer->SetViewport(CrViewport(0.0f, 0.0f, (float)m_swapchain->GetWidth(), (float)m_swapchain->GetHeight()));
			commandBuffer->SetScissor(CrRectangle(0, 0, m_swapchain->GetWidth(), m_swapchain->GetHeight()));

			const CrRenderList& forwardRenderList = m_renderWorld->GetRenderList(CrRenderListUsage::Transparency);

			CrRenderPacketBatcher renderPacketBatcher(commandBuffer);
			forwardRenderList.ForEachRenderPacket([&](const CrRenderPacket& renderPacket)
			{
				renderPacketBatcher.ProcessRenderPacket(renderPacket);
			});

			renderPacketBatcher.FlushBatch(); // Execute the last batch
		});
	}

	// PostProcessing (includes Tonemapping)
	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Post Processing"), float4(200, 170, 130, 255) / 255.0f, CrRenderGraphPassType::Compute,
	[this](CrRenderGraph& renderGraph)
	{
		renderGraph.BindTexture(Textures::HDRInput, m_lightingTexture.get(), cr3d::ShaderStageFlags::Compute);
		renderGraph.BindRWTexture(RWTextures::RWPostProcessedOutput, m_preSwapchainTexture.get(), cr3d::ShaderStageFlags::Compute);
	},
	[this](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
	{
		commandBuffer->BindTexture(Textures::HDRInput, m_lightingTexture.get());
		commandBuffer->BindRWTexture(RWTextures::RWPostProcessedOutput, m_preSwapchainTexture.get(), 0);
		commandBuffer->BindComputePipelineState(m_postProcessing.get());

		uint32_t groupSizeX = m_postProcessing->GetGroupSizeX();
		uint32_t groupSizeY = m_postProcessing->GetGroupSizeY();

		commandBuffer->Dispatch
		(
			(m_lightingTexture->GetWidth() + groupSizeX - 1) / groupSizeX,
			(m_lightingTexture->GetHeight() + groupSizeY - 1) / groupSizeY,
			1
		);
	});

	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Editor Grid Render"), float4(200, 70, 100, 255) / 255.0f, CrRenderGraphPassType::Graphics,
	[this](CrRenderGraph& renderGraph)
	{
		renderGraph.BindRenderTarget(m_preSwapchainTexture.get(), CrRenderTargetLoadOp::Load, CrRenderTargetStoreOp::Store);
		renderGraph.BindDepthStencilTarget(m_depthStencilTexture.get(), CrRenderTargetLoadOp::Load, CrRenderTargetStoreOp::Store);
	},
	[this](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
	{
		float subdivisionWidth = 1.0f;

		CrGPUBufferViewT<EditorGridCB> gridCB = commandBuffer->AllocateConstantBuffer<EditorGridCB>();
		EditorGridCB* gridData = gridCB.GetData();
		{
			gridData->gridParams = float4(1000.0f, subdivisionWidth, 0.0f, 0.0f);
		}

		commandBuffer->BindConstantBuffer(gridCB);
		commandBuffer->BindGraphicsPipelineState(m_editorGridPipeline.get());
		commandBuffer->Draw(6, 1, 0, 0);
	});

	if (m_renderWorld->HasRenderList(CrRenderListUsage::EdgeSelection))
	{
		m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Edge Selection Render"), float4(200, 70, 100, 255) / 255.0f, CrRenderGraphPassType::Graphics,
		[this](CrRenderGraph& renderGraph)
		{
			renderGraph.BindRenderTarget(m_debugShaderTexture.get(), CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, float4(0.0f, 0.0f, 0.0f, 0.0f));
			renderGraph.BindDepthStencilTarget(m_depthStencilTexture.get(), CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, 0.0f);
		},
		[this](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
		{
			commandBuffer->SetViewport(CrViewport(0.0f, 0.0f, (float)m_swapchain->GetWidth(), (float)m_swapchain->GetHeight()));
			commandBuffer->SetScissor(CrRectangle(0, 0, m_swapchain->GetWidth(), m_swapchain->GetHeight()));

			const CrRenderList& edgeSelectionRenderList = m_renderWorld->GetRenderList(CrRenderListUsage::EdgeSelection);

			CrRenderPacketBatcher renderPacketBatcher(commandBuffer);

			CrGPUBufferViewT<DebugShaderCB> debugShaderBuffer = commandBuffer->AllocateConstantBuffer<DebugShaderCB>();
			DebugShaderCB* debugShaderData = debugShaderBuffer.GetData();
			{
				debugShaderData->debugProperties = float4(1.0f, 0.0f, 0.0f, 0.0f);
			}
			commandBuffer->BindConstantBuffer(debugShaderBuffer);

			edgeSelectionRenderList.ForEachRenderPacket([&](const CrRenderPacket& renderPacket)
			{
				renderPacketBatcher.ProcessRenderPacket(renderPacket);
			});

			renderPacketBatcher.FlushBatch(); // Execute the last batch
		});

		m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Edge Selection Resolve"), float4(200, 70, 100, 255) / 255.0f, CrRenderGraphPassType::Graphics,
		[=](CrRenderGraph& renderGraph)
		{
			renderGraph.BindRenderTarget(m_preSwapchainTexture.get(), CrRenderTargetLoadOp::Load, CrRenderTargetStoreOp::Store);
			renderGraph.BindTexture(Textures::EditorSelectionTexture, m_debugShaderTexture.get(), cr3d::ShaderStageFlags::Pixel);
		},
		[this](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
		{
			commandBuffer->BindTexture(Textures::EditorSelectionTexture, m_debugShaderTexture.get());
			commandBuffer->BindGraphicsPipelineState(m_editorEdgeSelectionPipeline.get());
			commandBuffer->Draw(3, 1, 0, 0);
		});
	}

	if (m_gbufferDebugMode != GBufferDebugMode::None)
	{
		// Override with the decoded GBuffer
		m_mainRenderGraph.AddRenderPass(CrRenderGraphString("GBuffer Debug Pass"), float4(128.0f, 0.0f, 128.0f, 1.0f) / 255.0f, CrRenderGraphPassType::Graphics,
		[=](CrRenderGraph& renderGraph)
		{
			renderGraph.BindRenderTarget(m_preSwapchainTexture.get(), CrRenderTargetLoadOp::DontCare, CrRenderTargetStoreOp::Store, float4(0.0f));
			renderGraph.BindTexture(Textures::GBufferDepthTexture, m_depthStencilTexture.get(), cr3d::ShaderStageFlags::Pixel);
			renderGraph.BindTexture(Textures::GBufferAlbedoAOTexture, m_gbufferAlbedoAOTexture.get(), cr3d::ShaderStageFlags::Pixel);
			renderGraph.BindTexture(Textures::GBufferNormalsTexture, m_gbufferNormalsTexture.get(), cr3d::ShaderStageFlags::Pixel);
			renderGraph.BindTexture(Textures::GBufferMaterialTexture, m_gbufferMaterialTexture.get(), cr3d::ShaderStageFlags::Pixel);
		},
		[this](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
		{
			CrGPUBufferViewT<GBufferDebugCB> gbufferDebug = commandBuffer->AllocateConstantBuffer<GBufferDebugCB>();
			GBufferDebugCB* gbufferDebugData = gbufferDebug.GetData();
			{
				gbufferDebugData->decodeOptions = uint4((uint32_t)m_gbufferDebugMode, 0.0f, 0.0f, 0.0f);
			}
			commandBuffer->BindConstantBuffer(gbufferDebug);

			commandBuffer->BindTexture(Textures::GBufferDepthTexture, m_depthStencilTexture.get());
			commandBuffer->BindTexture(Textures::GBufferAlbedoAOTexture, m_gbufferAlbedoAOTexture.get());
			commandBuffer->BindTexture(Textures::GBufferNormalsTexture, m_gbufferNormalsTexture.get());
			commandBuffer->BindTexture(Textures::GBufferMaterialTexture, m_gbufferMaterialTexture.get());
			commandBuffer->BindGraphicsPipelineState(m_gbufferDebugPipeline.get());
			commandBuffer->Draw(3, 1, 0, 0);
		});
	}

	CrTextureHandle swapchainTexture = m_swapchain->GetCurrentTexture();

	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Copy Pass"), float4(1.0f, 0.0f, 1.0f, 1.0f), CrRenderGraphPassType::Graphics,
	[this, &swapchainTexture](CrRenderGraph& renderGraph)
	{
		renderGraph.BindTexture(Textures::CopyTexture, m_preSwapchainTexture.get(), cr3d::ShaderStageFlags::Pixel);
		renderGraph.BindRenderTarget(swapchainTexture.get(), CrRenderTargetLoadOp::DontCare, CrRenderTargetStoreOp::Store, float4(0.0f));
	},
	[this](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
	{
		commandBuffer->BindTexture(Textures::CopyTexture, m_preSwapchainTexture.get());
		commandBuffer->BindGraphicsPipelineState(m_copyTexturePipeline.get());
		commandBuffer->Draw(3, 1, 0, 0);
	});

	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Compute 1"), float4(0.0f, 0.0, 1.0f, 1.0f), CrRenderGraphPassType::Compute,
	[this](CrRenderGraph& renderGraph)
	{
		renderGraph.BindStorageBuffer(StorageBuffers::ExampleStructuredBufferCompute, m_structuredBuffer->GetHardwareBuffer(), cr3d::ShaderStageFlags::Compute);
		renderGraph.BindRWStorageBuffer(RWStorageBuffers::ExampleRWStructuredBufferCompute, m_rwStructuredBuffer->GetHardwareBuffer(), cr3d::ShaderStageFlags::Compute);
		renderGraph.BindRWTypedBuffer(RWTypedBuffers::ExampleRWTypedBufferCompute, m_colorsRWTypedBuffer->GetHardwareBuffer(), cr3d::ShaderStageFlags::Compute);
		renderGraph.BindRWTexture(RWTextures::ExampleRWTextureCompute, m_colorsRWTexture.get(), cr3d::ShaderStageFlags::Compute);
	},
	[=](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
	{
		commandBuffer->BindComputePipelineState(m_exampleComputePipeline.get());
		commandBuffer->BindStorageBuffer(StorageBuffers::ExampleStructuredBufferCompute, m_structuredBuffer->GetHardwareBuffer());
		commandBuffer->BindRWStorageBuffer(RWStorageBuffers::ExampleRWStructuredBufferCompute, m_rwStructuredBuffer->GetHardwareBuffer());
		commandBuffer->BindRWTypedBuffer(RWTypedBuffers::ExampleRWTypedBufferCompute, m_colorsRWTypedBuffer->GetHardwareBuffer());
		commandBuffer->BindRWTexture(RWTextures::ExampleRWTextureCompute, m_colorsRWTexture.get(), 0);
		commandBuffer->BindTexture(Textures::ExampleTexture3DCompute, m_colorfulVolumeTexture.get());
		commandBuffer->BindTexture(Textures::ExampleTextureArrayCompute, m_colorfulTextureArray.get());
		commandBuffer->Dispatch(1, 1, 1);
	});

	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Indirect Arguments Create"), float4(0.0f, 0.0, 1.0f, 1.0f), CrRenderGraphPassType::Compute,
	[&](CrRenderGraph& renderGraph)
	{
		renderGraph.BindRWStorageBuffer(RWStorageBuffers::ExampleIndirectBuffer, m_indirectDispatchArguments->GetHardwareBuffer(), cr3d::ShaderStageFlags::Compute);
	},
	[=](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
	{
		commandBuffer->BindComputePipelineState(m_createIndirectArguments.get());
		commandBuffer->BindRWStorageBuffer(RWStorageBuffers::ExampleIndirectBuffer, m_indirectDispatchArguments->GetHardwareBuffer());
		commandBuffer->Dispatch(1, 1, 1);
	});
	
	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Indirect Arguments Consume"), float4(0.0f, 0.0, 1.0f, 1.0f), CrRenderGraphPassType::Compute,
	[&](CrRenderGraph& renderGraph)
	{
		renderGraph.BindStorageBuffer(StorageBuffers::ExampleStructuredBufferCompute, m_structuredBuffer->GetHardwareBuffer(), cr3d::ShaderStageFlags::Compute);
		renderGraph.BindRWStorageBuffer(RWStorageBuffers::ExampleRWStructuredBufferCompute, m_rwStructuredBuffer->GetHardwareBuffer(), cr3d::ShaderStageFlags::Compute);
		renderGraph.BindRWTypedBuffer(RWTypedBuffers::ExampleRWTypedBufferCompute, m_colorsRWTypedBuffer->GetHardwareBuffer(), cr3d::ShaderStageFlags::Compute);
		renderGraph.BindRWTexture(RWTextures::ExampleRWTextureCompute, m_colorsRWTexture.get(), cr3d::ShaderStageFlags::Compute, 0, 0, 1);
	},
	[=](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
	{
		commandBuffer->BindComputePipelineState(m_exampleComputePipeline.get());
		commandBuffer->BindStorageBuffer(StorageBuffers::ExampleStructuredBufferCompute, m_structuredBuffer->GetHardwareBuffer());
		commandBuffer->BindRWStorageBuffer(RWStorageBuffers::ExampleRWStructuredBufferCompute, m_rwStructuredBuffer->GetHardwareBuffer());
		commandBuffer->BindRWTypedBuffer(RWTypedBuffers::ExampleRWTypedBufferCompute, m_colorsRWTypedBuffer->GetHardwareBuffer());
		commandBuffer->BindRWTexture(RWTextures::ExampleRWTextureCompute, m_colorsRWTexture.get(), 0);
		commandBuffer->BindTexture(Textures::ExampleTexture3DCompute, m_colorfulVolumeTexture.get());
		commandBuffer->BindTexture(Textures::ExampleTextureArrayCompute, m_colorfulTextureArray.get());
	
		commandBuffer->DispatchIndirect(m_indirectDispatchArguments->GetHardwareBuffer(), 0);
	});
	
	if (m_renderWorld->GetMouseSelectionEnabled())
	{
		m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Mouse Instance ID"), float4(0.5f, 0.0, 0.5f, 1.0f), CrRenderGraphPassType::Graphics,
		[&](CrRenderGraph& renderGraph)
		{
			renderGraph.BindDepthStencilTarget(m_depthStencilTexture.get(), CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, 0.0f);
			renderGraph.BindRenderTarget(m_debugShaderTexture.get(), CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, float4(1.0f, 1.0f, 1.0f, 1.0f));
		},
		[=](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
		{
			const ICrTexture* debugShaderTexture = m_debugShaderTexture.get();
	
			commandBuffer->SetViewport(CrViewport(0.0f, 0.0f, (float)debugShaderTexture->GetWidth(), (float)debugShaderTexture->GetHeight()));
			commandBuffer->SetScissor(CrRectangle(0, 0, debugShaderTexture->GetWidth(), debugShaderTexture->GetHeight()));
	
			const CrRenderList& mouseSelectionRenderList = m_renderWorld->GetRenderList(CrRenderListUsage::MouseSelection);
	
			// We don't batch here to get unique ids
			CrRenderPacketBatcher renderPacketBatcher(commandBuffer);
			renderPacketBatcher.SetMaximumBatchSize(1);

			mouseSelectionRenderList.ForEachRenderPacket([&](const CrRenderPacket& renderPacket)
			{
				renderPacketBatcher.FlushBatch();

				CrGPUBufferViewT<DebugShaderCB> debugShaderBuffer = commandBuffer->AllocateConstantBuffer<DebugShaderCB>();
				DebugShaderCB* debugShaderData = debugShaderBuffer.GetData();
				{
					uint32_t instanceId = (uint32_t)(uintptr_t)renderPacket.extra;
					debugShaderData->debugProperties = float4(0.0f, instanceId, 0.0f, 0.0f);
				}
				commandBuffer->BindConstantBuffer(debugShaderBuffer);

				renderPacketBatcher.ProcessRenderPacket(renderPacket);

				// TODO Fix dodgy behavior where we bind a constant buffer, bind another, then flush the batch
				// The problem happens when we modify per-instance data that the batcher knows nothing about
				renderPacketBatcher.FlushBatch();
			});

			renderPacketBatcher.FlushBatch(); // Execute last batch
		});
	
		const ICrComputePipeline* mouseSelectionResolvePipeline = m_mouseSelectionResolvePipeline.get();
		int32_t mouseX = mouseState.position.x;
		int32_t mouseY = mouseState.position.y;
	
		m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Resolve Selected Instance"), float4(0.5f, 0.0, 0.5f, 1.0f), CrRenderGraphPassType::Compute,
		[&](CrRenderGraph& renderGraph)
		{
			renderGraph.BindRWStorageBuffer(RWStorageBuffers::EditorSelectedInstanceID, m_mouseSelectionBuffer->GetHardwareBuffer(), cr3d::ShaderStageFlags::Compute);
			renderGraph.BindTexture(Textures::EditorInstanceIDTexture, m_debugShaderTexture.get(), cr3d::ShaderStageFlags::Compute);
		},
		[=](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
		{
			CrGPUBufferViewT<MouseSelectionCB> mouseSelectionBuffer = commandBuffer->AllocateConstantBuffer<MouseSelectionCB>(1);
			MouseSelectionCB* mouseSelectionData = mouseSelectionBuffer.GetData();
			{
				mouseSelectionData->mouseCoordinates.x = mouseX;
				mouseSelectionData->mouseCoordinates.y = mouseY;
			}
	
			commandBuffer->BindComputePipelineState(mouseSelectionResolvePipeline);
			commandBuffer->BindConstantBuffer(mouseSelectionBuffer);
			commandBuffer->BindTexture(Textures::EditorInstanceIDTexture, m_debugShaderTexture.get());
			commandBuffer->BindRWStorageBuffer(RWStorageBuffers::EditorSelectedInstanceID, m_mouseSelectionBuffer->GetHardwareBuffer());
			commandBuffer->Dispatch(1, 1, 1);
		});
	}

	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Draw Debug UI"), float4(), CrRenderGraphPassType::Behavior,
	[](CrRenderGraph&)
	{},
	[this](const CrRenderGraph&, ICrCommandBuffer*)
	{
		DrawDebugUI();
	});

	// Render ImGui
	ImGuiRenderer->AddRenderPass(m_mainRenderGraph, swapchainTexture);

	// Create a render pass that transitions the frame. We need to give the render graph visibility over what's going to happen with the texture, but not necessarily
	// execute the behavior inside as we may want to do further work before we end the command buffer
	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Present"), float4(), CrRenderGraphPassType::Behavior,
	[=](CrRenderGraph& renderGraph)
	{
		renderGraph.BindSwapchain(swapchainTexture.get());
	},
	[](const CrRenderGraph&, ICrCommandBuffer*)
	{

	});

	// Execute the render graph
	m_mainRenderGraph.Execute();

	// End the timing query tracker (inserts last timing query)
	m_timingQueryTracker->EndFrame(drawCommandBuffer);

	// End command buffer recording
	drawCommandBuffer->End();

	// Submit command buffer (should be reworked)
	drawCommandBuffer->Submit();

	// Download the mouse selection id
	if (m_renderWorld->GetMouseSelectionEnabled())
	{
		renderDevice->DownloadBuffer
		(
			m_mouseSelectionBuffer->GetHardwareBuffer(),
			[mouseState, keyboardState](const CrHardwareGPUBufferHandle& mouseIdBuffer)
			{
				uint32_t* mouseIdMemory = (uint32_t*)mouseIdBuffer->Lock();
				{
					SelectionState state;
					state.modelInstanceId = *mouseIdMemory;
					state.mouseState = mouseState;
					state.keyboardState = keyboardState;
					Editor->AddSelectionState(state);
				}
				mouseIdBuffer->Unlock();
			}
		);
	}

	ImGui::EndFrame();

	ImGui::UpdatePlatformWindows();

	// Present all available swapchains
	for (int i = 0; i < imguiPlatformIO.Viewports.Size; i++)
	{
		ImGuiViewport* imguiViewport = imguiPlatformIO.Viewports[i];
		ImGuiViewportsData* viewportData = (ImGuiViewportsData*)imguiViewport->PlatformUserData;
		CrOSWindow* osWindow = viewportData->osWindow;
		if (osWindow)
		{
			osWindow->GetSwapchain()->Present();
		}
	}

	m_currentCommandBuffer = (m_currentCommandBuffer + 1) % m_drawCmdBuffers.size();

	// Clears render lists
	m_renderWorld->EndRendering();

	// Clears all the resources of the render graph
	m_mainRenderGraph.End();

	// Processes deferred GPU commands
	renderDevice->ProcessQueuedCommands();

	// Processes deferred deletion of resources
	renderDevice->ProcessDeletionQueue();

	CrFrameTime::IncrementFrameCount();
}

struct CrSizeUnit
{
	uint64_t smallUnit;
	const char* unit;
};

CrSizeUnit GetGPUMemorySizeUnit(uint64_t bytes)
{
	if (bytes > 1024 * 1024 * 1024)
	{
		return { bytes / (1024 * 1024), "MB" };
	}
	else if (bytes > 1024 * 1024)
	{
		return { bytes / (1024), "KB" };
	}
	else
	{
		return { bytes, "bytes" };
	}
}

void CrFrame::DrawDebugUI()
{
	static bool s_DemoOpen = true;
	static bool s_ShowGeneralStatistics = true;
	static bool s_ShowWorldInfo = true;
	if (s_DemoOpen)
	{
		ImGui::ShowDemoWindow(&s_DemoOpen);
	}

	if (s_ShowGeneralStatistics)
	{
		if (ImGui::Begin("Statistics", &s_ShowGeneralStatistics))
		{
			crstl::time delta = CrFrameTime::GetFrameDelta();
			crstl::time averageDelta = CrFrameTime::GetFrameDeltaAverage();

			const CrRenderDeviceProperties& properties = RenderSystem->GetRenderDevice()->GetProperties();
			CrSizeUnit sizeUnit = GetGPUMemorySizeUnit(properties.gpuMemoryBytes);

			ImGui::Text("GPU: %s (%llu %s) (%s)", properties.description.c_str(), sizeUnit.smallUnit, sizeUnit.unit, properties.graphicsApiDisplay.c_str());
			ImGui::Text("Frame: %llu", CrFrameTime::GetFrameIndex());
			ImGui::Text("CPU Delta: [Instant] %.2f ms [Average] %.2fms [Max] %.2fms", delta.milliseconds(), averageDelta.milliseconds(), CrFrameTime::GetFrameDeltaMax().milliseconds());
			ImGui::Text("CPU FPS: [Instant] %.2f fps [Average] %.2f fps", delta.ticks_per_second(), averageDelta.ticks_per_second());
			ImGui::Text("Drawcalls: %d Instances: %d Vertices: %d", CrRenderingStatistics::GetDrawcallCount(), CrRenderingStatistics::GetInstanceCount(), CrRenderingStatistics::GetVertexCount());

			ImDrawList* drawList = ImGui::GetWindowDrawList();

			ImGuiTableFlags tableFlags = 0;
			tableFlags |= ImGuiTableFlags_Resizable;
			tableFlags |= ImGuiTableFlags_BordersOuter;
			tableFlags |= ImGuiTableFlags_BordersV;
			tableFlags |= ImGuiTableFlags_ScrollY;

			if (ImGui::BeginTable("##table1", 3, tableFlags))
			{
				// Set up header rows
				ImGui::TableSetupColumn("GPU Pass Name");
				ImGui::TableSetupColumn("Duration");
				ImGui::TableSetupColumn("Timeline");
				ImGui::TableHeadersRow();

				// Exit header row
				ImGui::TableNextRow();

				ImVec2 framePadding = ImGui::GetStyle().FramePadding;

				// Make sure we take padding into account when calculating initial positions
				ImGui::TableSetColumnIndex(2);
				ImVec2 timebarSize = ImVec2(ImGui::GetItemRectSize().x, ImGui::GetFrameHeight());
				ImVec2 initialTimebarPosition = ImGui::GetCursorScreenPos();
				initialTimebarPosition.x -= framePadding.x;
				initialTimebarPosition.y -= framePadding.y;

				CrGPUInterval totalFrameDuration = m_timingQueryTracker->GetFrameDuration();

				m_mainRenderGraph.ForEachPass([this, drawList, timebarSize, &initialTimebarPosition, totalFrameDuration](const CrRenderGraphPass& pass)
				{
					CrGPUInterval interval = m_timingQueryTracker->GetResultForFrame(CrHash(pass.name.c_str(), pass.name.length()));

					if (pass.type != CrRenderGraphPassType::Behavior)
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("%s", pass.name.c_str());

						ImGui::TableSetColumnIndex(1);
						ImGui::Text("%lfms", interval.durationNanoseconds / 1e6);

						ImGui::TableSetColumnIndex(2);
						
						{
							// Update timebar position
							initialTimebarPosition.y = ImGui::GetCursorScreenPos().y;
							initialTimebarPosition.y -= ImGui::GetStyle().FramePadding.y;

							float durationMillisecondsScaled = (float)(interval.durationNanoseconds / totalFrameDuration.durationNanoseconds);

							float durationPixels = durationMillisecondsScaled * timebarSize.x;
							ImVec2 finalPosition = ImVec2(initialTimebarPosition.x + durationPixels, initialTimebarPosition.y + timebarSize.y);

							int4 iColor = pass.color * 255.0f;
							ImU32 imColor = ImGui::GetColorU32(IM_COL32(iColor.x, iColor.y, iColor.z, 255));

							drawList->AddRectFilledMultiColor(initialTimebarPosition, finalPosition, imColor, imColor, imColor, imColor);

							initialTimebarPosition.x = finalPosition.x;
						}

						ImGui::TableNextRow();
					}
				});

				ImGui::EndTable();
			}

			ImGui::End();
		}
	}

	if (s_ShowWorldInfo)
	{
		if (ImGui::Begin("Rendering Debug", &s_ShowWorldInfo))
		{
			const float3& cameraPosition = m_camera->GetPosition();
			const float3& cameraForward = m_camera->GetForwardVector();

			ImGui::Text("Camera Position: (%.3f, %.3f, %.3f)", (float)cameraPosition.x, (float)cameraPosition.y, (float)cameraPosition.z);
			ImGui::Text("Camera Forward: (%.3f, %.3f, %.3f)", (float)cameraForward.x, (float)cameraForward.y, (float)cameraForward.z);

			ImGui::Separator();

			if (ImGui::BeginCombo("GBuffer Debug", GBufferDebugString[m_gbufferDebugMode], 0))
			{
				for (size_t n = 0; n < GBufferDebugMode::Count; n++)
				{
					bool isSelected = m_gbufferDebugMode == n;
					if (ImGui::Selectable(GBufferDebugString[n], isSelected))
					{
						m_gbufferDebugMode = (GBufferDebugMode::T)n;
					}

					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			ImGui::End();
		}
	}
}

void CrFrame::RecreateRenderTargets()
{
	CrRenderDeviceHandle renderDevice = RenderSystem->GetRenderDevice();

	// Ensure all operations on the device have been finished before destroying resources
	renderDevice->WaitIdle();

	m_swapchain = m_mainWindow->GetSwapchain();

	// Recreate depth stencil texture
	CrTextureDescriptor depthTextureDescriptor;
	depthTextureDescriptor.width = m_swapchain->GetWidth();
	depthTextureDescriptor.height = m_swapchain->GetHeight();
	depthTextureDescriptor.format = CrRendererConfig::DepthBufferFormat;
	depthTextureDescriptor.usage = cr3d::TextureUsage::DepthStencil;
	depthTextureDescriptor.name = "Depth Texture D32S8";

	m_depthStencilTexture = renderDevice->CreateTexture(depthTextureDescriptor);

	CrTextureDescriptor linearDepth16MinMaxMipChainTextureDescriptor;
	linearDepth16MinMaxMipChainTextureDescriptor.width = m_swapchain->GetWidth() >> 1;
	linearDepth16MinMaxMipChainTextureDescriptor.height = m_swapchain->GetHeight() >> 1;
	linearDepth16MinMaxMipChainTextureDescriptor.format = cr3d::DataFormat::RG16_Float;
	linearDepth16MinMaxMipChainTextureDescriptor.usage = cr3d::TextureUsage::UnorderedAccess;
	linearDepth16MinMaxMipChainTextureDescriptor.name = "Linear Depth 16 Min Max Mip Chain";
	linearDepth16MinMaxMipChainTextureDescriptor.mipmapCount = 4;

	m_linearDepth16MinMaxMipChain = renderDevice->CreateTexture(linearDepth16MinMaxMipChainTextureDescriptor);

	// Recreate render targets
	{
		CrTextureDescriptor preSwapchainDescriptor;
		preSwapchainDescriptor.width = m_swapchain->GetWidth();
		preSwapchainDescriptor.height = m_swapchain->GetHeight();
		preSwapchainDescriptor.format = m_swapchain->GetFormat();
		preSwapchainDescriptor.usage = cr3d::TextureUsage::RenderTarget | cr3d::TextureUsage::UnorderedAccess;
		preSwapchainDescriptor.name = "Pre Swapchain";
		m_preSwapchainTexture = renderDevice->CreateTexture(preSwapchainDescriptor);
	}

	{
		CrTextureDescriptor albedoAODescriptor;
		albedoAODescriptor.width = m_swapchain->GetWidth();
		albedoAODescriptor.height = m_swapchain->GetHeight();
		albedoAODescriptor.format = CrRendererConfig::GBufferAlbedoAOFormat;
		albedoAODescriptor.usage = cr3d::TextureUsage::RenderTarget;
		albedoAODescriptor.name = "GBuffer Albedo AO";
		m_gbufferAlbedoAOTexture = renderDevice->CreateTexture(albedoAODescriptor);
	}

	{
		CrTextureDescriptor normalsDescriptor;
		normalsDescriptor.width  = m_swapchain->GetWidth();
		normalsDescriptor.height = m_swapchain->GetHeight();
		normalsDescriptor.format = CrRendererConfig::GBufferNormalsFormat;
		normalsDescriptor.usage  = cr3d::TextureUsage::RenderTarget;
		normalsDescriptor.name   = "GBuffer Normals";
		m_gbufferNormalsTexture  = renderDevice->CreateTexture(normalsDescriptor);
	}

	{
		CrTextureDescriptor materialDescriptor;
		materialDescriptor.width  = m_swapchain->GetWidth();
		materialDescriptor.height = m_swapchain->GetHeight();
		materialDescriptor.format = CrRendererConfig::GBufferMaterialFormat;
		materialDescriptor.usage  = cr3d::TextureUsage::RenderTarget;
		materialDescriptor.name   = "GBuffer Material";
		m_gbufferMaterialTexture  = renderDevice->CreateTexture(materialDescriptor);
	}

	{
		CrTextureDescriptor lightingDescriptor;
		lightingDescriptor.width  = m_swapchain->GetWidth();
		lightingDescriptor.height = m_swapchain->GetHeight();
		lightingDescriptor.format = CrRendererConfig::LightingFormat;
		lightingDescriptor.usage  = cr3d::TextureUsage::RenderTarget;
		lightingDescriptor.name   = "Lighting HDR";
		m_lightingTexture = renderDevice->CreateTexture(lightingDescriptor);
	}

	{
		CrTextureDescriptor debugShaderDescriptor;
		debugShaderDescriptor.width  = m_swapchain->GetWidth();
		debugShaderDescriptor.height = m_swapchain->GetHeight();
		debugShaderDescriptor.format = CrRendererConfig::DebugShaderFormat;
		debugShaderDescriptor.usage  = cr3d::TextureUsage::RenderTarget;
		debugShaderDescriptor.name   = "Model Instance ID";
		m_debugShaderTexture         = renderDevice->CreateTexture(debugShaderDescriptor);
	}

	// Recreate command buffers
	m_drawCmdBuffers.resize(m_swapchain->GetImageCount());
	for (uint32_t i = 0; i < m_drawCmdBuffers.size(); ++i)
	{
		CrCommandBufferDescriptor descriptor;
		descriptor.dynamicBufferSizeBytes = 8 * 1024 * 1024; // 8 MB
		descriptor.dynamicVertexBufferSizeVertices = 1024 * 1024; // 1 million vertices
		descriptor.name.append_sprintf("Draw Command Buffer %i", i);
		m_drawCmdBuffers[i] = renderDevice->CreateCommandBuffer(descriptor);
	}

	m_timingQueryTracker = crstl::unique_ptr<CrGPUTimingQueryTracker>(new CrGPUTimingQueryTracker());
	m_timingQueryTracker->Initialize(renderDevice.get(), m_swapchain->GetImageCount());

	// Make sure all of this work is finished
	renderDevice->WaitIdle();
}

CrFrame::CrFrame()
{

}

CrFrame::~CrFrame()
{
}