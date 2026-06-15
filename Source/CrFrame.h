#pragma once

#include "Graphics/CrRenderingForwardDeclarations.h"
#include "Graphics/CrRenderGraph.h"
#include "Graphics/CrBuiltinPipeline.h"
#include "Graphics/RenderWorld/CrRenderWorld.h"

#include "GeneratedShaders/ShaderMetadata.h"

#include "crstl/unique_ptr.h"
#include "crstl/vector.h"

namespace GBufferDebugMode { enum T : uint32_t; }

class CrOSWindow;

class CrFrame
{
public:

	CrFrame();

	~CrFrame();

	void Initialize(crstl::intrusive_ptr<CrOSWindow> mainWindow);

	void Deinitialize();

	void Process();

	void DrawDebugUI();

	void RecreateRenderTargets();

private:

	uint32_t m_currentCommandBuffer = 0;

	crstl::vector<CrCommandBufferHandle> m_drawCmdBuffers; // Command buffers used for rendering
	
	CrComputePipelineHandle m_exampleComputePipeline;

	CrComputePipelineHandle m_depthDownsampleLinearize;

	CrComputePipelineHandle m_postProcessing;

	CrGraphicsPipelineHandle m_copyTexturePipeline;

	// Gets the value of the instance id at the mouse position and stores it in a buffer
	CrComputePipelineHandle m_mouseSelectionResolvePipeline;

	CrGraphicsPipelineHandle m_directionalLightPipeline;

	CrGraphicsPipelineHandle m_gbufferDebugPipeline;

	CrComputePipelineHandle m_createIndirectArguments;

	crgfx::TextureHandle m_colorfulVolumeTexture;


	crgfx::TextureHandle m_colorfulTextureArray;

	// Editor Shaders
	CrGraphicsPipelineHandle m_editorEdgeSelectionPipeline;

	CrGraphicsPipelineHandle m_editorGridPipeline;

	CrSwapchainHandle m_swapchain;

	crgfx::TextureHandle m_depthStencilTexture;

	// Linear depth with a min max mip chain
	crgfx::TextureHandle m_linearDepthMinMaxMipChain;

	crgfx::TextureHandle m_preSwapchainTexture;

	crgfx::TextureHandle m_gbufferAlbedoAOTexture;
	crgfx::TextureHandle m_gbufferNormalsTexture;
	crgfx::TextureHandle m_gbufferMaterialTexture;

	crgfx::TextureHandle m_lightingTexture;

	crgfx::TextureHandle m_lightClusters;

	crgfx::TextureHandle m_debugShaderTexture;

	CrGPUBufferHandle m_mouseSelectionBuffer;

	crgfx::TextureHandle m_colorsRWTexture;

	CrStructuredBufferHandle<ExampleRWStructuredBufferCompute> m_rwStructuredBuffer;

	CrStructuredBufferHandle<ExampleStructuredBufferCompute> m_structuredBuffer;

	CrTypedBufferHandle m_colorsRWTypedBuffer;

	CrGPUBufferHandle m_indirectDispatchArguments;

	crstl::intrusive_ptr<CrOSWindow> m_mainWindow;

	// We use these two variables to trigger render target resizing after a window resize
	uint32_t m_currentWindowWidth;

	uint32_t m_currentWindowHeight;

	CrCameraHandle m_camera;

	CameraCB m_cameraConstantData;

	CrRenderWorldHandle m_renderWorld;

	CrRenderGraph m_mainRenderGraph;

	crstl::unique_ptr<CrGPUTimingQueryTracker> m_timingQueryTracker;

	// This would need to be buffered to account for multiple threads
	crstl::intrusive_ptr<CrCPUStackAllocator> m_renderingStream;

#if !defined(CR_CONFIG_FINAL)
	GBufferDebugMode::T m_gbufferDebugMode = (GBufferDebugMode::T)0;
#endif
};