#pragma once

#include "Core/Containers/CrVector.h"
#include "Core/SmartPointers/CrSharedPtr.h"

class CrVertexBufferCommon;
using CrVertexBufferSharedHandle = CrSharedPtr<CrVertexBufferCommon>;

class CrIndexBufferCommon;
using CrIndexBufferSharedHandle = CrSharedPtr<CrIndexBufferCommon>;

class CrRenderModel;
using CrRenderModelSharedHandle = CrSharedPtr<CrRenderModel>;

class ICrRenderPass;
using CrRenderPassSharedHandle = CrSharedPtr<ICrRenderPass>;

class ICrFramebuffer;
using CrFramebufferSharedHandle = CrSharedPtr<ICrFramebuffer>;

class ICrGPUSemaphore;
using CrGPUSemaphoreSharedHandle = CrSharedPtr<ICrGPUSemaphore>;

class ICrSwapchain;
using CrSwapchainSharedHandle = CrSharedPtr<ICrSwapchain>;

class ICrTexture;
using CrTextureSharedHandle = CrSharedPtr<ICrTexture>;

class ICrCommandBuffer;
class CrGraphicsPipeline;

class CrFrame
{
public:

	void Init(void* platformHandle, void* platformWindow, uint32_t width, uint32_t height);

	void Process();

	void UpdateCamera();

	void RecreateSwapchainAndFramebuffers();

private:

	CrVector<ICrCommandBuffer*> m_drawCmdBuffers; // Command buffers used for rendering
	CrVector<CrFramebufferSharedHandle> m_frameBuffers;
	// TODO Pass as param
	uint32_t m_width = 0;
	uint32_t m_height = 0;

	// Semaphores
	// Used to coordinate operations within the graphics queue and ensure correct command ordering
	CrGPUSemaphoreSharedHandle m_renderCompleteSemaphore;
	CrGPUSemaphoreSharedHandle m_presentCompleteSemaphore;
	
	// TODO Temporary
	CrGraphicsPipeline* m_pipelineTriangleState;
	CrGraphicsPipeline* m_pipelineLineState;

	CrRenderPassSharedHandle m_renderPass;
	
	CrVertexBufferSharedHandle m_triangleVertexBuffer;
	CrIndexBufferSharedHandle m_triangleIndexBuffer;
	
	CrRenderModelSharedHandle m_renderModel;

	CrSwapchainSharedHandle m_swapchain;

	CrTextureSharedHandle m_depthStencilTexture;

	void* m_platformWindow = nullptr;
	void* m_platformHandle = nullptr;
};