#pragma once

#include "Core/Containers/CrVector.h"
#include "Core/SmartPointers/CrSharedPtr.h"

#include "CrRenderingForwardDeclarations.h"

class CrRenderModel;
using CrRenderModelSharedHandle = CrSharedPtr<CrRenderModel>;

class ICrGraphicsPipeline;

class CrFrame
{
public:

	void Init(void* platformHandle, void* platformWindow, uint32_t width, uint32_t height);

	void Process();

	void UpdateCamera();

	void RecreateSwapchainAndFramebuffers();

private:

	CrVector<CrCommandBufferSharedHandle> m_drawCmdBuffers; // Command buffers used for rendering
	CrVector<CrFramebufferSharedHandle> m_frameBuffers;
	// TODO Pass as param
	uint32_t m_width = 0;
	uint32_t m_height = 0;

	// Semaphores
	// Used to coordinate operations within the graphics queue and ensure correct command ordering
	CrGPUSemaphoreSharedHandle m_renderCompleteSemaphore;
	CrGPUSemaphoreSharedHandle m_presentCompleteSemaphore;
	
	// TODO Temporary
	CrGraphicsPipelineHandle m_pipelineTriangleState;
	CrGraphicsPipelineHandle m_pipelineLineState;
	CrComputePipelineHandle m_computePipelineState;

	CrRenderPassSharedHandle m_renderPass;
	
	CrVertexBufferSharedHandle m_triangleVertexBuffer;
	CrIndexBufferSharedHandle m_triangleIndexBuffer;
	
	CrRenderModelSharedHandle m_renderModel;

	CrSwapchainSharedHandle m_swapchain;

	CrTextureSharedHandle m_depthStencilTexture;

	CrSamplerSharedHandle m_linearClampSamplerHandle;
	CrSamplerSharedHandle m_linearWrapSamplerHandle;

	CrDataBufferSharedHandle m_colorsRWDataBuffer;
	void* m_platformWindow = nullptr;
	void* m_platformHandle = nullptr;
};