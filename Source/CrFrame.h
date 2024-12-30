#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrRenderGraph.h"
#include "Rendering/CrBuiltinPipeline.h"
#include "Rendering/RenderWorld/CrRenderWorld.h"

#include "GeneratedShaders/ShaderMetadata.h"

#include "Core/Containers/CrVector.h"
#include "Core/SmartPointers/CrUniquePtr.h"

namespace GBufferDebugMode { enum T : uint32_t; }

class CrOSWindow;

class CrFrame
{
public:

	CrFrame();

	~CrFrame();

	void Initialize(CrIntrusivePtr<CrOSWindow> mainWindow);

	void Deinitialize();

	void Process();

	void DrawDebugUI();

	void RecreateRenderTargets();

private:

	uint32_t m_currentCommandBuffer = 0;

	CrVector<CrCommandBufferHandle> m_drawCmdBuffers; // Command buffers used for rendering
	
	CrComputePipelineHandle m_exampleComputePipeline;

	CrComputePipelineHandle m_depthDownsampleLinearize;

	CrComputePipelineHandle m_postProcessing;

	CrGraphicsPipelineHandle m_copyTexturePipeline;

	// Gets the value of the instance id at the mouse position and stores it in a buffer
	CrComputePipelineHandle m_mouseSelectionResolvePipeline;

	CrGraphicsPipelineHandle m_directionalLightPipeline;

	CrGraphicsPipelineHandle m_gbufferDebugPipeline;

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

	CrTypedBufferHandle m_colorsRWTypedBuffer;

	CrGPUBufferHandle m_indirectDispatchArguments;

	CrIntrusivePtr<CrOSWindow> m_mainWindow;

	// We use these two variables to trigger render target resizing after a window resize
	uint32_t m_currentWindowWidth;

	uint32_t m_currentWindowHeight;

	CrCameraHandle m_camera;

	Camera m_cameraConstantData;

	CrRenderWorldHandle m_renderWorld;

	CrRenderGraph m_mainRenderGraph;

	CrUniquePtr<CrGPUTimingQueryTracker> m_timingQueryTracker;

	// This would need to be buffered to account for multiple threads
	CrIntrusivePtr<CrCPUStackAllocator> m_renderingStream;

#if !defined(CR_CONFIG_FINAL)
	GBufferDebugMode::T m_gbufferDebugMode = (GBufferDebugMode::T)0;
#endif
};