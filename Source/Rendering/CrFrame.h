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

	void Init(void* platformHandle, void* platformWindow, uint32_t width, uint32_t height);

	void Process();

	void DrawDebugUI();

	void UpdateCamera();

	void RecreateSwapchainAndDepth();

private:

	CrVector<CrCommandBufferSharedHandle> m_drawCmdBuffers; // Command buffers used for rendering
	
	uint32_t m_width = 0;

	uint32_t m_height = 0;

	// TODO Temporary
	CrBuiltinGraphicsPipeline m_linePipeline;
	CrBuiltinComputePipeline m_computePipeline;
	CrBuiltinGraphicsPipeline m_copyTexturePipeline;

	CrTextureSharedHandle m_defaultWhiteTexture;

	CrSwapchainSharedHandle m_swapchain;

	CrTextureSharedHandle m_depthStencilTexture;
	CrTextureSharedHandle m_preSwapchainTexture;

	CrSamplerSharedHandle m_linearClampSamplerHandle;
	CrSamplerSharedHandle m_linearWrapSamplerHandle;
	CrSamplerSharedHandle m_pointClampSamplerHandle;
	CrSamplerSharedHandle m_pointWrapSamplerHandle;

	CrTextureSharedHandle m_colorsRWTexture;

	CrStructuredBufferSharedHandle<ExampleRWStructuredBufferCompute> m_rwStructuredBuffer;

	CrStructuredBufferSharedHandle<ExampleStructuredBufferCompute> m_structuredBuffer;

	CrDataBufferSharedHandle m_colorsRWDataBuffer;

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