#include "CrFrame.h"

#include "Rendering/ICrRenderSystem.h"
#include "Rendering/ICrRenderDevice.h"
#include "Rendering/ICrSwapchain.h"
#include "Rendering/CrShaderManager.h"
#include "Rendering/ICrShader.h"
#include "Rendering/CrPipelineStateManager.h"
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
#include "Rendering/RenderWorld/CrRenderModelInstance.h"

#include "Rendering/CrRenderGraph.h"

#include "Rendering/CrRendererConfig.h"
#include "Rendering/CrRenderingResources.h"

#include "Rendering/CrCommonVertexLayouts.h"

#include "Core/Input/CrInputManager.h"
#include "Core/CrPlatform.h"
#include "Core/CrFrameTime.h"
#include "Core/CrGlobalPaths.h"

#include "Resource/CrResourceManager.h"

#include <imgui.h>

#include "GeneratedShaders/BuiltinShaders.h"

#include "Math/CrMath.h"
#include "Math/CrHalf.h"

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
			renderPacket.renderMesh != m_renderMesh;

		bool noMoreSpace = (m_numInstances + renderPacket.numInstances) > m_maxBatchSize;

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
			CrGPUBufferViewT<Instance> transformBuffer = m_commandBuffer->AllocateConstantBuffer<Instance>(sizeof(Instance::local2World[0]) * m_numInstances);
			cr3d::float4x4* transforms = (cr3d::float4x4*)transformBuffer.GetData();
			{
				for (uint32_t i = 0; i < m_numInstances; ++i)
				{
					transforms[i] = *m_matrices[i];
				}
			}
			m_commandBuffer->BindConstantBuffer(cr3d::ShaderStage::Vertex, transformBuffer);

			m_commandBuffer->BindGraphicsPipelineState(m_pipeline);

			for (uint32_t t = 0; t < m_material->m_textures.size(); ++t)
			{
				CrMaterial::TextureBinding binding = m_material->m_textures[t];
				m_commandBuffer->BindTexture(cr3d::ShaderStage::Pixel, binding.semantic, binding.texture.get());
			}

			for (uint32_t streamIndex = 0; streamIndex < m_renderMesh->GetVertexBufferCount(); ++streamIndex)
			{
				m_commandBuffer->BindVertexBuffer(m_renderMesh->GetVertexBuffer(streamIndex).get(), streamIndex);
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

	uint32_t m_maxBatchSize = 0;

	// Has to match the maximum number of matrices declared in the shader
	CrArray<float4x4*, sizeof_array(Instance::local2World)> m_matrices;
};

void CrFrame::Initialize(void* platformHandle, void* platformWindow, uint32_t width, uint32_t height)
{
	HashingAssert();

	m_platformHandle = platformHandle;
	m_platformWindow = platformWindow;

	m_width = width;
	m_height = height;

	CrRenderDeviceHandle renderDevice = ICrRenderSystem::GetRenderDevice();

	RecreateSwapchainAndRenderTargets(m_width, m_height);

	// TODO Move block to rendering subsystem initialization function
	{

		CrShaderSources::Get().Initialize();
		CrShaderManager::Get().Initialize(renderDevice.get());
		CrMaterialCompiler::Get().Initialize();
		CrPipelineStateManager::Get().Initialize(renderDevice.get());

		CrBuiltinPipelines::Initialize();

		// Initialize ImGui renderer
		CrImGuiRendererInitParams imguiInitParams = {};
		imguiInitParams.m_swapchainFormat = m_swapchain->GetFormat();
		CrImGuiRenderer::Create(imguiInitParams);
	}

	// Create the rendering scratch. Start with 10MB
	m_renderingStream = CrIntrusivePtr<CrCPUStackAllocator>(new CrCPUStackAllocator());
	m_renderingStream->Initialize(10 * 1024 * 1024);

	// Create a render world
	m_renderWorld = CrMakeShared<CrRenderWorld>();

	bool loadData = true;

	if (loadData)
	{
		CrRenderModelHandle nyraModel = CrResourceManager::LoadModel(CrResourceManager::GetFullResourcePath("nyra/nyra_pose_mod.fbx"));
		CrRenderModelHandle jainaModel = CrResourceManager::LoadModel(CrResourceManager::GetFullResourcePath("jaina/storm_hero_jaina.fbx"));
		CrRenderModelHandle damagedHelmet = CrResourceManager::LoadModel(CrResourceManager::GetFullResourcePath("gltf-helmet/DamagedHelmet.gltf"));

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

			m_renderWorld->SetRenderModel(modelInstance.GetId(), renderModel);
		}
	}

	m_camera = CrSharedPtr<CrCamera>(new CrCamera());

	CrTextureDescriptor rwTextureParams;
	rwTextureParams.width = 64;
	rwTextureParams.height = 64;
	rwTextureParams.format = cr3d::DataFormat::RGBA16_Unorm;
	rwTextureParams.usage = cr3d::TextureUsage::UnorderedAccess;
	rwTextureParams.name = "Colors RW Texture";
	m_colorsRWTexture = renderDevice->CreateTexture(rwTextureParams);

	m_colorsRWDataBuffer = renderDevice->CreateDataBuffer(cr3d::MemoryAccess::GPUOnlyWrite, cr3d::DataFormat::RGBA8_Unorm, 128);

	m_exampleComputePipeline = CrBuiltinPipelines::GetComputePipeline(CrBuiltinShaders::ExampleCompute);
	m_depthDownsampleLinearize = CrBuiltinPipelines::GetComputePipeline(CrBuiltinShaders::DepthDownsampleLinearizeMinMax);
	m_mouseSelectionResolvePipeline = CrBuiltinPipelines::GetComputePipeline(CrBuiltinShaders::EditorMouseSelectionResolveCS);

	{
		CrGraphicsPipelineDescriptor copyTextureGraphicsPipelineDescriptor;
		copyTextureGraphicsPipelineDescriptor.renderTargets.colorFormats[0] = cr3d::DataFormat::BGRA8_Unorm;
		copyTextureGraphicsPipelineDescriptor.depthStencilState.depthTestEnable = false;
		m_copyTexturePipeline = CrBuiltinPipelines::GetGraphicsPipeline(copyTextureGraphicsPipelineDescriptor, NullVertexDescriptor, CrBuiltinShaders::FullscreenTriangle, CrBuiltinShaders::CopyTextureColor);
	}

	{
		m_createIndirectArguments = CrBuiltinPipelines::GetComputePipeline(CrBuiltinShaders::CreateIndirectArguments);
	}

	{
		CrGraphicsPipelineDescriptor directionalLightPipelineDescriptor;
		directionalLightPipelineDescriptor.renderTargets.colorFormats[0] = CrRendererConfig::LightingFormat;
		directionalLightPipelineDescriptor.depthStencilState.depthTestEnable = false;
		m_directionalLightPipeline = CrBuiltinPipelines::GetGraphicsPipeline(directionalLightPipelineDescriptor, NullVertexDescriptor, CrBuiltinShaders::FullscreenTriangle, CrBuiltinShaders::DirectionalLightPS);
	}

	{
		CrGraphicsPipelineDescriptor editorEdgeSelectionPipelineDescriptor;
		editorEdgeSelectionPipelineDescriptor.renderTargets.colorFormats[0] = cr3d::DataFormat::BGRA8_Unorm;
		editorEdgeSelectionPipelineDescriptor.depthStencilState.depthTestEnable = false;
		editorEdgeSelectionPipelineDescriptor.blendState.renderTargetBlends[0].enable = true;
		editorEdgeSelectionPipelineDescriptor.blendState.renderTargetBlends[0].colorBlendOp = cr3d::BlendOp::Add;
		m_editorEdgeSelectionPipeline = CrBuiltinPipelines::GetGraphicsPipeline(editorEdgeSelectionPipelineDescriptor, NullVertexDescriptor, CrBuiltinShaders::FullscreenTriangle, CrBuiltinShaders::EditorEdgeSelectionPS);
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

	m_timingQueryTracker = CrUniquePtr<CrGPUTimingQueryTracker>(new CrGPUTimingQueryTracker());
	m_timingQueryTracker->Initialize(renderDevice.get(), m_swapchain->GetImageCount());
}

void CrFrame::Deinitialize()
{
	CrImGuiRenderer::Destroy();
}

struct SelectionPacket
{
	uint32_t entityId = 0xffffffff;
	bool isLeftShiftClicked = false;
};

SelectionPacket CurrentSelectionData;

void CrFrame::Process()
{
	const CrRenderDeviceHandle& renderDevice = ICrRenderSystem::GetRenderDevice();

	// Swapchain resizes are not immediately processed but deferred to the main loop. That way we have control over where it happens
	if (m_requestSwapchainResize)
	{
		RecreateSwapchainAndRenderTargets(m_swapchainResizeRequestWidth, m_swapchainResizeRequestHeight);
		m_width = m_swapchainResizeRequestWidth;
		m_height = m_swapchainResizeRequestHeight;
		m_requestSwapchainResize = false;
	}

	// 2. Rendering

	CrSwapchainResult swapchainResult = m_swapchain->AcquireNextImage(UINT64_MAX);

	CrAssertMsg(swapchainResult != CrSwapchainResult::Invalid, "Was unable to acquire next swapchain image");

	renderDevice->ProcessQueuedCommands();

	CrRenderingStatistics::Reset();

	ICrCommandBuffer* drawCommandBuffer = m_drawCmdBuffers[m_swapchain->GetCurrentFrameIndex()].get();

	CrImGuiRenderer::Get().NewFrame(m_swapchain->GetWidth(), m_swapchain->GetHeight());

	const MouseState& mouseState = CrInput.GetMouseState();
	bool isMouseClicked = mouseState.buttonClicked[MouseButton::Left];
	
	const KeyboardState& keyboardState = CrInput.GetKeyboardState();
	bool isEscapeClicked = keyboardState.keyHeld[KeyboardKey::Escape];
	bool isLeftShiftClicked = keyboardState.keyHeld[KeyboardKey::LeftShift];

	if (keyboardState.keyHeld[KeyboardKey::LeftCtrl] &&
		keyboardState.keyHeld[KeyboardKey::LeftShift] &&
		keyboardState.keyPressed[KeyboardKey::F5])
	{
		CrBuiltinPipelines::RecompileComputePipelines();
	}

	if (isEscapeClicked)
	{
		m_renderWorld->ClearSelection();
		CurrentSelectionData.entityId = 0xffffffff;
	}
	
	if (CurrentSelectionData.entityId != 0xffffffff)
	{
		if (CurrentSelectionData.entityId == 65535)
		{
			m_renderWorld->ClearSelection();
		}
		else
		{
			if (CurrentSelectionData.isLeftShiftClicked)
			{
				m_renderWorld->ToggleSelected(CrModelInstanceId(CurrentSelectionData.entityId));
			}
			else
			{
				m_renderWorld->SetSelected(CrModelInstanceId(CurrentSelectionData.entityId));
			}
		}

		CurrentSelectionData = SelectionPacket();
	}

	CrRectangle mouseRectangle(mouseState.position.x - 10, mouseState.position.y - 10, 20, 20);
	m_renderWorld->SetMouseSelectionEnabled(isMouseClicked, mouseRectangle);

	// Set up render graph to start recording passes
	CrRenderGraphFrameParams frameRenderGraphParams;
	frameRenderGraphParams.commandBuffer = drawCommandBuffer;
	frameRenderGraphParams.timingQueryTracker = m_timingQueryTracker.get();
	frameRenderGraphParams.frameIndex = CrFrameTime::GetFrameIndex();
	m_mainRenderGraph.Begin(frameRenderGraphParams);

	m_renderingStream->Reset();

	m_renderWorld->BeginRendering(m_renderingStream);

	m_renderWorld->SetCamera(m_camera);

	m_renderWorld->ComputeVisibilityAndRenderPackets();

	drawCommandBuffer->Begin();

	drawCommandBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::DiffuseTexture0, CrRenderingResources::Get().WhiteSmallTexture.get());
	drawCommandBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::NormalTexture0, CrRenderingResources::Get().NormalsSmallTexture.get());
	drawCommandBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::SpecularTexture0, CrRenderingResources::Get().WhiteSmallTexture.get());

	for (cr3d::ShaderStage::T shaderStage = cr3d::ShaderStage::Vertex; shaderStage < cr3d::ShaderStage::Count; ++shaderStage)
	{
		drawCommandBuffer->BindSampler(shaderStage, Samplers::AllLinearClampSampler, CrRenderingResources::Get().AllLinearClampSampler.get());
		drawCommandBuffer->BindSampler(shaderStage, Samplers::AllLinearWrapSampler, CrRenderingResources::Get().AllLinearWrapSampler.get());
		drawCommandBuffer->BindSampler(shaderStage, Samplers::AllPointClampSampler, CrRenderingResources::Get().AllPointClampSampler.get());
		drawCommandBuffer->BindSampler(shaderStage, Samplers::AllPointWrapSampler, CrRenderingResources::Get().AllPointWrapSampler.get());
	}

	// Set up default values for common constant buffers

	CrGPUBufferViewT<Color> colorBuffer = drawCommandBuffer->AllocateConstantBuffer<Color>();
	Color* colorData2 = colorBuffer.GetData();
	{
		colorData2->color = float4(1.0f, 1.0f, 1.0f, 1.0f);
		colorData2->tint = float4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	drawCommandBuffer->BindConstantBuffer(cr3d::ShaderStage::Pixel, colorBuffer);

	m_cameraConstantData.world2View = m_camera->GetWorld2ViewMatrix();
	m_cameraConstantData.view2Projection = m_camera->GetView2ProjectionMatrix();
	m_cameraConstantData.projectionParams = m_camera->ComputeProjectionParams();

	CrGPUBufferViewT<Camera> cameraDataBuffer = drawCommandBuffer->AllocateConstantBuffer<Camera>();
	Camera* cameraData = cameraDataBuffer.GetData();
	{
		*cameraData = m_cameraConstantData;
	}
	drawCommandBuffer->BindConstantBuffer(cr3d::ShaderStage::Vertex, cameraDataBuffer);
	drawCommandBuffer->BindConstantBuffer(cr3d::ShaderStage::Pixel, cameraDataBuffer);
	drawCommandBuffer->BindConstantBuffer(cr3d::ShaderStage::Compute, cameraDataBuffer);

	CrGPUBufferViewT<Instance> identityConstantBuffer = drawCommandBuffer->AllocateConstantBuffer<Instance>();
	Instance* identityTransformData = identityConstantBuffer.GetData();
	{
		identityTransformData->local2World[0] = float4x4::identity();
	}

	m_timingQueryTracker->BeginFrame(drawCommandBuffer, CrFrameTime::GetFrameIndex());

	CrRenderGraphTextureDescriptor depthDescriptor(m_depthStencilTexture.get());
	CrRenderGraphTextureId depthTexture = m_mainRenderGraph.CreateTexture(CrRenderGraphString("Depth"), depthDescriptor);

	CrRenderGraphTextureDescriptor swapchainDescriptor(m_swapchain->GetTexture(m_swapchain->GetCurrentFrameIndex()).get());
	CrRenderGraphTextureId swapchainTexture = m_mainRenderGraph.CreateTexture(CrRenderGraphString("Swapchain"), swapchainDescriptor);

	CrRenderGraphTextureDescriptor preSwapchainDescriptor(m_preSwapchainTexture.get());
	CrRenderGraphTextureId preSwapchainTexture = m_mainRenderGraph.CreateTexture(CrRenderGraphString("Pre Swapchain"), preSwapchainDescriptor);

	CrRenderGraphTextureDescriptor gBufferAlbedoDescriptor(m_gbufferAlbedoAOTexture.get());
	CrRenderGraphTextureId gBufferAlbedoAO = m_mainRenderGraph.CreateTexture(CrRenderGraphString("GBuffer Albedo AO"), gBufferAlbedoDescriptor);

	CrRenderGraphTextureDescriptor gBufferNormalsDescriptor(m_gbufferNormalsTexture.get());
	CrRenderGraphTextureId gBufferNormals = m_mainRenderGraph.CreateTexture(CrRenderGraphString("GBuffer Normals"), gBufferNormalsDescriptor);

	CrRenderGraphTextureDescriptor gBufferMaterialDescriptor(m_gbufferMaterialTexture.get());
	CrRenderGraphTextureId gBufferMaterial = m_mainRenderGraph.CreateTexture(CrRenderGraphString("GBuffer Material"), gBufferMaterialDescriptor);

	CrRenderGraphTextureDescriptor lightingDescriptor(m_lightingTexture.get());
	CrRenderGraphTextureId lightingTexture = m_mainRenderGraph.CreateTexture(CrRenderGraphString("Lighting HDR"), lightingDescriptor);

	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("GBuffer Render Pass"), float4(160.0f / 255.05f, 180.0f / 255.05f, 150.0f / 255.05f, 1.0f), CrRenderGraphPassType::Graphics,
	[=](CrRenderGraph& renderGraph)
	{
		renderGraph.AddDepthStencilTarget(depthTexture, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, 0.0f);
		renderGraph.AddRenderTarget(gBufferAlbedoAO, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, float4(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f));
		renderGraph.AddRenderTarget(gBufferNormals, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, float4(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f));
		renderGraph.AddRenderTarget(gBufferMaterial, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, float4(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f));
	},
	[this, gBufferAlbedoAO](const CrRenderGraph& renderGraph, ICrCommandBuffer* commandBuffer)
	{
		const ICrTexture* gBufferAlbedoAOTexture = renderGraph.GetPhysicalTexture(gBufferAlbedoAO);

		commandBuffer->SetViewport(CrViewport(0.0f, 0.0f, (float)gBufferAlbedoAOTexture->GetWidth(), (float)gBufferAlbedoAOTexture->GetHeight()));
		commandBuffer->SetScissor(CrRectangle(0, 0, gBufferAlbedoAOTexture->GetWidth(), gBufferAlbedoAOTexture->GetHeight()));

		const CrRenderList& gBufferRenderList = m_renderWorld->GetRenderList(CrRenderListUsage::GBuffer);

		CrRenderPacketBatcher renderPacketBatcher(commandBuffer);

		gBufferRenderList.ForEachRenderPacket([&](const CrRenderPacket& renderPacket)
		{
			renderPacketBatcher.ProcessRenderPacket(renderPacket);
		});

		renderPacketBatcher.ExecuteBatch(); // Execute the last batch
	});

	CrRenderGraphTextureDescriptor linearDepthMipChainDescriptor(m_linearDepth16MinMaxMipChain.get());
	CrRenderGraphTextureId linearDepth16MinMaxMipChainId = m_mainRenderGraph.CreateTexture(CrRenderGraphString("Linear Depth 16 Mip Chain"), linearDepthMipChainDescriptor);

	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Depth Downsample Linearize"), float4(160.0f / 255.05f, 180.0f / 255.05f, 150.0f / 255.05f, 1.0f), CrRenderGraphPassType::Compute,
	[=](CrRenderGraph& renderGraph)
	{
		renderGraph.AddTexture(depthTexture, cr3d::ShaderStageFlags::Compute);
		renderGraph.AddRWTexture(linearDepth16MinMaxMipChainId, cr3d::ShaderStageFlags::Compute);
	},
	[this, depthTexture, linearDepth16MinMaxMipChainId](const CrRenderGraph& renderGraph, ICrCommandBuffer* commandBuffer)
	{
		const ICrTexture* rawDepthTexture = renderGraph.GetPhysicalTexture(depthTexture);
		const ICrTexture* linearDepth16MipChainTexture = renderGraph.GetPhysicalTexture(linearDepth16MinMaxMipChainId);
		
		commandBuffer->BindComputePipelineState(m_depthDownsampleLinearize->GetPipeline());
		commandBuffer->BindTexture(cr3d::ShaderStage::Compute, Textures::RawDepthTexture, rawDepthTexture, cr3d::TexturePlane::Depth);
		commandBuffer->BindTexture(cr3d::ShaderStage::Compute, Textures::StencilTexture, rawDepthTexture, cr3d::TexturePlane::Stencil);
		commandBuffer->BindRWTexture(cr3d::ShaderStage::Compute, RWTextures::RWLinearDepthMinMaxMip1, linearDepth16MipChainTexture, 0);
		commandBuffer->BindRWTexture(cr3d::ShaderStage::Compute, RWTextures::RWLinearDepthMinMaxMip2, linearDepth16MipChainTexture, 1);
		commandBuffer->BindRWTexture(cr3d::ShaderStage::Compute, RWTextures::RWLinearDepthMinMaxMip3, linearDepth16MipChainTexture, 2);
		commandBuffer->BindRWTexture(cr3d::ShaderStage::Compute, RWTextures::RWLinearDepthMinMaxMip4, linearDepth16MipChainTexture, 3);
		commandBuffer->Dispatch
		(
			(linearDepth16MipChainTexture->GetWidth() + 7) / 8,
			(linearDepth16MipChainTexture->GetHeight() + 7) / 8,
			1
		);
	});

	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("GBuffer Lighting Pass"), float4(160.0f / 255.05f, 180.0f / 255.05f, 150.0f / 255.05f, 1.0f), CrRenderGraphPassType::Graphics,
	[=](CrRenderGraph& renderGraph)
	{
		renderGraph.AddRenderTarget(lightingTexture, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, float4(0.0f));
		renderGraph.AddTexture(gBufferAlbedoAO, cr3d::ShaderStageFlags::Pixel);
		renderGraph.AddTexture(gBufferNormals, cr3d::ShaderStageFlags::Pixel);
		renderGraph.AddTexture(gBufferMaterial, cr3d::ShaderStageFlags::Pixel);
	},
	[this, gBufferAlbedoAO, gBufferNormals, gBufferMaterial]
	(const CrRenderGraph& renderGraph, ICrCommandBuffer* commandBuffer)
	{
		CrGPUBufferViewT<DynamicLightCB> lightConstantBuffer = commandBuffer->AllocateConstantBuffer<DynamicLightCB>();
		DynamicLightCB* lightData = lightConstantBuffer.GetData();
		{
			lightData->positionRadius = float4(0.0f, 1.0f, 0.0f, 1.0f);
			lightData->radiance = float4(1.0f, 1.0f, 1.0f, 0.0f);
		}

		commandBuffer->BindConstantBuffer(cr3d::ShaderStage::Pixel, lightConstantBuffer);
		commandBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::GBufferAlbedoAOTexture, renderGraph.GetPhysicalTexture(gBufferAlbedoAO));
		commandBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::GBufferNormalsTexture, renderGraph.GetPhysicalTexture(gBufferNormals));
		commandBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::GBufferMaterialTexture, renderGraph.GetPhysicalTexture(gBufferMaterial));
		commandBuffer->BindGraphicsPipelineState(m_directionalLightPipeline->GetPipeline());
		commandBuffer->Draw(3, 1, 0, 0);
	});

	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Main Render Pass"), float4(160.0f / 255.05f, 180.0f / 255.05f, 150.0f / 255.05f, 1.0f), CrRenderGraphPassType::Graphics,
	[=](CrRenderGraph& renderGraph)
	{
		renderGraph.AddDepthStencilTarget(depthTexture, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, 0.0f);
		renderGraph.AddRenderTarget(preSwapchainTexture, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, float4(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f));
	},
	[this](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
	{
		commandBuffer->SetViewport(CrViewport(0.0f, 0.0f, (float)m_swapchain->GetWidth(), (float)m_swapchain->GetHeight()));
		commandBuffer->SetScissor(CrRectangle(0, 0, m_swapchain->GetWidth(), m_swapchain->GetHeight()));

		const CrRenderList& forwardRenderList = m_renderWorld->GetRenderList(CrRenderListUsage::Forward);

		CrRenderPacketBatcher renderPacketBatcher(commandBuffer);

		forwardRenderList.ForEachRenderPacket([&](const CrRenderPacket& renderPacket)
		{
			renderPacketBatcher.ProcessRenderPacket(renderPacket);
		});

		renderPacketBatcher.ExecuteBatch(); // Execute the last batch
	});

	CrRenderGraphTextureDescriptor debugShaderTextureDescriptor(m_debugShaderTexture.get());
	CrRenderGraphTextureId debugShaderTextureId = m_mainRenderGraph.CreateTexture(CrRenderGraphString("Debug Shader Texture"), debugShaderTextureDescriptor);

	CrRenderGraphBufferDescriptor mouseSelectionBufferDescriptor(m_mouseSelectionBuffer->GetHardwareBuffer());
	CrRenderGraphBufferId mouseSelectionBufferId = m_mainRenderGraph.CreateBuffer(CrRenderGraphString("Mouse Selection Buffer"), mouseSelectionBufferDescriptor);

	const CrRenderList& edgeSelectionRenderList = m_renderWorld->GetRenderList(CrRenderListUsage::EdgeSelection);

	if (edgeSelectionRenderList.Size() > 0)
	{
		m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Edge Selection Render"), float4(160.0f / 255.05f, 180.0f / 255.05f, 150.0f / 255.05f, 1.0f), CrRenderGraphPassType::Graphics,
		[depthTexture, debugShaderTextureId](CrRenderGraph& renderGraph)
		{
			renderGraph.AddDepthStencilTarget(depthTexture, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, 0.0f);
			renderGraph.AddRenderTarget(debugShaderTextureId, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, float4(0.0f, 0.0f, 0.0f, 0.0f));
		},
		[this, edgeSelectionRenderList](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
		{
			commandBuffer->SetViewport(CrViewport(0.0f, 0.0f, (float)m_swapchain->GetWidth(), (float)m_swapchain->GetHeight()));
			commandBuffer->SetScissor(CrRectangle(0, 0, m_swapchain->GetWidth(), m_swapchain->GetHeight()));

			CrRenderPacketBatcher renderPacketBatcher(commandBuffer);

			CrGPUBufferViewT<DebugShader> debugShaderBuffer = commandBuffer->AllocateConstantBuffer<DebugShader>();
			DebugShader* debugShaderData = debugShaderBuffer.GetData();
			{
				debugShaderData->debugProperties = float4(1.0f, 0.0f, 0.0f, 0.0f);
			}
			commandBuffer->BindConstantBuffer(cr3d::ShaderStage::Vertex, debugShaderBuffer);
			commandBuffer->BindConstantBuffer(cr3d::ShaderStage::Pixel, debugShaderBuffer);

			edgeSelectionRenderList.ForEachRenderPacket([&](const CrRenderPacket& renderPacket)
			{
				renderPacketBatcher.ProcessRenderPacket(renderPacket);
			});

			renderPacketBatcher.ExecuteBatch(); // Execute the last batch
		});

		m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Edge Selection Resolve"), float4(160.0f / 255.05f, 180.0f / 255.05f, 150.0f / 255.05f, 1.0f), CrRenderGraphPassType::Graphics,
		[=](CrRenderGraph& renderGraph)
		{
			renderGraph.AddRenderTarget(preSwapchainTexture, CrRenderTargetLoadOp::Load, CrRenderTargetStoreOp::Store, float4(0.0f));
			renderGraph.AddTexture(debugShaderTextureId, cr3d::ShaderStageFlags::Pixel);
		},
		[this, debugShaderTextureId]
		(const CrRenderGraph& renderGraph, ICrCommandBuffer* commandBuffer)
		{
			commandBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::EditorSelectionTexture, renderGraph.GetPhysicalTexture(debugShaderTextureId));
			commandBuffer->BindGraphicsPipelineState(m_editorEdgeSelectionPipeline->GetPipeline());
			commandBuffer->Draw(3, 1, 0, 0);
		});
	}

	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Copy Pass"), float4(1.0f, 0.0f, 1.0f, 1.0f), CrRenderGraphPassType::Graphics,
	[=](CrRenderGraph& renderGraph)
	{
		renderGraph.AddTexture(preSwapchainTexture, cr3d::ShaderStageFlags::Pixel);
		renderGraph.AddRenderTarget(swapchainTexture, CrRenderTargetLoadOp::DontCare, CrRenderTargetStoreOp::Store, float4(0.0f));
	},
	[this, preSwapchainTexture](const CrRenderGraph& renderGraph, ICrCommandBuffer* commandBuffer)
	{
		commandBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::CopyTexture, renderGraph.GetPhysicalTexture(preSwapchainTexture));
		commandBuffer->BindGraphicsPipelineState(m_copyTexturePipeline->GetPipeline());
		commandBuffer->Draw(3, 1, 0, 0);
	});

	CrRenderGraphBufferDescriptor structuredBufferDescriptor(m_structuredBuffer->GetHardwareBuffer());
	CrRenderGraphBufferId structuredBuffer = m_mainRenderGraph.CreateBuffer(CrRenderGraphString("Structured Buffer"), structuredBufferDescriptor);

	CrRenderGraphBufferDescriptor rwStructuredBufferDescriptor(m_rwStructuredBuffer->GetHardwareBuffer());
	CrRenderGraphBufferId rwStructuredBuffer = m_mainRenderGraph.CreateBuffer(CrRenderGraphString("RW Structured Buffer"), rwStructuredBufferDescriptor);

	CrRenderGraphBufferDescriptor colorsRWDataBufferDescriptor(m_colorsRWDataBuffer->GetHardwareBuffer());
	CrRenderGraphBufferId colorsRWDataBuffer = m_mainRenderGraph.CreateBuffer(CrRenderGraphString("Colors RW Data Buffer"), colorsRWDataBufferDescriptor);

	CrRenderGraphTextureDescriptor colorsRWTextureDescriptor(m_colorsRWTexture.get());
	CrRenderGraphTextureId colorsRWTexture = m_mainRenderGraph.CreateTexture(CrRenderGraphString("Colors RW Texture"), colorsRWTextureDescriptor);

	const ICrComputePipeline* exampleComputePipeline = m_exampleComputePipeline->GetPipeline();

	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Compute 1"), float4(0.0f, 0.0, 1.0f, 1.0f), CrRenderGraphPassType::Compute,
	[&](CrRenderGraph& renderGraph)
	{
		renderGraph.AddBuffer(structuredBuffer, cr3d::ShaderStageFlags::Compute);
		renderGraph.AddRWBuffer(rwStructuredBuffer, cr3d::ShaderStageFlags::Compute);
		renderGraph.AddRWBuffer(colorsRWDataBuffer, cr3d::ShaderStageFlags::Compute);
		renderGraph.AddRWTexture(colorsRWTexture, cr3d::ShaderStageFlags::Compute, 0, 1, 0, 1);
	},
	[=](const CrRenderGraph& renderGraph, ICrCommandBuffer* commandBuffer)
	{
		commandBuffer->BindComputePipelineState(exampleComputePipeline);
		commandBuffer->BindStorageBuffer(cr3d::ShaderStage::Compute, StorageBuffers::ExampleStructuredBufferCompute, renderGraph.GetPhysicalBuffer(structuredBuffer));
		commandBuffer->BindRWStorageBuffer(cr3d::ShaderStage::Compute, RWStorageBuffers::ExampleRWStructuredBufferCompute, renderGraph.GetPhysicalBuffer(rwStructuredBuffer));
		commandBuffer->BindRWDataBuffer(cr3d::ShaderStage::Compute, RWDataBuffers::ExampleRWDataBufferCompute, renderGraph.GetPhysicalBuffer(colorsRWDataBuffer));
		commandBuffer->BindRWTexture(cr3d::ShaderStage::Compute, RWTextures::ExampleRWTextureCompute, renderGraph.GetPhysicalTexture(colorsRWTexture), 0);
		commandBuffer->BindTexture(cr3d::ShaderStage::Compute, Textures::ExampleTexture3DCompute, m_colorfulVolumeTexture.get());
		commandBuffer->BindTexture(cr3d::ShaderStage::Compute, Textures::ExampleTextureArrayCompute, m_colorfulTextureArray.get());
		commandBuffer->Dispatch(1, 1, 1);
	});

	CrRenderGraphBufferDescriptor indirectArgumentsBufferDescriptor(m_indirectDispatchArguments->GetHardwareBuffer());
	CrRenderGraphBufferId indirectArgumentsBuffer = m_mainRenderGraph.CreateBuffer(CrRenderGraphString("Indirect Arguments"), indirectArgumentsBufferDescriptor);

	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Indirect Arguments Create"), float4(0.0f, 0.0, 1.0f, 1.0f), CrRenderGraphPassType::Compute,
	[&](CrRenderGraph& renderGraph)
	{
		renderGraph.AddRWBuffer(indirectArgumentsBuffer, cr3d::ShaderStageFlags::Compute);
	},
	[=](const CrRenderGraph& renderGraph, ICrCommandBuffer* commandBuffer)
	{
		commandBuffer->BindComputePipelineState(m_createIndirectArguments->GetPipeline());
		commandBuffer->BindRWStorageBuffer(cr3d::ShaderStage::Compute, RWStorageBuffers::ExampleIndirectBuffer, renderGraph.GetPhysicalBuffer(indirectArgumentsBuffer));
		commandBuffer->Dispatch(1, 1, 1);
	});
	
	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Indirect Arguments Consume"), float4(0.0f, 0.0, 1.0f, 1.0f), CrRenderGraphPassType::Compute,
	[&](CrRenderGraph& renderGraph)
	{
		renderGraph.AddBuffer(structuredBuffer, cr3d::ShaderStageFlags::Compute);
		renderGraph.AddRWBuffer(rwStructuredBuffer, cr3d::ShaderStageFlags::Compute);
		renderGraph.AddRWBuffer(colorsRWDataBuffer, cr3d::ShaderStageFlags::Compute);
		renderGraph.AddRWTexture(colorsRWTexture, cr3d::ShaderStageFlags::Compute, 0, 1, 0, 1);
	},
	[=](const CrRenderGraph& renderGraph, ICrCommandBuffer* commandBuffer)
	{
		commandBuffer->BindComputePipelineState(exampleComputePipeline);
		commandBuffer->BindStorageBuffer(cr3d::ShaderStage::Compute, StorageBuffers::ExampleStructuredBufferCompute, renderGraph.GetPhysicalBuffer(structuredBuffer));
		commandBuffer->BindRWStorageBuffer(cr3d::ShaderStage::Compute, RWStorageBuffers::ExampleRWStructuredBufferCompute, renderGraph.GetPhysicalBuffer(rwStructuredBuffer));
		commandBuffer->BindRWDataBuffer(cr3d::ShaderStage::Compute, RWDataBuffers::ExampleRWDataBufferCompute, renderGraph.GetPhysicalBuffer(colorsRWDataBuffer));
		commandBuffer->BindRWTexture(cr3d::ShaderStage::Compute, RWTextures::ExampleRWTextureCompute, renderGraph.GetPhysicalTexture(colorsRWTexture), 0);
		commandBuffer->BindTexture(cr3d::ShaderStage::Compute, Textures::ExampleTexture3DCompute, m_colorfulVolumeTexture.get());
		commandBuffer->BindTexture(cr3d::ShaderStage::Compute, Textures::ExampleTextureArrayCompute, m_colorfulTextureArray.get());
	
		const ICrHardwareGPUBuffer* indirectBuffer = renderGraph.GetPhysicalBuffer(indirectArgumentsBuffer);
		commandBuffer->DispatchIndirect(indirectBuffer, 0);
	});

	if (m_renderWorld->GetMouseSelectionEnabled())
	{
		m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Mouse Instance ID"), float4(0.5f, 0.0, 0.5f, 1.0f), CrRenderGraphPassType::Graphics,
		[&](CrRenderGraph& renderGraph)
		{
			renderGraph.AddDepthStencilTarget(depthTexture, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, 0.0f);
			renderGraph.AddRenderTarget(debugShaderTextureId, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, float4(1.0f, 1.0f, 1.0f, 1.0f));
		},
		[=](const CrRenderGraph& renderGraph, ICrCommandBuffer* commandBuffer)
		{
			const ICrTexture* debugShaderTexture = renderGraph.GetPhysicalTexture(debugShaderTextureId);

			commandBuffer->SetViewport(CrViewport(0.0f, 0.0f, (float)debugShaderTexture->GetWidth(), (float)debugShaderTexture->GetHeight()));
			commandBuffer->SetScissor(CrRectangle(0, 0, debugShaderTexture->GetWidth(), debugShaderTexture->GetHeight()));

			const CrRenderList& mouseSelectionRenderList = m_renderWorld->GetRenderList(CrRenderListUsage::MouseSelection);

			// We don't batch here to get unique ids
			CrRenderPacketBatcher renderPacketBatcher(commandBuffer);
			renderPacketBatcher.SetMaximumBatchSize(1);

			mouseSelectionRenderList.ForEachRenderPacket([&](const CrRenderPacket& renderPacket)
			{
				CrGPUBufferViewT<DebugShader> debugShaderBuffer = commandBuffer->AllocateConstantBuffer<DebugShader>();
				DebugShader* debugShaderData = debugShaderBuffer.GetData();
				{
					uint32_t instanceId = (uint32_t)(uintptr_t)renderPacket.extra;
					debugShaderData->debugProperties = float4(0.0f, instanceId, 0.0f, 0.0f);
				}
				commandBuffer->BindConstantBuffer(cr3d::ShaderStage::Pixel, debugShaderBuffer);

				renderPacketBatcher.ProcessRenderPacket(renderPacket);
				renderPacketBatcher.ExecuteBatch(); // TODO Fix dodgy behavior
			});

			renderPacketBatcher.ExecuteBatch(); // Execute last batch
		});

		const ICrComputePipeline* mouseSelectionResolvePipeline = m_mouseSelectionResolvePipeline->GetPipeline();
		int32_t mouseX = mouseState.position.x;
		int32_t mouseY = mouseState.position.y;

		m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Resolve Selected Instance"), float4(0.5f, 0.0, 0.5f, 1.0f), CrRenderGraphPassType::Compute,
		[&](CrRenderGraph& renderGraph)
		{
			renderGraph.AddRWBuffer(mouseSelectionBufferId, cr3d::ShaderStageFlags::Compute);
			renderGraph.AddTexture(debugShaderTextureId, cr3d::ShaderStageFlags::Compute);
		},
		[=](const CrRenderGraph& renderGraph, ICrCommandBuffer* commandBuffer)
		{
			CrGPUBufferViewT<MouseSelection> mouseSelectionBuffer = commandBuffer->AllocateConstantBuffer<MouseSelection>(1);
			MouseSelection* mouseSelectionData = mouseSelectionBuffer.GetData();
			{
				mouseSelectionData->mouseCoordinates.x = mouseX;
				mouseSelectionData->mouseCoordinates.y = mouseY;
			}

			commandBuffer->BindComputePipelineState(mouseSelectionResolvePipeline);
			commandBuffer->BindConstantBuffer(cr3d::ShaderStage::Compute, mouseSelectionBuffer);
			commandBuffer->BindTexture(cr3d::ShaderStage::Compute, Textures::EditorInstanceIDTexture, renderGraph.GetPhysicalTexture(debugShaderTextureId));
			commandBuffer->BindRWStorageBuffer(cr3d::ShaderStage::Compute, RWStorageBuffers::EditorSelectedInstanceID, renderGraph.GetPhysicalBuffer(mouseSelectionBufferId));
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
	CrImGuiRenderer::Get().Render(m_mainRenderGraph, swapchainTexture);

	// Create a render pass that transitions the frame. We need to give the render graph
	// visibility over what's going to happen with the texture, but not necessarily
	// execute the behavior inside as we may want to do further work before we end the 
	// command buffer
	m_mainRenderGraph.AddRenderPass(CrRenderGraphString("Present"), float4(), CrRenderGraphPassType::Behavior,
	[=](CrRenderGraph& renderGraph)
	{
		renderGraph.AddSwapchain(swapchainTexture);
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
	if(isMouseClicked)
	{
		renderDevice->DownloadBuffer
		(
			m_mouseSelectionBuffer->GetHardwareBuffer(),
			[isLeftShiftClicked](const CrHardwareGPUBufferHandle& mouseIdBuffer)
			{
				uint32_t* mouseIdMemory = (uint32_t*)mouseIdBuffer->Lock();
				{
					CurrentSelectionData.entityId = *mouseIdMemory;
					CurrentSelectionData.isLeftShiftClicked = isLeftShiftClicked;
				}
				mouseIdBuffer->Unlock();
			}
		);
	}

	// Present the swapchain
	m_swapchain->Present();

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
			CrSizeUnit sizeUnit = GetGPUMemorySizeUnit(properties.gpuMemoryBytes);

			ImGui::Text("GPU: %s (%llu %s) (%s)", properties.description.c_str(), sizeUnit.smallUnit, sizeUnit.unit, properties.graphicsApiDisplay.c_str());
			ImGui::Text("Frame: %llu", CrFrameTime::GetFrameIndex());
			ImGui::Text("Delta: [Instant] %.2f ms [Average] %.2fms [Max] %.2fms", delta.AsMilliseconds(), averageDelta.AsMilliseconds(), CrFrameTime::GetFrameDeltaMax().AsMilliseconds());
			ImGui::Text("FPS: [Instant] %.2f fps [Average] %.2f fps", delta.AsFPS(), averageDelta.AsFPS());
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
}

void CrFrame::HandleWindowResize(uint32_t width, uint32_t height)
{
	m_requestSwapchainResize = true;
	m_swapchainResizeRequestWidth = width;
	m_swapchainResizeRequestHeight = height;
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

	if (keyboard.keyHeld[KeyboardKey::LeftShift])
	{
		translationSpeed *= 10.0f;
	}

	// TODO Hack to get a bit of movement on the camera
	if (keyboard.keyHeld[KeyboardKey::A] || gamepadState.axes[GamepadAxis::LeftX] < 0.0f)
	{
		m_camera->Translate(currentRight * -translationSpeed * frameDelta);
	}
	
	if (keyboard.keyHeld[KeyboardKey::D] || gamepadState.axes[GamepadAxis::LeftX] > 0.0f)
	{
		m_camera->Translate(currentRight * translationSpeed * frameDelta);
	}
	
	if (keyboard.keyHeld[KeyboardKey::W] || gamepadState.axes[GamepadAxis::LeftY] > 0.0f)
	{
		m_camera->Translate(currentLookAt * translationSpeed * frameDelta);
	}
	
	if (keyboard.keyHeld[KeyboardKey::S] || gamepadState.axes[GamepadAxis::LeftY] < 0.0f)
	{
		m_camera->Translate(currentLookAt * -translationSpeed * frameDelta);
	}
	
	if (keyboard.keyHeld[KeyboardKey::Q] || gamepadState.axes[GamepadAxis::LeftTrigger] > 0.0f)
	{
		m_camera->Translate(float3(0.0f, -translationSpeed, 0.0f) * frameDelta);
	}
	
	if (keyboard.keyHeld[KeyboardKey::E] || gamepadState.axes[GamepadAxis::RightTrigger] > 0.0f)
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

	if (mouseState.buttonHeld[MouseButton::Right])
	{
		m_camera->Rotate(float3(mouseState.relativePosition.y, mouseState.relativePosition.x, 0.0f) * frameDelta);
	}

	m_camera->Update();

	m_cameraConstantData.world2View = m_camera->GetWorld2ViewMatrix();
	m_cameraConstantData.view2Projection = m_camera->GetView2ProjectionMatrix();
}

void CrFrame::RecreateSwapchainAndRenderTargets(uint32_t width, uint32_t height)
{
	CrRenderDeviceHandle renderDevice = ICrRenderSystem::GetRenderDevice();

	// Ensure all operations on the device have been finished before destroying resources
	renderDevice->WaitIdle();

	// Resize swapchain if it already exists, create if null
	if (m_swapchain)
	{
		m_swapchain->Resize(width, height);
	}
	else
	{
		CrSwapchainDescriptor swapchainDescriptor = {};
		swapchainDescriptor.name = "Main Swapchain";
		swapchainDescriptor.platformWindow = m_platformWindow;
		swapchainDescriptor.platformHandle = m_platformHandle;
		swapchainDescriptor.requestedWidth = width;
		swapchainDescriptor.requestedHeight = height;
		swapchainDescriptor.format = CrRendererConfig::SwapchainFormat;
		swapchainDescriptor.requestedBufferCount = 3;
		m_swapchain = renderDevice->CreateSwapchain(swapchainDescriptor);
	}

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
		preSwapchainDescriptor.usage = cr3d::TextureUsage::RenderTarget;
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
		descriptor.dynamicConstantBufferSizeBytes = 8 * 1024 * 1024; // 8 MB
		descriptor.dynamicVertexBufferSizeVertices = 1024 * 1024; // 1 million vertices
		descriptor.name.append_sprintf("Draw Command Buffer %i", i);
		m_drawCmdBuffers[i] = renderDevice->CreateCommandBuffer(descriptor);
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