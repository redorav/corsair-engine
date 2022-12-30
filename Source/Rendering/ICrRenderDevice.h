#pragma once

#include "Rendering/CrRendering.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

#include "Core/Containers/CrVector.h"
#include "Core/String/CrFixedString.h"
#include "Core/String/CrString.h"
#include "Core/Function/CrFixedFunction.h"
#include "Core/Containers/CrHashMap.h"
#include "Core/CrHash.h"
#include "Core/SmartPointers/CrUniquePtr.h"
#include "Core/SmartPointers/CrIntrusivePtr.h"

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

struct CrDriverVersion
{
	CrDriverVersion() = default;

	uint32_t major = 0;

	uint32_t minor = 0;

	uint32_t patch = 0;
};

struct CrRenderDeviceProperties
{
	// We can choose to search for a vendor. If we don't have it, we choose the best available one
	cr3d::GraphicsVendor::T preferredVendor = cr3d::GraphicsVendor::Unknown;

	// Actual vendor we selected
	cr3d::GraphicsVendor::T vendor = cr3d::GraphicsVendor::Unknown;

	// Graphics API
	cr3d::GraphicsApi::T graphicsApi = cr3d::GraphicsApi::Count;

	// Just for display
	CrFixedString16 graphicsApiDisplay;

	// Version of the driver
	CrDriverVersion driverVersion;

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

// Texture uploads encapsulate the idea that platforms have
// an optimal texture format but for some (PC mainly) we can
// only provide the data in a linear format
struct CrTextureUpload
{
	~CrTextureUpload();

	CrHardwareGPUBufferHandle buffer;
	ICrTexture* texture;
	uint32_t mipmapStart;
	uint32_t mipmapCount;
	uint32_t sliceStart;
	uint32_t sliceCount;
};

struct CrBufferUpload
{
	~CrBufferUpload();

	CrHardwareGPUBufferHandle stagingBuffer;
	const ICrHardwareGPUBuffer* destinationBuffer;
	uint32_t sizeBytes;
	uint32_t sourceOffsetBytes;
	uint32_t destinationOffsetBytes;
};

class CrGPUDeletionQueue;
class CrGPUTransferCallbackQueue;
class CrGPUDeletable;
typedef CrFixedFunction<4, void(CrGPUDeletable*)> CrGPUDeletionCallbackType;

struct CrRenderDeviceDescriptor
{
	cr3d::GraphicsVendor::T preferredVendor = cr3d::GraphicsVendor::Unknown;
};

class ICrRenderDevice : public CrIntrusivePtrInterfaceBase
{
public:

	ICrRenderDevice(const ICrRenderSystem* renderSystem, const CrRenderDeviceDescriptor& descriptor);

	virtual ~ICrRenderDevice();

	template<typename T>
	void delete_callback()
	{
		FinalizeDeletion();
		delete this;
	}

	void Initialize();

	void ProcessDeletionQueue();

	void FinalizeDeletion();

	void ProcessQueuedCommands();

	const CrCommandBufferHandle& GetAuxiliaryCommandBuffer();

	//------------------
	// Resource Creation
	//------------------

	CrCommandBufferHandle CreateCommandBuffer(const CrCommandBufferDescriptor& descriptor);

	CrIndexBufferHandle CreateIndexBuffer(cr3d::MemoryAccess::T access, cr3d::DataFormat::T dataFormat, uint32_t numIndices);

	CrSamplerHandle CreateSampler(const CrSamplerDescriptor& descriptor);

	CrSwapchainHandle CreateSwapchain(const CrSwapchainDescriptor& swapchainDescriptor);

	CrTextureHandle CreateTexture(const CrTextureDescriptor& descriptor);

	CrVertexBufferHandle CreateVertexBuffer(cr3d::MemoryAccess::T access, const CrVertexDescriptor& vertexDescriptor, uint32_t numVertices);

	template<typename Metadata>
	CrStructuredBufferHandle<Metadata> CreateStructuredBuffer(cr3d::MemoryAccess::T access, uint32_t numElements);

	CrDataBufferHandle CreateDataBuffer(cr3d::MemoryAccess::T access, cr3d::DataFormat::T dataFormat, uint32_t numElements);

	CrGraphicsShaderHandle CreateGraphicsShader(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor);

	CrComputeShaderHandle CreateComputeShader(const CrComputeShaderDescriptor& computeShaderDescriptor);

	CrGraphicsPipelineHandle CreateGraphicsPipeline(const CrGraphicsPipelineDescriptor& pipelineDescriptor, const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor);
	
	CrComputePipelineHandle CreateComputePipeline(const CrComputePipelineDescriptor& pipelineDescriptor, const CrComputeShaderHandle& computeShader);

	CrGPUQueryPoolHandle CreateGPUQueryPool(const CrGPUQueryPoolDescriptor& queryPoolDescriptor);

	CrHardwareGPUBufferHandle CreateHardwareGPUBuffer(const CrHardwareGPUBufferDescriptor& descriptor);

	CrGPUFenceHandle CreateGPUFence();

	CrGPUSemaphoreHandle CreateGPUSemaphore();

	void AddToDeletionQueue(CrGPUDeletable* resource);

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

	uint8_t* BeginBufferUpload(const ICrHardwareGPUBuffer* destinationBuffer);

	void EndBufferUpload(const ICrHardwareGPUBuffer* destinationBuffer);

	void DownloadBuffer(const ICrHardwareGPUBuffer* buffer, const CrGPUTransferCallbackType& callback);

	//-------------------------------
	// Properties and feature support
	//-------------------------------

	const CrRenderDeviceProperties& GetProperties() const;

	void SubmitCommandBuffer(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence);

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

	virtual void FinalizeDeletionPS() {}

	//--------------------
	// GPU Synchronization
	//--------------------

	virtual cr3d::GPUFenceResult WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds) = 0;

	virtual cr3d::GPUFenceResult GetFenceStatusPS(const ICrGPUFence* fence) const = 0;

	virtual void SignalFencePS(CrCommandQueueType::T queueType, const ICrGPUFence* signalFence) = 0;

	virtual void ResetFencePS(const ICrGPUFence* fence) = 0;

	virtual void WaitIdlePS() = 0;

	//--------------------
	// Download and Upload
	//--------------------

	// Begins a texture upload. Prepares a buffer and returns a pointer to the beginning of the memory.
	// External code then populates the given memory
	virtual uint8_t* BeginTextureUploadPS(const ICrTexture* texture) = 0;

	// Ends a texture upload. The render device keeps track of the requested upload and matches it to
	// schedule an upload that is guaranteed to be visible on the next texture usage
	virtual void EndTextureUploadPS(const ICrTexture* texture) = 0;

	virtual uint8_t* BeginBufferUploadPS(const ICrHardwareGPUBuffer* destinationBuffer) = 0;

	virtual void EndBufferUploadPS(const ICrHardwareGPUBuffer* destinationBuffer) = 0;

	virtual CrHardwareGPUBufferHandle DownloadBufferPS(const ICrHardwareGPUBuffer* sourceBuffer) = 0;

	virtual void SubmitCommandBufferPS(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence) = 0;

	void StorePipelineCache(void* pipelineCacheData, size_t pipelineCacheSize);

	void LoadPipelineCache(CrVector<char>& pipelineCacheData);

	CrGPUDeletionCallbackType m_gpuDeletionCallback;

	CrUniquePtr<CrGPUDeletionQueue> m_gpuDeletionQueue;

	CrUniquePtr<CrGPUTransferCallbackQueue> m_gpuTransferCallbackQueue;

	const ICrRenderSystem* m_renderSystem;

	CrRenderDeviceProperties m_renderDeviceProperties;

	// Texture uploads that have started but haven't been committed yet
	CrHashMap<CrHash, CrTextureUpload> m_openTextureUploads;

	// Buffer uploads that have started but haven't been committed yet
	CrHashMap<CrHash, CrBufferUpload> m_openBufferUploads;

	// The platform-specific code is able to determine whether
	// the pipeline is valid or not
	bool m_isValidPipelineCache;

	CrString m_pipelineCacheDirectory;

	CrString m_pipelineCacheFilename;

private:

	// Auxiliary command buffers. Subclasses don't need to know about the implementation details,
	// they queue work onto the auxiliary command buffer (via the getter)
	CrCommandBufferHandle m_auxiliaryCommandBuffer;

	CrVector<CrCommandBufferHandle> m_auxiliaryCommandBuffers;

	uint32_t m_auxiliaryCommandBufferIndex = 0;

	uint32_t m_auxiliaryCommandBufferCount = 0;
};

template<typename Metadata>
CrStructuredBufferHandle<Metadata> ICrRenderDevice::CreateStructuredBuffer(cr3d::MemoryAccess::T access, uint32_t numElements)
{
	return CrStructuredBufferHandle<Metadata>(new CrStructuredBuffer<Metadata>(this, access, numElements));
}