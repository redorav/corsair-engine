#pragma once

#include "Core/Containers/CrVector.h"
#include "Core/SmartPointers/CrSharedPtr.h"

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrRenderWorld.h"

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
	CrGraphicsPipelineHandle m_linePipelineState;
	CrComputePipelineHandle m_computePipelineState;

	CrTextureSharedHandle m_defaultWhiteTexture;

	CrSwapchainSharedHandle m_swapchain;

	CrTextureSharedHandle m_depthStencilTexture;

	CrSamplerSharedHandle m_linearClampSamplerHandle;
	CrSamplerSharedHandle m_linearWrapSamplerHandle;

	CrTextureSharedHandle m_colorsRWTexture;

	CrStructuredBufferSharedHandle<ExampleRWStructuredBufferCompute> m_rwStructuredBuffer;

	CrStructuredBufferSharedHandle<ExampleStructuredBufferCompute> m_structuredBuffer;

	CrDataBufferSharedHandle m_colorsRWDataBuffer;

	void* m_platformWindow = nullptr;
	void* m_platformHandle = nullptr;

	CrRenderModelInstance m_modelInstance0;
	CrRenderModelInstance m_modelInstance1;
	CrRenderModelInstance m_modelInstance2;

	CrRenderWorldSharedHandle m_renderWorld;
};