#pragma once

#include "Rendering/CrRendering.h"

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrGPUDeletionQueue.h"

#include "Core/Containers/CrVector.h"
#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/String/CrFixedString.h"
#include "Core/Function/CrFixedFunction.h"

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

typedef CrFixedFunction<4, void(CrGPUDeletable*)> CrGPUDeletionCallbackType;

class ICrRenderDevice
{
public:

	ICrRenderDevice(const ICrRenderSystem* renderSystem);

	virtual ~ICrRenderDevice();

	void InitializeDeletionQueue();

	void ProcessDeletionQueue();

	void FinalizeDeletionQueue();

	// Resource Creation Functions

	CrCommandQueueSharedHandle CreateCommandQueue(CrCommandQueueType::T type);

	CrIndexBufferSharedHandle CreateIndexBuffer(cr3d::DataFormat::T dataFormat, uint32_t numIndices);

	CrSamplerSharedHandle CreateSampler(const CrSamplerDescriptor& descriptor);

	CrSwapchainSharedHandle CreateSwapchain(const CrSwapchainDescriptor& swapchainDescriptor);

	CrTextureSharedHandle CreateTexture(const CrTextureDescriptor& descriptor);

	CrVertexBufferSharedHandle CreateVertexBuffer(uint32_t numVertices, uint32_t stride);

	template<typename Metadata>
	CrStructuredBufferSharedHandle<Metadata> CreateStructuredBuffer(cr3d::BufferAccess::T access, uint32_t numElements);

	CrDataBufferSharedHandle CreateDataBuffer(cr3d::BufferAccess::T access, cr3d::DataFormat::T dataFormat, uint32_t numElements);

	CrGraphicsShaderHandle CreateGraphicsShader(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor) const;

	CrComputeShaderHandle CreateComputeShader(const CrComputeShaderDescriptor& computeShaderDescriptor) const;

	CrGraphicsPipelineHandle CreateGraphicsPipeline(const CrGraphicsPipelineDescriptor& pipelineDescriptor, const ICrGraphicsShader* graphicsShader, const CrVertexDescriptor& vertexDescriptor);
	
	CrComputePipelineHandle CreateComputePipeline(const CrComputePipelineDescriptor& pipelineDescriptor, const ICrComputeShader* computeShader);

	ICrHardwareGPUBuffer* CreateHardwareGPUBuffer(const CrHardwareGPUBufferDescriptor& descriptor);

	CrGPUFenceSharedHandle CreateGPUFence();

	CrGPUSemaphoreSharedHandle CreateGPUSemaphore();

	// GPU Synchronization functions

	cr3d::GPUFenceResult WaitForFence(ICrGPUFence* fence, uint64_t timeoutNanoseconds) const;

	cr3d::GPUFenceResult GetFenceStatus(ICrGPUFence* fence) const;

	void ResetFence(ICrGPUFence* fence);

	// Wait until all operations on all queues have completed
	void WaitIdle();

	virtual bool GetIsFeatureSupported(CrRenderingFeature::T feature) const = 0;

	const CrRenderDeviceProperties& GetProperties() const;

	const CrCommandQueueSharedHandle& GetMainCommandQueue() const;

	const CrGPUDeletionCallbackType& GetGPUDeletionCallback() const
	{
		return m_gpuDeletionCallback;
	}

protected:

	virtual ICrCommandQueue* CreateCommandQueuePS(CrCommandQueueType::T type) = 0;

	virtual ICrGPUFence* CreateGPUFencePS() = 0;

	virtual ICrGPUSemaphore* CreateGPUSemaphorePS() = 0;

	virtual ICrGraphicsShader* CreateGraphicsShaderPS(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor) const = 0;

	virtual ICrComputeShader* CreateComputeShaderPS(const CrComputeShaderDescriptor& computeShaderDescriptor) const = 0;

	virtual ICrHardwareGPUBuffer* CreateHardwareGPUBufferPS(const CrHardwareGPUBufferDescriptor& params) = 0;

	virtual ICrSampler* CreateSamplerPS(const CrSamplerDescriptor& descriptor) = 0;

	virtual ICrSwapchain* CreateSwapchainPS(const CrSwapchainDescriptor& swapchainDescriptor) = 0;

	virtual ICrTexture* CreateTexturePS(const CrTextureDescriptor& descriptor) = 0;
	
	virtual ICrGraphicsPipeline* CreateGraphicsPipelinePS(const CrGraphicsPipelineDescriptor& psoDescriptor, const ICrGraphicsShader* graphicsShader, const CrVertexDescriptor& vertexDescriptor) = 0;

	virtual ICrComputePipeline* CreateComputePipelinePS(const CrComputePipelineDescriptor& pipelineDescriptor, const ICrComputeShader* computeShader) = 0;
	
	virtual cr3d::GPUFenceResult WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds) const = 0;

	virtual cr3d::GPUFenceResult GetFenceStatusPS(const ICrGPUFence* fence) const = 0;

	virtual void WaitIdlePS() = 0;

	virtual void ResetFencePS(const ICrGPUFence* fence) = 0;

	void StorePipelineCache(void* pipelineCacheData, size_t pipelineCacheSize);

	void LoadPipelineCache(CrVector<char>& pipelineCacheData);

	CrGPUDeletionCallbackType m_gpuDeletionCallback = [this](CrGPUDeletable* deletable) { m_gpuDeletionQueue.AddToQueue(deletable); };

	CrGPUDeletionQueue m_gpuDeletionQueue;

	const ICrRenderSystem* m_renderSystem;

	CrRenderDeviceProperties m_renderDeviceProperties;

	CrCommandQueueSharedHandle m_mainCommandQueue;

	CrVector<CrCommandQueueSharedHandle> m_commandQueues;

	CrString m_pipelineCacheDirectory;

	CrString m_pipelineCacheFilename;
};

template<typename Metadata>
CrStructuredBufferSharedHandle<Metadata> ICrRenderDevice::CreateStructuredBuffer(cr3d::BufferAccess::T access, uint32_t numElements)
{
	return CrStructuredBufferSharedHandle<Metadata>(new CrStructuredBuffer<Metadata>(this, access, numElements));
}

inline const CrCommandQueueSharedHandle& ICrRenderDevice::GetMainCommandQueue() const
{
	return m_mainCommandQueue;
}