#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrRenderGraph.h"
#include "Rendering/CrBuiltinPipeline.h"
#include "Rendering/RenderWorld/CrRenderWorld.h"

#include "GeneratedShaders/ShaderMetadata.h"

#include "Core/Containers/CrVector.h"
#include "Core/SmartPointers/CrUniquePtr.h"

class CrFrame
{
public:

	CrFrame();

	~CrFrame();

	void Initialize(void* platformHandle, void* platformWindow, uint32_t width, uint32_t height);

	void Deinitialize();

	void Process();

	void DrawDebugUI();

	void HandleWindowResize(uint32_t width, uint32_t height);

	void RecreateSwapchainAndRenderTargets(uint32_t width, uint32_t height);

private:

	bool m_requestSwapchainResize = false;

	uint32_t m_swapchainResizeRequestWidth;

	uint32_t m_swapchainResizeRequestHeight;

	CrVector<CrCommandBufferHandle> m_drawCmdBuffers; // Command buffers used for rendering
	
	uint32_t m_width = 0;

	uint32_t m_height = 0;

	CrComputePipelineHandle m_exampleComputePipeline;

	CrComputePipelineHandle m_depthDownsampleLinearize;

	CrComputePipelineHandle m_postProcessing;

	CrGraphicsPipelineHandle m_copyTexturePipeline;

	// Gets the value of the instance id at the mouse position and stores it in a buffer
	CrComputePipelineHandle m_mouseSelectionResolvePipeline;

	CrGraphicsPipelineHandle m_directionalLightPipeline;

	CrComputePipelineHandle m_createIndirectArguments;

	CrTextureHandle m_colorfulVolumeTexture;

	CrTextureHandle m_colorfulTextureArray;

	// Editor Shaders
	CrGraphicsPipelineHandle m_editorEdgeSelectionPipeline;

	CrGraphicsPipelineHandle m_editorGridPipeline;

	CrSwapchainHandle m_swapchain;

	CrTextureHandle m_depthStencilTexture;

	// 16-bit linear depth with a min max mip chain
	CrTextureHandle m_linearDepth16MinMaxMipChain;

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

	CrCameraHandle m_camera;

	Camera m_cameraConstantData;

	CrRenderWorldHandle m_renderWorld;

	CrRenderGraph m_mainRenderGraph;

	CrUniquePtr<CrGPUTimingQueryTracker> m_timingQueryTracker;

	// This would need to be buffered to account for multiple threads
	CrIntrusivePtr<CrCPUStackAllocator> m_renderingStream;
};