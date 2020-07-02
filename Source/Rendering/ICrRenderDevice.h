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

class ICrGPUFence;
using CrGPUFenceSharedHandle = CrSharedPtr<ICrGPUFence>;

class ICrSwapchain;
using CrSwapchainSharedHandle = CrSharedPtr<ICrSwapchain>;

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

	void Init(void* platformHandle, void* platformWindow, cr3d::GraphicsApi::T graphicsApi);

	void Present();

	// Resource Creation Functions

	CrCommandQueueSharedHandle CreateCommandQueue(CrCommandQueueType::T type);

	CrFramebufferSharedHandle CreateFramebuffer(const CrFramebufferCreateParams& params);

	CrGPUFenceSharedHandle CreateGPUFence();

	CrIndexBufferSharedHandle CreateIndexBuffer(cr3d::DataFormat::T dataFormat, uint32_t numIndices);

	CrRenderPassSharedHandle CreateRenderPass(const CrRenderPassDescriptor& renderPassDescriptor);

	CrSamplerSharedHandle CreateSampler(const CrSamplerDescriptor& descriptor);

	CrSwapchainSharedHandle CreateSwapchain(uint32_t requestedWidth, uint32_t requestedHeight);

	CrTextureSharedHandle CreateTexture(const CrTextureCreateParams& params);

	CrVertexBufferSharedHandle CreateVertexBuffer(uint32_t numVertices, const CrVertexDescriptor& vertexDescriptor);

	ICrHardwareGPUBuffer* CreateHardwareGPUBuffer(const CrGPUBufferCreateParams& params);

	template<typename Struct>
	CrVertexBufferSharedHandle CreateVertexBuffer(uint32_t numVertices);

	CrUniquePtr<ICrGPUStackAllocator> CreateGPUMemoryStream();

	// TODO Create GPU Semaphore and GPU fence from here. We can call them whatever we like, at the end of the day they are GPU-GPU and GPU-CPU
	// synchronization primitives

	virtual bool GetIsFeatureSupported(CrRenderingFeature::T feature) = 0;

	const CrRenderDeviceProperties& GetProperties();

	static ICrRenderDevice* GetRenderDevice();

	static CrRenderDeviceVulkan* GetRenderDevicePS(); // TODO Remove

	const CrCommandQueueSharedHandle& GetMainCommandQueue() const;

	const CrSwapchainSharedHandle& GetMainSwapchain() const;

	const CrTextureSharedHandle& GetMainDepthBuffer() const;

	static void Create(void* platformHandle, void* platformWindow, cr3d::GraphicsApi::T graphicsApi);

protected:

	virtual ICrCommandQueue* CreateCommandQueuePS(CrCommandQueueType::T type) = 0;

	virtual ICrFramebuffer* CreateFramebufferPS(const CrFramebufferCreateParams& params) = 0;

	virtual ICrGPUFence* CreateGPUFencePS() = 0;

	virtual ICrRenderPass* CreateRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor) = 0;

	virtual ICrSampler* CreateSamplerPS(const CrSamplerDescriptor& descriptor) = 0;

	virtual ICrSwapchain* CreateSwapchainPS(uint32_t requestedWidth, uint32_t requestedHeight) = 0;

	virtual ICrTexture* CreateTexturePS(const CrTextureCreateParams& params) = 0;

	virtual ICrGPUStackAllocator* CreateGPUMemoryStreamPS() = 0;

	virtual ICrHardwareGPUBuffer* CreateHardwareGPUBufferPS(const CrGPUBufferCreateParams& params) = 0;

	virtual void InitPS(void* platformHandle, void* platformWindow) = 0;

	virtual void PresentPS() = 0;

	CrRenderDeviceProperties m_renderDeviceProperties;

	CrSwapchainSharedHandle m_swapChain;

	CrTextureSharedHandle m_depthStencilTexture;

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