#pragma once

#include "Rendering/CrRendering.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

#include "Core/Containers/CrVector.h"
#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/String/CrFixedString.h"

namespace CrVendor
{
	enum T
	{
		Unknown,
		NVIDIA,
		AMD,
		Intel
	};
}

struct CrRenderDeviceProperties
{
	CrVendor::T vendor = CrVendor::Unknown;
	CrFixedString128 description;

	uint32_t maxConstantBufferRange = 0;
	uint32_t maxTextureDimension1D = 0;
	uint32_t maxTextureDimension2D = 0;
	uint32_t maxTextureDimension3D = 0;
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

inline CrVendor::T GetVendorFromVendorID(unsigned int vendorID)
{
	switch (vendorID)
	{
		case 0x10DE:
			return CrVendor::NVIDIA;
		case 0x1002:
			return CrVendor::AMD;
		case 0x8086:
			return CrVendor::Intel;
		default:
			return CrVendor::Unknown;
	}
}

class ICrRenderDevice
{
public:

	ICrRenderDevice(const ICrRenderSystem* renderSystem);

	virtual ~ICrRenderDevice();

	// Resource Creation Functions

	CrCommandQueueSharedHandle CreateCommandQueue(CrCommandQueueType::T type);

	CrFramebufferSharedHandle CreateFramebuffer(const CrFramebufferDescriptor& params);

	CrIndexBufferSharedHandle CreateIndexBuffer(cr3d::DataFormat::T dataFormat, uint32_t numIndices);

	CrRenderPassSharedHandle CreateRenderPass(const CrRenderPassDescriptor& renderPassDescriptor);

	CrSamplerSharedHandle CreateSampler(const CrSamplerDescriptor& descriptor);

	CrSwapchainSharedHandle CreateSwapchain(const CrSwapchainDescriptor& swapchainDescriptor);

	CrTextureSharedHandle CreateTexture(const CrTextureDescriptor& descriptor);

	CrVertexBufferSharedHandle CreateVertexBuffer(uint32_t numVertices, const CrVertexDescriptor& vertexDescriptor);

	template<typename Metadata>
	CrStructuredBufferSharedHandle<Metadata> CreateStructuredBuffer(cr3d::BufferAccess::T access, uint32_t numElements);

	CrDataBufferSharedHandle CreateDataBuffer(cr3d::BufferAccess::T access, cr3d::DataFormat::T dataFormat, uint32_t numElements);

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

	const CrCommandQueueSharedHandle& GetMainCommandQueue() const;

protected:

	virtual ICrCommandQueue* CreateCommandQueuePS(CrCommandQueueType::T type) = 0;

	virtual ICrFramebuffer* CreateFramebufferPS(const CrFramebufferDescriptor& params) = 0;

	virtual ICrGPUFence* CreateGPUFencePS() = 0;

	virtual ICrGPUSemaphore* CreateGPUSemaphorePS() = 0;

	virtual ICrGraphicsShader* CreateGraphicsShaderPS(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor) const = 0;

	virtual ICrComputeShader* CreateComputeShaderPS(const CrComputeShaderDescriptor& computeShaderDescriptor) const = 0;

	virtual ICrHardwareGPUBuffer* CreateHardwareGPUBufferPS(const CrHardwareGPUBufferDescriptor& params) = 0;

	virtual ICrRenderPass* CreateRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor) = 0;

	virtual ICrSampler* CreateSamplerPS(const CrSamplerDescriptor& descriptor) = 0;

	virtual ICrSwapchain* CreateSwapchainPS(const CrSwapchainDescriptor& swapchainDescriptor) = 0;

	virtual ICrTexture* CreateTexturePS(const CrTextureDescriptor& descriptor) = 0;
	
	virtual ICrGraphicsPipeline* CreateGraphicsPipelinePS(const CrGraphicsPipelineDescriptor& psoDescriptor, const ICrGraphicsShader* graphicsShader,
		const CrVertexDescriptor& vertexDescriptor, const CrRenderPassDescriptor& renderPassDescriptor) = 0;

	virtual ICrComputePipeline* CreateComputePipelinePS(const CrComputePipelineDescriptor& pipelineDescriptor, const ICrComputeShader* computeShader) = 0;
	
	virtual cr3d::GPUWaitResult WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds) = 0;

	virtual void WaitIdlePS() = 0;

	virtual void ResetFencePS(const ICrGPUFence* fence) = 0;

	const ICrRenderSystem* m_renderSystem;

	CrRenderDeviceProperties m_renderDeviceProperties;

	CrCommandQueueSharedHandle m_mainCommandQueue;

	CrVector<CrCommandQueueSharedHandle> m_commandQueues;
};

template<typename Struct>
inline CrVertexBufferSharedHandle ICrRenderDevice::CreateVertexBuffer(uint32_t numVertices)
{
	return CreateVertexBuffer(numVertices, Struct::GetVertexDescriptor());
}

template<typename Metadata>
CrStructuredBufferSharedHandle<Metadata> ICrRenderDevice::CreateStructuredBuffer(cr3d::BufferAccess::T access, uint32_t numElements)
{
	return CrStructuredBufferSharedHandle<Metadata>(new CrStructuredBuffer<Metadata>(this, access, numElements));
}

inline const CrCommandQueueSharedHandle& ICrRenderDevice::GetMainCommandQueue() const
{
	return m_mainCommandQueue;
}