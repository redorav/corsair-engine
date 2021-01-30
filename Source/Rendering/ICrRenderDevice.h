#pragma once

#include "ICrSwapchain.h"
#include "Core/SmartPointers/CrUniquePtr.h"
#include "Rendering/CrRendering.h"

#include "CrRenderingForwardDeclarations.h"

struct CrRenderDeviceProperties
{
	cr3d::GraphicsApi::T m_graphicsApi;

	uint32_t maxConstantBufferRange;
	uint32_t maxTextureDimension1D;
	uint32_t maxTextureDimension2D;
	uint32_t maxTextureDimension3D;
};

namespace CrCommandQueueType { enum T : uint8_t; }

namespace CrRenderingFeature
{
	enum T
	{
		Tessellation,
		GeometryShaders,
		Raytracing,
		MeshShaders,
		AmplificationShaders,
		TextureCompressionBC,
		TextureCompressionETC,
		TextureCompressionASTC,
	};
}

class ICrRenderDevice
{
public:

	ICrRenderDevice();

	~ICrRenderDevice();

	void Init(cr3d::GraphicsApi::T graphicsApi);

	// Resource Creation Functions

	CrCommandQueueSharedHandle CreateCommandQueue(CrCommandQueueType::T type);

	CrFramebufferSharedHandle CreateFramebuffer(const CrFramebufferCreateParams& params);

	CrIndexBufferSharedHandle CreateIndexBuffer(cr3d::DataFormat::T dataFormat, uint32_t numIndices);

	CrRenderPassSharedHandle CreateRenderPass(const CrRenderPassDescriptor& renderPassDescriptor);

	CrSamplerSharedHandle CreateSampler(const CrSamplerDescriptor& descriptor);

	CrSwapchainSharedHandle CreateSwapchain(const CrSwapchainDescriptor& swapchainDescriptor);

	CrTextureSharedHandle CreateTexture(const CrTextureCreateParams& params);

	CrVertexBufferSharedHandle CreateVertexBuffer(uint32_t numVertices, const CrVertexDescriptor& vertexDescriptor);

	CrGraphicsShaderHandle CreateGraphicsShader(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor) const;

	CrComputeShaderHandle CreateComputeShader(const CrComputeShaderDescriptor& computeShaderDescriptor) const;

	CrGraphicsPipelineHandle CreateGraphicsPipeline(const CrGraphicsPipelineDescriptor& pipelineDescriptor, const ICrGraphicsShader* graphicsShader,
		const CrVertexDescriptor& vertexDescriptor, const CrRenderPassDescriptor& renderPassDescriptor);
	
	CrComputePipelineHandle CreateComputePipeline(const CrComputePipelineDescriptor& pipelineDescriptor, const ICrComputeShader* computeShader);

	ICrHardwareGPUBuffer* CreateHardwareGPUBuffer(const CrHardwareGPUBufferDescriptor& descriptor);

	template<typename Struct>
	CrVertexBufferSharedHandle CreateVertexBuffer(uint32_t numVertices);

	CrGPUFenceSharedHandle CreateGPUFence();

	CrGPUSemaphoreSharedHandle CreateGPUSemaphore();

	// GPU Synchronization functions

	cr3d::GPUWaitResult WaitForFence(ICrGPUFence* fence, uint64_t timeoutNanoseconds);

	void ResetFence(ICrGPUFence* fence);

	// Wait until all operations on all queues have completed
	void WaitIdle();

	virtual bool GetIsFeatureSupported(CrRenderingFeature::T feature) const = 0;

	const CrRenderDeviceProperties& GetProperties();

	static ICrRenderDevice* GetRenderDevice();

	const CrCommandQueueSharedHandle& GetMainCommandQueue() const;

	static void Create(cr3d::GraphicsApi::T graphicsApi);

protected:

	virtual ICrCommandQueue* CreateCommandQueuePS(CrCommandQueueType::T type) = 0;

	virtual ICrFramebuffer* CreateFramebufferPS(const CrFramebufferCreateParams& params) = 0;

	virtual ICrGPUFence* CreateGPUFencePS() = 0;

	virtual ICrGPUSemaphore* CreateGPUSemaphorePS() = 0;

	virtual ICrGraphicsShader* CreateGraphicsShaderPS(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor) const = 0;

	virtual ICrComputeShader* CreateComputeShaderPS(const CrComputeShaderDescriptor& computeShaderDescriptor) const = 0;

	virtual ICrHardwareGPUBuffer* CreateHardwareGPUBufferPS(const CrHardwareGPUBufferDescriptor& params) = 0;

	virtual ICrRenderPass* CreateRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor) = 0;

	virtual ICrSampler* CreateSamplerPS(const CrSamplerDescriptor& descriptor) = 0;

	virtual ICrSwapchain* CreateSwapchainPS(const CrSwapchainDescriptor& swapchainDescriptor) = 0;

	virtual ICrTexture* CreateTexturePS(const CrTextureCreateParams& params) = 0;
	
	virtual ICrGraphicsPipeline* CreateGraphicsPipelinePS(const CrGraphicsPipelineDescriptor& psoDescriptor, const ICrGraphicsShader* graphicsShader,
		const CrVertexDescriptor& vertexDescriptor, const CrRenderPassDescriptor& renderPassDescriptor) = 0;

	virtual ICrComputePipeline* CreateComputePipelinePS(const CrComputePipelineDescriptor& psoDescriptor, const ICrComputeShader* computeShader) = 0;
	
	virtual void InitPS() = 0;

	virtual cr3d::GPUWaitResult WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds) = 0;

	virtual void WaitIdlePS() = 0;

	virtual void ResetFencePS(const ICrGPUFence* fence) = 0;

	CrRenderDeviceProperties m_renderDeviceProperties;

	CrCommandQueueSharedHandle m_mainCommandQueue;

	CrVector<CrCommandQueueSharedHandle> m_commandQueues;
};

template<typename Struct>
inline CrVertexBufferSharedHandle ICrRenderDevice::CreateVertexBuffer(uint32_t numVertices)
{
	return CreateVertexBuffer(numVertices, Struct::GetVertexDescriptor());
}

inline const CrCommandQueueSharedHandle& ICrRenderDevice::GetMainCommandQueue() const
{
	return m_mainCommandQueue;
}