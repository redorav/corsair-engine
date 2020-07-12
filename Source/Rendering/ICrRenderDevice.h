#pragma once

#include "ICrSwapchain.h"
#include "Core/SmartPointers/CrUniquePtr.h"
#include "Rendering/CrRendering.h"

class ICrFramebuffer;
using CrFramebufferSharedHandle = CrSharedPtr<ICrFramebuffer>;
using CrFramebufferUniqueHandle = CrUniquePtr<ICrFramebuffer>;
struct CrFramebufferCreateParams;

class ICrTexture;
using CrTextureSharedHandle = CrSharedPtr<ICrTexture>;
struct CrTextureCreateParams;

class ICrSampler;
using CrSamplerSharedHandle = CrSharedPtr<ICrSampler>;
struct CrSamplerDescriptor;

class ICrRenderPass;
using CrRenderPassSharedHandle = CrSharedPtr<ICrRenderPass>;
struct CrRenderPassDescriptor;

class ICrCommandQueue;
using CrCommandQueueSharedHandle = CrSharedPtr<ICrCommandQueue>;

class ICrCommandBuffer;
using CrCommandBufferUniqueHandle = CrUniquePtr<ICrCommandBuffer>;

class CrIndexBufferCommon;
using CrIndexBufferSharedHandle = CrSharedPtr<CrIndexBufferCommon>;

class CrVertexBufferCommon;
using CrVertexBufferSharedHandle = CrSharedPtr<CrVertexBufferCommon>;

class ICrSwapchain;
using CrSwapchainSharedHandle = CrSharedPtr<ICrSwapchain>;

class ICrGPUFence;
using CrGPUFenceSharedHandle = CrSharedPtr<ICrGPUFence>;

class ICrGPUSemaphore;
using CrGPUSemaphoreSharedHandle = CrSharedPtr<ICrGPUSemaphore>;

class ICrGPUStackAllocator;

class ICrHardwareGPUBuffer;
struct CrGPUBufferCreateParams;

class CrGPUStackAllocatorVulkan;
class CrRenderDeviceVulkan;

class CrVertexDescriptor;

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

	ICrHardwareGPUBuffer* CreateHardwareGPUBuffer(const CrGPUBufferCreateParams& params);

	template<typename Struct>
	CrVertexBufferSharedHandle CreateVertexBuffer(uint32_t numVertices);

	CrUniquePtr<ICrGPUStackAllocator> CreateGPUMemoryStream();

	CrGPUFenceSharedHandle CreateGPUFence();

	CrGPUSemaphoreSharedHandle CreateGPUSemaphore();

	// GPU Synchronization functions

	cr3d::GPUWaitResult WaitForFence(ICrGPUFence* fence, uint64_t timeoutNanoseconds);

	void ResetFence(ICrGPUFence* fence);

	// Wait until all operations on all queues have completed
	void WaitIdle();

	virtual bool GetIsFeatureSupported(CrRenderingFeature::T feature) = 0;

	const CrRenderDeviceProperties& GetProperties();

	static ICrRenderDevice* GetRenderDevice();

	const CrCommandQueueSharedHandle& GetMainCommandQueue() const;

	static void Create(cr3d::GraphicsApi::T graphicsApi);

protected:

	virtual ICrCommandQueue* CreateCommandQueuePS(CrCommandQueueType::T type) = 0;

	virtual ICrFramebuffer* CreateFramebufferPS(const CrFramebufferCreateParams& params) = 0;

	virtual ICrRenderPass* CreateRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor) = 0;

	virtual ICrSampler* CreateSamplerPS(const CrSamplerDescriptor& descriptor) = 0;

	virtual ICrSwapchain* CreateSwapchainPS(const CrSwapchainDescriptor& swapchainDescriptor) = 0;

	virtual ICrTexture* CreateTexturePS(const CrTextureCreateParams& params) = 0;

	virtual ICrGPUStackAllocator* CreateGPUMemoryStreamPS() = 0;

	virtual ICrGPUFence* CreateGPUFencePS() = 0;

	virtual ICrGPUSemaphore* CreateGPUSemaphorePS() = 0;

	virtual ICrHardwareGPUBuffer* CreateHardwareGPUBufferPS(const CrGPUBufferCreateParams& params) = 0;

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