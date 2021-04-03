#pragma once

#include "Core/Containers/CrVector.h"
#include "Core/SmartPointers/CrSharedPtr.h"

#include "CrRenderingForwardDeclarations.h"

#include "GeneratedShaders/ShaderMetadata.h"

class CrRenderModel;
using CrRenderModelSharedHandle = CrSharedPtr<CrRenderModel>;


class CrFrame
{
public:

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
	CrGraphicsPipelineHandle m_pipelineTriangleState;
	CrGraphicsPipelineHandle m_pipelineLineState;
	CrComputePipelineHandle m_computePipelineState;

	CrVertexBufferSharedHandle m_triangleVertexBuffer;
	CrIndexBufferSharedHandle m_triangleIndexBuffer;
	
	CrRenderModelSharedHandle m_renderModel;

	CrSwapchainSharedHandle m_swapchain;

	CrTextureSharedHandle m_depthStencilTexture;

	CrSamplerSharedHandle m_linearClampSamplerHandle;
	CrSamplerSharedHandle m_linearWrapSamplerHandle;

	CrTextureSharedHandle m_colorsRWTexture;

	CrStructuredBufferSharedHandle<ExampleRWStructuredBufferCompute> m_structuredBuffer;
	CrDataBufferSharedHandle m_colorsRWDataBuffer;
	void* m_platformWindow = nullptr;
	void* m_platformHandle = nullptr;
};