#pragma once

#include "Core/Containers/CrVector.h"
#include "Core/SmartPointers/CrSharedPtr.h"

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrRenderGraph.h"
#include "Rendering/CrBuiltinPipeline.h"

#include "Rendering/RenderWorld/CrRenderWorld.h"

#include "GeneratedShaders/ShaderMetadata.h"

class CrRenderModel;
using CrRenderModelHandle = CrSharedPtr<CrRenderModel>;

class CrRenderWorld;
using CrRenderWorldHandle = CrSharedPtr<CrRenderWorld>;

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

	CrVector<CrCommandBufferHandle> m_drawCmdBuffers; // Command buffers used for rendering
	
	uint32_t m_width = 0;

	uint32_t m_height = 0;

	// Basic Shaders

	CrBuiltinGraphicsPipeline m_basic2DPipeline;
	CrBuiltinGraphicsPipeline m_basic3DPipeline;

	CrBuiltinGraphicsPipeline m_line2DPipeline;
	CrBuiltinGraphicsPipeline m_line3DPipeline;

	CrBuiltinComputePipeline m_exampleComputePipeline;
	CrBuiltinGraphicsPipeline m_copyTexturePipeline;

	// Gets the value of the instance id at the mouse position and stores it in a buffer
	CrBuiltinComputePipeline m_mouseSelectionResolvePipeline;

	CrBuiltinGraphicsPipeline m_directionalLightPipeline;

	CrBuiltinComputePipeline m_createIndirectArguments;

	CrTextureHandle m_colorfulVolumeTexture;

	CrTextureHandle m_colorfulTextureArray;

	// Editor Shaders
	CrBuiltinGraphicsPipeline m_editorEdgeSelectionPipeline;

	CrSwapchainHandle m_swapchain;

	CrTextureHandle m_depthStencilTexture;
	CrTextureHandle m_preSwapchainTexture;

	CrTextureHandle m_gbufferAlbedoAOTexture;
	CrTextureHandle m_gbufferNormalsTexture;
	CrTextureHandle m_gbufferMaterialTexture;

	CrTextureHandle m_lightingTexture;

	CrTextureHandle m_debugShaderTexture;

	CrGPUBufferHandle m_mouseSelectionBuffer;

	CrTextureHandle m_colorsRWTexture;

	CrStructuredBufferHandle<ExampleRWStructuredBufferCompute> m_rwStructuredBuffer;

	CrStructuredBufferHandle<ExampleStructuredBufferCompute> m_structuredBuffer;

	CrDataBufferHandle m_colorsRWDataBuffer;

	CrGPUBufferHandle m_indirectDispatchArguments;

	void* m_platformWindow = nullptr;
	void* m_platformHandle = nullptr;

	CrSharedPtr<CrCamera> m_camera;
	Camera m_cameraConstantData;

	CrRenderWorldHandle m_renderWorld;

	CrRenderGraph m_mainRenderGraph;

	CrUniquePtr<CrGPUTimingQueryTracker> m_timingQueryTracker;

	// This would need to be buffered to account for multiple threads
	CrSharedPtr<CrCPUStackAllocator> m_renderingStream;
};