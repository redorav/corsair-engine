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
#include "Rendering/CrGPUBuffer.h"
#include "Rendering/CrRenderPassDescriptor.h"
#include "Rendering/UI/CrImGuiRenderer.h"
#include "Rendering/ICrGPUQueryPool.h"
#include "Rendering/CrGPUTimingQueryTracker.h"

#include "Rendering/CrCamera.h"
#include "Rendering/CrRenderModel.h"
#include "Rendering/CrRenderMesh.h"
#include "Rendering/CrMaterialCompiler.h"
#include "Rendering/CrMaterial.h"
#include "Rendering/CrShaderSources.h"
#include "Rendering/CrVisibility.h"

#include "Rendering/CrRenderWorld.h"
#include "Rendering/CrRenderModelInstance.h"
#include "Rendering/CrCPUStackAllocator.h"

#include "Rendering/CrRenderGraph.h"

#include "Rendering/CrRendererConfig.h"

#include "Input/CrInputManager.h"

#include "Core/CrPlatform.h"
#include "Core/CrFrameTime.h"
#include "Core/CrGlobalPaths.h"

#include "CrResourceManager.h"

#include "imgui.h"

#include "Core/FileSystem/CrPath.h"

#include "Rendering/CrVertexDescriptor.h"
#include "Rendering/CrCommonVertexLayouts.h"

#include "GeneratedShaders/BuiltinShaders.h"

// TODO Put somewhere else
bool HashingAssert()
{
	CrGraphicsPipelineDescriptor defaultDescriptor;
	CrHash defaultDescriptorHash = CrHash(defaultDescriptor);
	CrAssertMsg(defaultDescriptorHash == CrHash(16322922871074920531), "Failed to hash known pipeline descriptor!");
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

	CrRenderDeviceSharedHandle renderDevice = ICrRenderSystem::GetRenderDevice();

	RecreateSwapchainAndRenderTargets();

	// TODO Move block to rendering subsystem initialization function
	{
		CrShaderSources::Get().Initialize();
		CrShaderManager::Get().Initialize(renderDevice.get());
		CrMaterialCompiler::Get().Initialize();
		CrPipelineStateManager::Get().Initialize(renderDevice.get());

		// Initialize ImGui renderer
		CrImGuiRendererInitParams imguiInitParams = {};
		imguiInitParams.m_swapchainFormat = m_swapchain->GetFormat();
		CrImGuiRenderer::Create(imguiInitParams);
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

	{
		CrSamplerDescriptor descriptor;
		descriptor.name = "Linear Clamp Sampler";
		m_linearClampSamplerHandle = renderDevice->CreateSampler(descriptor);
	}

	{
		CrSamplerDescriptor descriptor;
		descriptor.addressModeU = cr3d::AddressMode::Wrap;
		descriptor.addressModeV = cr3d::AddressMode::Wrap;
		descriptor.addressModeW = cr3d::AddressMode::Wrap;
		descriptor.name = "Linear Wrap Sampler";
		m_linearWrapSamplerHandle = renderDevice->CreateSampler(descriptor);
	}

	{
		CrSamplerDescriptor descriptor;
		descriptor.name = "Point Clamp Sampler";
		descriptor.magFilter = cr3d::Filter::Point;
		descriptor.minFilter = cr3d::Filter::Point;
		m_pointClampSamplerHandle = renderDevice->CreateSampler(descriptor);
	}

	{
		CrSamplerDescriptor descriptor;
		descriptor.name = "Linear Wrap Sampler";
		descriptor.magFilter = cr3d::Filter::Point;
		descriptor.minFilter = cr3d::Filter::Point;
		descriptor.addressModeU = cr3d::AddressMode::Wrap;
		descriptor.addressModeV = cr3d::AddressMode::Wrap;
		descriptor.addressModeW = cr3d::AddressMode::Wrap;
		m_pointWrapSamplerHandle = renderDevice->CreateSampler(descriptor);
	}

	CrTextureDescriptor rwTextureParams;
	rwTextureParams.width = 64;
	rwTextureParams.height = 64;
	rwTextureParams.format = cr3d::DataFormat::RGBA16_Unorm;
	rwTextureParams.usage = cr3d::TextureUsage::UnorderedAccess;
	rwTextureParams.name = "Colors RW Texture";
	m_colorsRWTexture = renderDevice->CreateTexture(rwTextureParams);

	m_colorsRWDataBuffer = renderDevice->CreateDataBuffer(cr3d::MemoryAccess::GPUOnlyWrite, cr3d::DataFormat::RGBA8_Unorm, 128);

	CrGraphicsPipelineDescriptor lineGraphicsPipelineDescriptor;
	lineGraphicsPipelineDescriptor.renderTargets.colorFormats[0] = m_swapchain->GetFormat();
	lineGraphicsPipelineDescriptor.renderTargets.depthFormat = m_depthStencilTexture->GetFormat();
	lineGraphicsPipelineDescriptor.primitiveTopology = cr3d::PrimitiveTopology::LineList;
	m_linePipeline = CrBuiltinGraphicsPipeline(renderDevice.get(), lineGraphicsPipelineDescriptor, SimpleVertexDescriptor, CrBuiltinShaders::BasicVS, CrBuiltinShaders::BasicPS);

	{
		CrComputePipelineDescriptor computePipelineDescriptor;
		m_computePipeline = CrBuiltinComputePipeline(renderDevice.get(), computePipelineDescriptor, CrBuiltinShaders::ExampleCompute);
	}

	{
		CrGraphicsPipelineDescriptor copyTextureGraphicsPipelineDescriptor;
		copyTextureGraphicsPipelineDescriptor.renderTargets.colorFormats[0] = cr3d::DataFormat::BGRA8_Unorm;
		copyTextureGraphicsPipelineDescriptor.depthStencilState.depthTestEnable = false;
		m_copyTexturePipeline = CrBuiltinGraphicsPipeline(renderDevice.get(), copyTextureGraphicsPipelineDescriptor, NullVertexDescriptor, CrBuiltinShaders::FullscreenTriangle, CrBuiltinShaders::CopyTextureColor);
	}

	uint8_t whiteTextureInitialData[4 * 4 * 4];
	memset(whiteTextureInitialData, 0xff, sizeof(whiteTextureInitialData));

	CrTextureDescriptor whiteTextureDescriptor;
	whiteTextureDescriptor.width = 4;
	whiteTextureDescriptor.height = 4;
	whiteTextureDescriptor.initialData = whiteTextureInitialData;
	whiteTextureDescriptor.initialDataSize = sizeof(whiteTextureInitialData);
	whiteTextureDescriptor.name = "Default White Texture";
	m_defaultWhiteTexture = renderDevice->CreateTexture(whiteTextureDescriptor);


	m_rwStructuredBuffer = renderDevice->CreateStructuredBuffer<ExampleRWStructuredBufferCompute>(cr3d::MemoryAccess::GPUOnlyWrite, 32);

	m_structuredBuffer = renderDevice->CreateStructuredBuffer<ExampleStructuredBufferCompute>(cr3d::MemoryAccess::GPUOnlyRead, 32);
	m_timingQueryTracker = CrUniquePtr<CrGPUTimingQueryTracker>(new CrGPUTimingQueryTracker());
	m_timingQueryTracker->Initialize(renderDevice.get(), m_swapchain->GetImageCount());
}

void CrFrame::Deinitialize()
{
	CrImGuiRenderer::Destroy();
}

void CrFrame::Process()
{
	const CrRenderDeviceSharedHandle& renderDevice = ICrRenderSystem::GetRenderDevice();

	CrSwapchainResult swapchainResult = m_swapchain->AcquireNextImage(UINT64_MAX);

	if (swapchainResult == CrSwapchainResult::Invalid)
	{
		RecreateSwapchainAndRenderTargets();
		swapchainResult = m_swapchain->AcquireNextImage(UINT64_MAX);
	}

	CrRenderingStatistics::Reset();

	ICrCommandBuffer* drawCommandBuffer = m_drawCmdBuffers[m_swapchain->GetCurrentFrameIndex()].get();

	CrImGuiRenderer::Get().NewFrame(m_swapchain->GetWidth(), m_swapchain->GetHeight());

	// Set up render graph to start recording passes
	CrRenderGraphFrameParams frameParams;
	frameParams.commandBuffer = drawCommandBuffer;
	frameParams.timingQueryTracker = m_timingQueryTracker.get();
	m_mainRenderGraph.Begin(frameParams);

	UpdateCamera();

	m_renderingStream->Reset();
	m_renderWorld->BeginRendering(m_renderingStream);

	m_renderWorld->SetCamera(m_camera);

	m_renderWorld->ComputeVisibilityAndRenderPackets();

	drawCommandBuffer->Begin();

	// TODO Find a good place to obtain default resources
	drawCommandBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::DiffuseTexture0, m_defaultWhiteTexture.get());
	drawCommandBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::NormalTexture0, m_defaultWhiteTexture.get());
	drawCommandBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::SpecularTexture0, m_defaultWhiteTexture.get());

	for (cr3d::ShaderStage::T shaderStage = cr3d::ShaderStage::Vertex; shaderStage < cr3d::ShaderStage::Count; ++shaderStage)
	{
		drawCommandBuffer->BindSampler(shaderStage, Samplers::AllLinearClampSampler, m_linearClampSamplerHandle.get());
		drawCommandBuffer->BindSampler(shaderStage, Samplers::AllLinearWrapSampler, m_linearWrapSamplerHandle.get());
		drawCommandBuffer->BindSampler(shaderStage, Samplers::AllPointClampSampler, m_pointClampSamplerHandle.get());
		drawCommandBuffer->BindSampler(shaderStage, Samplers::AllPointWrapSampler, m_pointWrapSamplerHandle.get());
	}

	m_timingQueryTracker->BeginFrame(drawCommandBuffer, CrFrameTime::GetFrameCount());

	CrRenderGraphTextureId depthTexture;
	CrRenderGraphTextureId gBufferAlbedoAO;
	CrRenderGraphTextureId gBufferNormals;
	CrRenderGraphTextureId gBufferMaterial;
	CrRenderGraphTextureId swapchainTexture;
	CrRenderGraphTextureId preSwapchainTexture;

	{
		CrRenderGraphTextureDescriptor depthDescriptor;
		depthDescriptor.texture = m_depthStencilTexture.get();
		depthTexture = m_mainRenderGraph.CreateTexture("Depth", depthDescriptor);

		CrRenderGraphTextureDescriptor swapchainDescriptor;
		swapchainDescriptor.texture = m_swapchain->GetTexture(m_swapchain->GetCurrentFrameIndex()).get();
		swapchainTexture = m_mainRenderGraph.CreateTexture("Swapchain", swapchainDescriptor);

		CrRenderGraphTextureDescriptor preSwapchainDescriptor;
		preSwapchainDescriptor.texture = m_preSwapchainTexture.get();
		preSwapchainTexture = m_mainRenderGraph.CreateTexture("Pre Swapchain", preSwapchainDescriptor);

		CrRenderGraphTextureDescriptor gBufferAlbedoDescriptor;
		gBufferAlbedoDescriptor.texture = m_gbufferAlbedoAOTexture.get();
		gBufferAlbedoAO = m_mainRenderGraph.CreateTexture("GBuffer Albedo AO", gBufferAlbedoDescriptor);

		CrRenderGraphTextureDescriptor gBufferNormalsDescriptor;
		gBufferNormalsDescriptor.texture = m_gbufferNormalsTexture.get();
		gBufferNormals = m_mainRenderGraph.CreateTexture("GBuffer Normals", gBufferNormalsDescriptor);

		CrRenderGraphTextureDescriptor gBufferMaterialDescriptor;
		gBufferMaterialDescriptor.texture = m_gbufferMaterialTexture.get();
		gBufferMaterial = m_mainRenderGraph.CreateTexture("GBuffer Material", gBufferMaterialDescriptor);
	}

	m_mainRenderGraph.AddRenderPass("GBuffer Render Pass", float4(160.0f / 255.05f, 180.0f / 255.05f, 150.0f / 255.05f, 1.0f), CrRenderGraphPassType::Graphics,
	[=](CrRenderGraph& renderGraph)
	{
		renderGraph.AddDepthStencilTarget(depthTexture, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, 0.0f);
		renderGraph.AddRenderTarget(gBufferAlbedoAO, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, float4(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f));
		renderGraph.AddRenderTarget(gBufferNormals, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, float4(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f));
		renderGraph.AddRenderTarget(gBufferMaterial, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, float4(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f));
	},
	[this, gBufferAlbedoAO](const CrRenderGraph& renderGraph, ICrCommandBuffer* commandBuffer)
	{
		ICrTexture* gBufferAlbedoAOTexture = renderGraph.GetPhysicalTexture(gBufferAlbedoAO);

		commandBuffer->SetViewport(CrViewport(0.0f, 0.0f, (float)gBufferAlbedoAOTexture->GetWidth(), (float)gBufferAlbedoAOTexture->GetHeight()));
		commandBuffer->SetScissor(CrScissor(0, 0, gBufferAlbedoAOTexture->GetWidth(), gBufferAlbedoAOTexture->GetHeight()));

		CrGPUBufferType<Color> colorBuffer = commandBuffer->AllocateConstantBuffer<Color>();
		Color* theColorData2 = colorBuffer.Lock();
		{
			theColorData2->color = float4(1.0f, 1.0f, 1.0f, 1.0f);
			theColorData2->tint2 = float4(1.0f, 1.0f, 1.0f, 1.0f);
		}
		colorBuffer.Unlock();
		commandBuffer->BindConstantBuffer(&colorBuffer);

		CrGPUBufferType<Camera> cameraDataBuffer = commandBuffer->AllocateConstantBuffer<Camera>();
		Camera* cameraData2 = cameraDataBuffer.Lock();
		{
			*cameraData2 = m_cameraConstantData;
		}
		cameraDataBuffer.Unlock();
		commandBuffer->BindConstantBuffer(&cameraDataBuffer);

		CrGPUBufferType<Instance> identityConstantBuffer = commandBuffer->AllocateConstantBuffer<Instance>();
		Instance* identityTransformData = identityConstantBuffer.Lock();
		{
			identityTransformData->local2World[0] = float4x4::identity();
		}
		identityConstantBuffer.Unlock();

		const CrRenderList& gBufferRenderList = m_renderWorld->GetRenderList(CrRenderListUsage::GBuffer);

		CrRenderPacketBatcher renderPacketBatcher(commandBuffer);

		gBufferRenderList.ForEachRenderPacket([&](const CrRenderPacket& renderPacket)
		{
			renderPacketBatcher.ProcessRenderPacket(renderPacket);
		});

		// Execute the last batch
		renderPacketBatcher.ExecuteBatch();
	});

	m_mainRenderGraph.AddRenderPass("GBuffer Lighting Pass", float4(160.0f / 255.05f, 180.0f / 255.05f, 150.0f / 255.05f, 1.0f), CrRenderGraphPassType::Graphics,
	[=](CrRenderGraph& renderGraph)
	{
		renderGraph.AddDepthStencilTarget(depthTexture, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, 0.0f);
		renderGraph.AddTexture(gBufferAlbedoAO, cr3d::ShaderStageFlags::Pixel);
	},
	[this](const CrRenderGraph& /*renderGraph*/, ICrCommandBuffer* /*commandBuffer*/)
	{
	});

	m_mainRenderGraph.AddRenderPass("Main Render Pass", float4(160.0f / 255.05f, 180.0f / 255.05f, 150.0f / 255.05f, 1.0f), CrRenderGraphPassType::Graphics,
	[=](CrRenderGraph& renderGraph)
	{
		renderGraph.AddDepthStencilTarget(depthTexture, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, 0.0f);
		renderGraph.AddRenderTarget(preSwapchainTexture, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, float4(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f));
	},
	[this](const CrRenderGraph&, ICrCommandBuffer* commandBuffer)
	{
		commandBuffer->SetViewport(CrViewport(0.0f, 0.0f, (float)m_swapchain->GetWidth(), (float)m_swapchain->GetHeight()));
		commandBuffer->SetScissor(CrScissor(0, 0, m_swapchain->GetWidth(), m_swapchain->GetHeight()));

		CrGPUBufferType<Color> colorBuffer = commandBuffer->AllocateConstantBuffer<Color>();
		Color* theColorData2 = colorBuffer.Lock();
		{
			theColorData2->color = float4(1.0f, 1.0f, 1.0f, 1.0f);
			theColorData2->tint2 = float4(1.0f, 1.0f, 1.0f, 1.0f);
		}
		colorBuffer.Unlock();
		commandBuffer->BindConstantBuffer(&colorBuffer);

		CrGPUBufferType<Camera> cameraDataBuffer = commandBuffer->AllocateConstantBuffer<Camera>();
		Camera* cameraData2 = cameraDataBuffer.Lock();
		{
			*cameraData2 = m_cameraConstantData;
		}
		cameraDataBuffer.Unlock();
		commandBuffer->BindConstantBuffer(&cameraDataBuffer);

		CrGPUBufferType<Instance> identityConstantBuffer = commandBuffer->AllocateConstantBuffer<Instance>();
		Instance* identityTransformData = identityConstantBuffer.Lock();
		{
			identityTransformData->local2World[0] = float4x4::identity();
		}
		identityConstantBuffer.Unlock();

		const CrRenderList& forwardRenderList = m_renderWorld->GetRenderList(CrRenderListUsage::Forward);

		CrRenderPacketBatcher renderPacketBatcher(commandBuffer);

		forwardRenderList.ForEachRenderPacket([&](const CrRenderPacket& renderPacket)
		{
			renderPacketBatcher.ProcessRenderPacket(renderPacket);
		});

		// Execute the last batch
		renderPacketBatcher.ExecuteBatch();
	});

	m_mainRenderGraph.AddRenderPass("Copy Pass", float4(1.0f, 0.0f, 1.0f, 1.0f), CrRenderGraphPassType::Graphics,
	[=](CrRenderGraph& renderGraph)
	{
		renderGraph.AddTexture(preSwapchainTexture, cr3d::ShaderStageFlags::Pixel);
		renderGraph.AddRenderTarget(swapchainTexture);
	},
	[this, preSwapchainTexture](const CrRenderGraph& renderGraph, ICrCommandBuffer* commandBuffer)
	{
		commandBuffer->BindTexture(cr3d::ShaderStage::Pixel, Textures::CopyTexture, renderGraph.GetPhysicalTexture(preSwapchainTexture));
		commandBuffer->BindGraphicsPipelineState(m_copyTexturePipeline.get());
		commandBuffer->Draw(3, 1, 0, 0);
	});

	CrRenderGraphBufferDescriptor structuredBufferDescriptor;
	structuredBufferDescriptor.buffer = m_structuredBuffer.get();
	CrRenderGraphBufferId structuredBuffer = m_mainRenderGraph.CreateBuffer("Structured Buffer", structuredBufferDescriptor);

	CrRenderGraphBufferDescriptor rwStructuredBufferDescriptor;
	rwStructuredBufferDescriptor.buffer = m_rwStructuredBuffer.get();
	CrRenderGraphBufferId rwStructuredBuffer = m_mainRenderGraph.CreateBuffer("RW Structured Buffer", rwStructuredBufferDescriptor);

	CrRenderGraphBufferDescriptor colorsRWDataBufferDescriptor;
	colorsRWDataBufferDescriptor.buffer = m_colorsRWDataBuffer.get();
	CrRenderGraphBufferId colorsRWDataBuffer = m_mainRenderGraph.CreateBuffer("Colors RW Data Buffer", colorsRWDataBufferDescriptor);

	CrRenderGraphTextureDescriptor colorsRWTextureDescriptor;
	colorsRWTextureDescriptor.texture = m_colorsRWTexture.get();
	CrRenderGraphTextureId colorsRWTexture = m_mainRenderGraph.CreateTexture("Colors RW Texture", colorsRWTextureDescriptor);

	const ICrComputePipeline* computePipeline = m_computePipeline.get();

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
		commandBuffer->BindComputePipelineState(computePipeline);
		commandBuffer->BindStorageBuffer(cr3d::ShaderStage::Compute, StorageBuffers::ExampleStructuredBufferCompute, renderGraph.GetPhysicalBuffer(structuredBuffer));
		commandBuffer->BindRWStorageBuffer(cr3d::ShaderStage::Compute, RWStorageBuffers::ExampleRWStructuredBufferCompute, renderGraph.GetPhysicalBuffer(rwStructuredBuffer));
		commandBuffer->BindRWDataBuffer(cr3d::ShaderStage::Compute, RWDataBuffers::ExampleRWDataBufferCompute, renderGraph.GetPhysicalBuffer(colorsRWDataBuffer));
		commandBuffer->BindRWTexture(cr3d::ShaderStage::Compute, RWTextures::ExampleRWTextureCompute, renderGraph.GetPhysicalTexture(colorsRWTexture), 0);
		commandBuffer->Dispatch(1, 1, 1);
	});

	CrRenderGraphTextureDescriptor debugShaderTextureDescriptor;
	debugShaderTextureDescriptor.texture = m_debugShaderTexture.get();
	CrRenderGraphTextureId debugShaderTextureId = m_mainRenderGraph.CreateTexture("Debug Shader Texture", debugShaderTextureDescriptor);

	m_mainRenderGraph.AddRenderPass("Mouse Instance ID", float4(0.5f, 0.0, 0.5f, 1.0f), CrRenderGraphPassType::Graphics,
	[&](CrRenderGraph& renderGraph)
	{
		renderGraph.AddDepthStencilTarget(depthTexture, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, 0.0f);
		renderGraph.AddRenderTarget(debugShaderTextureId, CrRenderTargetLoadOp::Clear, CrRenderTargetStoreOp::Store, float4(100.0f / 255.0f, 149.0f / 255.0f, 237.0f / 255.0f, 1.0f));
	},
	[=](const CrRenderGraph& renderGraph, ICrCommandBuffer* commandBuffer)
	{
		const ICrTexture* debugShaderTexture = renderGraph.GetPhysicalTexture(debugShaderTextureId);

		commandBuffer->SetViewport(CrViewport(0.0f, 0.0f, (float)debugShaderTexture->GetWidth(), (float)debugShaderTexture->GetHeight()));
		commandBuffer->SetScissor(CrScissor(0, 0, debugShaderTexture->GetWidth(), debugShaderTexture->GetHeight()));

		const CrRenderList& mouseSelectionRenderList = m_renderWorld->GetRenderList(CrRenderListUsage::MouseSelection);

		// We don't batch here to get unique ids
		CrRenderPacketBatcher renderPacketBatcher(commandBuffer);
		renderPacketBatcher.SetMaximumBatchSize(1);

		mouseSelectionRenderList.ForEachRenderPacket([&](const CrRenderPacket& renderPacket)
		{
			CrGPUBufferType<DebugShader> debugShaderBuffer = commandBuffer->AllocateConstantBuffer<DebugShader>();
			DebugShader* debugShaderData = debugShaderBuffer.Lock();
			{
				debugShaderData->debugProperties = float4(0.0f, ((uint32_t*)renderPacket.extra)[0], 0.0f, 0.0f);
			}
			debugShaderBuffer.Unlock();
			commandBuffer->BindConstantBuffer(&debugShaderBuffer);

			renderPacketBatcher.ProcessRenderPacket(renderPacket);
		});

		// Execute last batch
		renderPacketBatcher.ExecuteBatch();
	});

	m_mainRenderGraph.AddRenderPass("Draw Debug UI", float4(), CrRenderGraphPassType::Behavior,
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
	m_mainRenderGraph.AddRenderPass("Present", float4(), CrRenderGraphPassType::Behavior,
	[=](CrRenderGraph& renderGraph)
	{
		renderGraph.AddSwapchain(swapchainTexture);
	},
	[](const CrRenderGraph&, ICrCommandBuffer*)
	{

	});

	m_mainRenderGraph.Execute();

	m_timingQueryTracker->EndFrame(drawCommandBuffer);

	drawCommandBuffer->End();

	drawCommandBuffer->Submit();

	m_swapchain->Present();

	m_renderWorld->EndRendering();

	m_mainRenderGraph.End();

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

			ImGui::Text("GPU: %s (%llu %s) (%s)", properties.description.c_str(), sizeUnit.smallUnit, sizeUnit.unit, cr3d::GraphicsApi::ToString(properties.graphicsApi));
			ImGui::Text("Frame: %i", CrFrameTime::GetFrameCount());
			ImGui::Text("Delta: [Instant] %.2f ms [Average] %.2fms [Max] %.2fms", delta.AsMilliseconds(), averageDelta.AsMilliseconds(), CrFrameTime::GetFrameDeltaMax().AsMilliseconds());
			ImGui::Text("FPS: [Instant] %.2f fps [Average] %.2f fps", delta.AsFPS(), averageDelta.AsFPS());
			ImGui::Text("Drawcalls: %i Vertices: %i", CrRenderingStatistics::GetDrawcallCount(), CrRenderingStatistics::GetVertexCount());

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

				// Make sure we take padding into account when calculating initial positions
				ImGui::TableSetColumnIndex(2);
				ImVec2 timebarSize = ImVec2(ImGui::GetItemRectSize().x, ImGui::GetFrameHeight());
				ImVec2 initialTimebarPosition = ImGui::GetCursorScreenPos();
				initialTimebarPosition.x -= ImGui::GetStyle().FramePadding.x;
				initialTimebarPosition.y -= ImGui::GetStyle().FramePadding.y;

				CrGPUInterval totalFrameDuration = m_timingQueryTracker->GetFrameDuration();

				m_mainRenderGraph.ForEachPass([this, drawList, timebarSize, &initialTimebarPosition, totalFrameDuration](const CrRenderGraphPass& pass)
				{
					CrGPUInterval interval = m_timingQueryTracker->GetResultForFrame(CrHash(pass.name.c_str(), pass.name.length()));

					if (pass.type != CrRenderGraphPassType::Behavior)
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::Text(pass.name.c_str());

						ImGui::TableSetColumnIndex(1);
						ImGui::Text("%lfms", interval.durationNanoseconds / 1e6);

						ImGui::TableSetColumnIndex(2);
						
						{
							initialTimebarPosition.y = ImGui::GetCursorScreenPos().y;
							initialTimebarPosition.y -= ImGui::GetStyle().FramePadding.y;

							float durationMillisecondsScaled = CrClamp((float)(interval.durationNanoseconds / totalFrameDuration.durationNanoseconds), 0.0f, 1.0f);

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

void CrFrame::RecreateSwapchainAndRenderTargets()
{
	CrRenderDeviceSharedHandle renderDevice = ICrRenderSystem::GetRenderDevice();

	// Ensure all operations on the device have been finished before destroying resources
	renderDevice->WaitIdle();

	// 1. Destroy the old swapchain before creating the new one. Otherwise the API will fail trying to create a resource
	// that becomes available after (once the pointer assignment happens and the resource is destroyed). Right after, create
	// the new swapchain
	m_swapchain = nullptr;

	CrSwapchainDescriptor swapchainDescriptor = {};
	swapchainDescriptor.name = "Main Swapchain";
	swapchainDescriptor.platformWindow = m_platformWindow;
	swapchainDescriptor.platformHandle = m_platformHandle;
	swapchainDescriptor.requestedWidth = m_width;
	swapchainDescriptor.requestedHeight = m_height;
	swapchainDescriptor.format = CrRendererConfig::SwapchainFormat;
	swapchainDescriptor.requestedBufferCount = 3;
	m_swapchain = renderDevice->CreateSwapchain(swapchainDescriptor);

	// 2. Recreate depth stencil texture
	CrTextureDescriptor depthTextureDescriptor;
	depthTextureDescriptor.width = m_swapchain->GetWidth();
	depthTextureDescriptor.height = m_swapchain->GetHeight();
	depthTextureDescriptor.format = CrRendererConfig::DepthBufferFormat;
	depthTextureDescriptor.usage = cr3d::TextureUsage::DepthStencil;
	depthTextureDescriptor.name = "Depth Texture D32S8";

	m_depthStencilTexture = renderDevice->CreateTexture(depthTextureDescriptor); // Create the depth buffer

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
		CrTextureDescriptor instanceIDDescriptor;
		instanceIDDescriptor.width  = m_swapchain->GetWidth();
		instanceIDDescriptor.height = m_swapchain->GetHeight();
		instanceIDDescriptor.format = CrRendererConfig::DebugShaderFormat;
		instanceIDDescriptor.usage  = cr3d::TextureUsage::RenderTarget;
		instanceIDDescriptor.name   = "Model Instance ID";
		m_debugShaderTexture        = renderDevice->CreateTexture(instanceIDDescriptor);
	}

	// 4. Recreate command buffers
	m_drawCmdBuffers.resize(m_swapchain->GetImageCount());
	for (uint32_t i = 0; i < m_drawCmdBuffers.size(); ++i)
	{
		m_drawCmdBuffers[i] = renderDevice->CreateCommandBuffer(CrCommandQueueType::Graphics);
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