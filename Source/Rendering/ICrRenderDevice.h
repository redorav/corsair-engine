#pragma once

#include "Rendering/CrRendering.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

#include "Core/Containers/CrVector.h"
#include "Core/SmartPointers/CrSharedPtr.h"
#include "Core/String/CrFixedString.h"
#include "Core/String/CrString.h"
#include "Core/Function/CrFixedFunction.h"
#include "Core/Containers/CrHashMap.h"
#include "Core/CrHash.h"

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
	bool isUMA = false; // Whether a unified memory architecture is used here
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

// Texture uploads encapsulate the idea that platforms have
// an optimal texture format but for some (PC mainly) we can
// only provide the data in a linear format
struct CrTextureUpload
{
	CrGPUHardwareBufferHandle buffer;
	ICrTexture* texture;
	uint32_t mipmapStart;
	uint32_t mipmapCount;
	uint32_t sliceStart;
	uint32_t sliceCount;
};

class CrGPUDeletionQueue;
class CrGPUDeletable;
typedef CrFixedFunction<4, void(CrGPUDeletable*)> CrGPUDeletionCallbackType;

class ICrRenderDevice
{
public:

	ICrRenderDevice(const ICrRenderSystem* renderSystem);

	virtual ~ICrRenderDevice();

	void Initialize();

	void ProcessDeletionQueue();

	void FinalizeDeletion();

	void ProcessQueuedCommands();

	const CrCommandBufferSharedHandle& GetAuxiliaryCommandBuffer();

	//------------------
	// Resource Creation
	//------------------

	CrCommandBufferSharedHandle CreateCommandBuffer(const CrCommandBufferDescriptor& descriptor);

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

	//--------------------
	// GPU Synchronization
	//--------------------

	cr3d::GPUFenceResult WaitForFence(ICrGPUFence* fence, uint64_t timeoutNanoseconds);

	cr3d::GPUFenceResult GetFenceStatus(ICrGPUFence* fence) const;

	void SignalFence(CrCommandQueueType::T queueType, const ICrGPUFence* signalFence);

	void ResetFence(ICrGPUFence* fence);

	// Wait until all operations on all queues have completed
	void WaitIdle();

	//--------------------
	// Download and Upload
	//--------------------
	
	uint8_t* BeginTextureUpload(const ICrTexture* texture);

	void EndTextureUpload(const ICrTexture* texture);

	//-------------------------------
	// Properties and feature support
	//-------------------------------

	const CrRenderDeviceProperties& GetProperties() const;

	void SubmitCommandBuffer(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence);

	const CrGPUDeletionCallbackType& GetGPUDeletionCallback() const
	{
		return m_gpuDeletionCallback;
	}

protected:

	virtual ICrCommandBuffer* CreateCommandBufferPS(const CrCommandBufferDescriptor& descriptor) = 0;

	virtual ICrGPUFence* CreateGPUFencePS() = 0;

	virtual ICrGPUSemaphore* CreateGPUSemaphorePS() = 0;

	virtual ICrGraphicsShader* CreateGraphicsShaderPS(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor) = 0;

	virtual ICrComputeShader* CreateComputeShaderPS(const CrComputeShaderDescriptor& computeShaderDescriptor) = 0;

	virtual ICrHardwareGPUBuffer* CreateHardwareGPUBufferPS(const CrHardwareGPUBufferDescriptor& params) = 0;

	virtual ICrSampler* CreateSamplerPS(const CrSamplerDescriptor& descriptor) = 0;

	virtual ICrSwapchain* CreateSwapchainPS(const CrSwapchainDescriptor& swapchainDescriptor) = 0;

	virtual ICrTexture* CreateTexturePS(const CrTextureDescriptor& descriptor) = 0;
	
	virtual ICrGraphicsPipeline* CreateGraphicsPipelinePS(const CrGraphicsPipelineDescriptor& psoDescriptor, const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor) = 0;

	virtual ICrComputePipeline* CreateComputePipelinePS(const CrComputePipelineDescriptor& pipelineDescriptor, const CrComputeShaderHandle& computeShader) = 0;
	
	virtual ICrGPUQueryPool* CreateGPUQueryPoolPS(const CrGPUQueryPoolDescriptor& queryPoolDescriptor) = 0;

	// Synchronization

	virtual cr3d::GPUFenceResult WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds) = 0;

	virtual cr3d::GPUFenceResult GetFenceStatusPS(const ICrGPUFence* fence) const = 0;

	virtual void SignalFencePS(CrCommandQueueType::T queueType, const ICrGPUFence* signalFence) = 0;

	virtual void ResetFencePS(const ICrGPUFence* fence) = 0;

	virtual void WaitIdlePS() = 0;

	//------------------------------
	// Download and Upload functions
	//------------------------------

	// Begins a texture upload. Prepares a buffer and returns a pointer to the beginning of the memory.
	// External code then populates the given memory
	virtual uint8_t* BeginTextureUploadPS(const ICrTexture* texture) = 0;

	// Ends a texture upload. The render device keeps track of the requested upload and matches it to
	// schedule an upload that is guaranteed to be visible on the next texture usage
	virtual void EndTextureUploadPS(const ICrTexture* texture) = 0;

	virtual void SubmitCommandBufferPS(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence) = 0;

	void StorePipelineCache(void* pipelineCacheData, size_t pipelineCacheSize);

	void LoadPipelineCache(CrVector<char>& pipelineCacheData);

	CrGPUDeletionCallbackType m_gpuDeletionCallback;

	CrUniquePtr<CrGPUDeletionQueue> m_gpuDeletionQueue;

	const ICrRenderSystem* m_renderSystem;

	CrRenderDeviceProperties m_renderDeviceProperties;

	// Texture uploads that have begun to be populated with data but haven't been committed yet
	CrHashMap<CrHash, CrTextureUpload> m_openTextureUploads;

	// The platform-specific code is able to determine whether
	// the pipeline is valid or not
	bool m_isValidPipelineCache;

	CrString m_pipelineCacheDirectory;

	CrString m_pipelineCacheFilename;

private:

	// Auxiliary command buffers. Subclasses don't need to know about the implementation details,
	// they queue work onto the auxiliary command buffer (via the getter)
	CrCommandBufferSharedHandle m_auxiliaryCommandBuffer;

	CrVector<CrCommandBufferSharedHandle> m_auxiliaryCommandBuffers;

	uint32_t m_auxiliaryCommandBufferIndex = 0;

	uint32_t m_auxiliaryCommandBufferCount = 0;
};

template<typename Metadata>
CrStructuredBufferSharedHandle<Metadata> ICrRenderDevice::CreateStructuredBuffer(cr3d::MemoryAccess::T access, uint32_t numElements)
{
	return CrStructuredBufferSharedHandle<Metadata>(new CrStructuredBuffer<Metadata>(this, access, numElements));
}