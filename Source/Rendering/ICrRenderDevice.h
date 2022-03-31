#pragma once

#include "Rendering/CrRendering.h"

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrGPUDeletionQueue.h"

#include "Core/Containers/CrVector.h"
#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/String/CrFixedString.h"
#include "Core/String/CrString.h"
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
		Count
	};
}

struct CrRenderDeviceProperties
{
	CrVendor::T vendor = CrVendor::Unknown;
	cr3d::GraphicsApi::T graphicsApi = cr3d::GraphicsApi::Count;
	CrFixedString128 description;

	uint32_t maxConstantBufferRange = 0;
	uint32_t maxTextureDimension1D = 0;
	uint32_t maxTextureDimension2D = 0;
	uint32_t maxTextureDimension3D = 0;

	uint64_t gpuMemoryBytes = 0;

	bool supportedFeatures[CrRenderingFeature::Count] = {};
};

namespace CrCommandQueueType { enum T : uint32_t; }

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

	void Initialize();

	void ProcessDeletionQueue();

	void FinalizeDeletion();

	//----------------------------
	// Resource Creation Functions
	//----------------------------

	CrCommandBufferSharedHandle CreateCommandBuffer(CrCommandQueueType::T type);

	CrIndexBufferSharedHandle CreateIndexBuffer(cr3d::MemoryAccess::T access, cr3d::DataFormat::T dataFormat, uint32_t numIndices);

	CrSamplerSharedHandle CreateSampler(const CrSamplerDescriptor& descriptor);

	CrSwapchainSharedHandle CreateSwapchain(const CrSwapchainDescriptor& swapchainDescriptor);

	CrTextureSharedHandle CreateTexture(const CrTextureDescriptor& descriptor);

	CrVertexBufferSharedHandle CreateVertexBuffer(cr3d::MemoryAccess::T access, const CrVertexDescriptor& vertexDescriptor, uint32_t numVertices);

	template<typename Metadata>
	CrStructuredBufferSharedHandle<Metadata> CreateStructuredBuffer(cr3d::MemoryAccess::T access, uint32_t numElements);

	CrDataBufferSharedHandle CreateDataBuffer(cr3d::MemoryAccess::T access, cr3d::DataFormat::T dataFormat, uint32_t numElements);

	CrGraphicsShaderHandle CreateGraphicsShader(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor);

	CrComputeShaderHandle CreateComputeShader(const CrComputeShaderDescriptor& computeShaderDescriptor);

	CrGraphicsPipelineHandle CreateGraphicsPipeline(const CrGraphicsPipelineDescriptor& pipelineDescriptor, const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor);
	
	CrComputePipelineHandle CreateComputePipeline(const CrComputePipelineDescriptor& pipelineDescriptor, const CrComputeShaderHandle& computeShader);

	CrGPUQueryPoolHandle CreateGPUQueryPool(const CrGPUQueryPoolDescriptor& queryPoolDescriptor);

	CrGPUHardwareBufferHandle CreateHardwareGPUBuffer(const CrHardwareGPUBufferDescriptor& descriptor);

	ICrHardwareGPUBuffer* CreateHardwareGPUBufferPointer(const CrHardwareGPUBufferDescriptor& descriptor);

	CrGPUFenceSharedHandle CreateGPUFence();

	CrGPUSemaphoreSharedHandle CreateGPUSemaphore();

	//------------------------------
	// GPU Synchronization functions
	//------------------------------

	cr3d::GPUFenceResult WaitForFence(ICrGPUFence* fence, uint64_t timeoutNanoseconds);

	cr3d::GPUFenceResult GetFenceStatus(ICrGPUFence* fence) const;

	void SignalFence(CrCommandQueueType::T queueType, const ICrGPUFence* signalFence);

	void ResetFence(ICrGPUFence* fence);

	// Wait until all operations on all queues have completed
	void WaitIdle();

	//-------------------------------
	// Properties and feature support
	//-------------------------------

	const CrRenderDeviceProperties& GetProperties() const;


	void SubmitCommandBuffer(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence);

	const CrCommandBufferSharedHandle& GetAuxiliaryCommandBuffer() const;

	const CrGPUDeletionCallbackType& GetGPUDeletionCallback() const
	{
		return m_gpuDeletionCallback;
	}

protected:

	virtual ICrCommandBuffer* CreateCommandBufferPS(CrCommandQueueType::T type) = 0;

	virtual ICrGPUFence* CreateGPUFencePS() = 0;

	virtual ICrGPUSemaphore* CreateGPUSemaphorePS() = 0;

	virtual ICrGraphicsShader* CreateGraphicsShaderPS(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor) = 0;

	virtual ICrComputeShader* CreateComputeShaderPS(const CrComputeShaderDescriptor& computeShaderDescriptor) = 0;

	virtual ICrHardwareGPUBuffer* CreateHardwareGPUBufferPS(const CrHardwareGPUBufferDescriptor& params) = 0;

	virtual ICrSampler* CreateSamplerPS(const CrSamplerDescriptor& descriptor) = 0;

	virtual ICrSwapchain* CreateSwapchainPS(const CrSwapchainDescriptor& swapchainDescriptor) = 0;

	virtual ICrTexture* CreateTexturePS(const CrTextureDescriptor& descriptor) = 0;
	
	virtual ICrGraphicsPipeline* CreateGraphicsPipelinePS(const CrGraphicsPipelineDescriptor& psoDescriptor, const ICrGraphicsShader* graphicsShader, const CrVertexDescriptor& vertexDescriptor) = 0;

	virtual ICrComputePipeline* CreateComputePipelinePS(const CrComputePipelineDescriptor& pipelineDescriptor, const ICrComputeShader* computeShader) = 0;
	
	virtual ICrGPUQueryPool* CreateGPUQueryPoolPS(const CrGPUQueryPoolDescriptor& queryPoolDescriptor) = 0;

	// Synchronization

	virtual cr3d::GPUFenceResult WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds) = 0;

	virtual cr3d::GPUFenceResult GetFenceStatusPS(const ICrGPUFence* fence) const = 0;

	virtual void SignalFencePS(CrCommandQueueType::T queueType, const ICrGPUFence* signalFence) = 0;

	virtual void ResetFencePS(const ICrGPUFence* fence) = 0;

	virtual void WaitIdlePS() = 0;

	virtual void SubmitCommandBufferPS(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence) = 0;
	void StorePipelineCache(void* pipelineCacheData, size_t pipelineCacheSize);

	void LoadPipelineCache(CrVector<char>& pipelineCacheData);

	CrGPUDeletionCallbackType m_gpuDeletionCallback = [this](CrGPUDeletable* deletable) { m_gpuDeletionQueue.AddToQueue(deletable); };

	CrGPUDeletionQueue m_gpuDeletionQueue;

	const ICrRenderSystem* m_renderSystem;

	CrRenderDeviceProperties m_renderDeviceProperties;

	CrCommandBufferSharedHandle m_auxiliaryCommandBuffer;

	CrVector<CrCommandQueueSharedHandle> m_commandQueues;

	// The platform-specific code is able to determine whether
	// the pipeline is valid or not
	bool m_isValidPipelineCache;

	CrString m_pipelineCacheDirectory;

	CrString m_pipelineCacheFilename;
};

template<typename Metadata>
CrStructuredBufferSharedHandle<Metadata> ICrRenderDevice::CreateStructuredBuffer(cr3d::MemoryAccess::T access, uint32_t numElements)
{
	return CrStructuredBufferSharedHandle<Metadata>(new CrStructuredBuffer<Metadata>(this, access, numElements));
}

inline const CrCommandBufferSharedHandle& ICrRenderDevice::GetAuxiliaryCommandBuffer() const
{
	return m_auxiliaryCommandBuffer;
}