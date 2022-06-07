#pragma once

#include "Core/Containers/CrVector.h"
#include "Core/SmartPointers/CrSharedPtr.h"

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrRenderWorld.h"
#include "Rendering/CrRenderGraph.h"
#include "Rendering/CrBuiltinPipeline.h"

#include "GeneratedShaders/ShaderMetadata.h"

class CrRenderModel;
using CrRenderModelSharedHandle = CrSharedPtr<CrRenderModel>;

class CrRenderWorld;
using CrRenderWorldSharedHandle = CrSharedPtr<CrRenderWorld>;

class CrFrame
{
public:

	CrFrame();

	~CrFrame();

	void Initialize(void* platformHandle, void* platformWindow, uint32_t width, uint32_t height);

	void Deinitialize();

	void Process();

	void DrawDebugUI();

	void UpdateCamera();

	void RecreateSwapchainAndRenderTargets();

private:

	CrVector<CrCommandBufferSharedHandle> m_drawCmdBuffers; // Command buffers used for rendering
	
	uint32_t m_width = 0;

	uint32_t m_height = 0;

	// TODO Temporary
	CrBuiltinGraphicsPipeline m_line2DPipeline;
	CrBuiltinGraphicsPipeline m_line3DPipeline;

	CrBuiltinComputePipeline m_exampleComputePipeline;
	CrBuiltinGraphicsPipeline m_copyTexturePipeline;

	CrBuiltinGraphicsPipeline m_directionalLightPipeline;

	CrBuiltinComputePipeline m_createIndirectArguments;

	CrTextureSharedHandle m_colorfulVolumeTexture;

	CrTextureSharedHandle m_colorfulTextureArray;

	// Editor Shaders
	CrBuiltinGraphicsPipeline m_editorEdgeSelectionPipeline;

	CrSwapchainSharedHandle m_swapchain;

	CrTextureSharedHandle m_depthStencilTexture;
	CrTextureSharedHandle m_preSwapchainTexture;

	CrTextureSharedHandle m_gbufferAlbedoAOTexture;
	CrTextureSharedHandle m_gbufferNormalsTexture;
	CrTextureSharedHandle m_gbufferMaterialTexture;

	CrTextureSharedHandle m_lightingTexture;

	CrTextureSharedHandle m_debugShaderTexture;

	CrTextureSharedHandle m_colorsRWTexture;

	CrStructuredBufferSharedHandle<ExampleRWStructuredBufferCompute> m_rwStructuredBuffer;

	CrStructuredBufferSharedHandle<ExampleStructuredBufferCompute> m_structuredBuffer;

	CrDataBufferSharedHandle m_colorsRWDataBuffer;

	CrGPUBufferSharedHandle m_indirectDispatchArguments;

	void* m_platformWindow = nullptr;
	void* m_platformHandle = nullptr;

	CrSharedPtr<CrCamera> m_camera;
	Camera m_cameraConstantData;

	CrRenderWorldSharedHandle m_renderWorld;

	CrRenderGraph m_mainRenderGraph;

	CrUniquePtr<CrGPUTimingQueryTracker> m_timingQueryTracker;

	// This would need to be buffered to account for multiple threads
	CrSharedPtr<CrCPUStackAllocator> m_renderingStream;
};