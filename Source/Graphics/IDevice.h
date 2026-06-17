#pragma once

#include "Graphics/CrRendering.h"

#include "Graphics/CrGraphicsForwardDeclarations.h"

#include "Core/CrHash.h"

#include "crstl/fixed_string.h"
#include "crstl/intrusive_ptr.h"
#include "crstl/open_hashmap.h"
#include "crstl/string.h"
#include "crstl/unique_ptr.h"
#include "crstl/vector.h"

namespace crgfx
{
	struct DriverVersion
	{
		DriverVersion() = default;

		uint32_t major = 0;

		uint32_t minor = 0;

		uint32_t patch = 0;
	};

	struct DeviceProperties
	{
		// We can choose to search for a vendor. If we don't have it, we choose the best available one
		crgfx::GraphicsVendor::T preferredVendor = crgfx::GraphicsVendor::Unknown;

		// Actual vendor we selected
		crgfx::GraphicsVendor::T vendor = crgfx::GraphicsVendor::Unknown;

		// Graphics API
		crgfx::GraphicsApi::T graphicsApi = crgfx::GraphicsApi::Count;

		// Just for display
		crstl::fixed_string32 graphicsApiDisplay;

		// Version of the driver
		crgfx::DriverVersion driverVersion;

		bool isUMA = false; // Whether a unified memory architecture is used here

		crstl::fixed_string128 description;

		uint32_t maxConstantBufferRange = 0;
		uint32_t maxTextureDimension1D = 0;
		uint32_t maxTextureDimension2D = 0;
		uint32_t maxTextureDimension3D = 0;

		uint64_t gpuMemoryBytes = 0;

		struct features
		{
			bool raytracing = false;
			bool geometryShaders = false;
			bool tessellation = false;
			bool meshShaders = false;
			bool compressionBC = false;
			bool compressionETC = false;
			bool compressionASTC = false;
			bool conservativeRasterization = false;
			bool textureFormatCasting = false;
		} features;
	};
};

// Texture uploads encapsulate the idea that platforms have
// an optimal texture format but for some (PC mainly) we can
// only provide the data in a linear format
struct CrTextureUpload
{
	crgfx::CrHardwareGPUBufferHandle stagingBuffer;
	crgfx::ITexture* texture;
	uint32_t mipmapStart;
	uint32_t mipmapCount;
	uint32_t sliceStart;
	uint32_t sliceCount;
};

struct CrBufferUpload
{
	crgfx::CrHardwareGPUBufferHandle stagingBuffer;
	const crgfx::ICrHardwareGPUBuffer* destinationBuffer;
	uint32_t sizeBytes;
	uint32_t sourceOffsetBytes;
	uint32_t destinationOffsetBytes;
};

class CrGPUDeletionQueue;
class CrGPUTransferCallbackQueue;
class CrGPUDeletable;

namespace crgfx
{
	struct DeviceDescriptor
	{
		crgfx::GraphicsVendor::T preferredVendor = crgfx::GraphicsVendor::Unknown;
	};

	class IDevice : public crstl::intrusive_ptr_interface_base
	{
	public:

		IDevice(IGraphicsSystem* renderSystem, const crgfx::DeviceDescriptor& descriptor);

		virtual ~IDevice();

		template<typename T>
		void intrusive_ptr_delete_callback()
		{
			FinalizeDeletion();
			delete this;
		}

		void Initialize();

		void ProcessDeletionQueue();

		void FinalizeDeletion();

		void ProcessQueuedCommands();

		const CommandBufferHandle& GetAuxiliaryCommandBuffer();

		//------------------
		// Resource Creation
		//------------------

		crgfx::ICommandBuffer* CreateCommandBuffer(const crgfx::CommandBufferDescriptor& descriptor);

		CrIndexBuffer* CreateIndexBuffer(crgfx::MemoryAccess::T access, crgfx::DataFormat::T dataFormat, uint32_t numIndices);

		ISampler* CreateSampler(const crgfx::SamplerDescriptor& descriptor);

		ISwapchain* CreateSwapchain(const crgfx::SwapchainDescriptor& swapchainDescriptor);

		crgfx::ITexture* CreateTexture(const crgfx::TextureDescriptor& descriptor);

		CrVertexBuffer* CreateVertexBuffer(crgfx::MemoryAccess::T access, const CrVertexDescriptor& vertexDescriptor, uint32_t numVertices);

		template<typename Metadata>
		CrStructuredBuffer<Metadata>* CreateStructuredBuffer(crgfx::MemoryAccess::T access, uint32_t numElements);

		CrTypedBuffer* CreateTypedBuffer(crgfx::MemoryAccess::T access, crgfx::DataFormat::T dataFormat, uint32_t numElements);

		IGraphicsShader* CreateGraphicsShader(const GraphicsShaderDescriptor& graphicsShaderDescriptor);

		IComputeShader* CreateComputeShader(const ComputeShaderDescriptor& computeShaderDescriptor);

		GraphicsPipelineHandle CreateGraphicsPipeline(const GraphicsPipelineDescriptor& pipelineDescriptor, const crgfx::GraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor);

		ComputePipelineHandle CreateComputePipeline(const ComputeShaderHandle& computeShader);

		IGPUQueryPool* CreateGPUQueryPool(const GPUQueryPoolDescriptor& queryPoolDescriptor);

		ICrHardwareGPUBuffer* CreateHardwareGPUBuffer(const CrHardwareGPUBufferDescriptor& descriptor);

		IGPUFence* CreateGPUFence(bool signaled = false);

		IGPUSemaphore* CreateGPUSemaphore();

		void AddToDeletionQueue(CrGPUDeletable* resource);

		//--------------------
		// GPU Synchronization
		//--------------------

		crgfx::GPUFenceResult WaitForFence(IGPUFence* fence, uint64_t timeoutNanoseconds);

		crgfx::GPUFenceResult GetFenceStatus(IGPUFence* fence) const;

		void SignalFence(crgfx::CommandQueueType::T queueType, const IGPUFence* signalFence);

		void ResetFence(IGPUFence* fence);

		// Wait until all operations on all queues have completed
		void WaitIdle();

		//--------------------
		// Download and Upload
		//--------------------

		uint8_t* BeginTextureUpload(const crgfx::ITexture* texture);

		void EndTextureUpload(const crgfx::ITexture* texture);

		uint8_t* BeginBufferUpload(const ICrHardwareGPUBuffer* destinationBuffer);

		void EndBufferUpload(const ICrHardwareGPUBuffer* destinationBuffer);

		void DownloadBuffer(const ICrHardwareGPUBuffer* buffer, const CrGPUTransferCallbackType& callback);

		//-------------------------------
		// Properties and feature support
		//-------------------------------

		const DeviceProperties& GetProperties() const;

		bool SupportsConservativeRasterization() const { return m_deviceProperties.features.conservativeRasterization; }

		bool SupportsTextureFormatCasting() const { return m_deviceProperties.features.textureFormatCasting; }

		void SubmitCommandBuffer(const crgfx::ICommandBuffer* commandBuffer, const IGPUSemaphore* waitSemaphore, const IGPUSemaphore* signalSemaphore, const IGPUFence* signalFence);

	protected:

		virtual crgfx::ICommandBuffer* CreateCommandBufferPS(const crgfx::CommandBufferDescriptor& descriptor) = 0;

		virtual IGPUFence* CreateGPUFencePS(bool signaled) = 0;

		virtual IGPUSemaphore* CreateGPUSemaphorePS() = 0;

		virtual IGraphicsShader* CreateGraphicsShaderPS(const GraphicsShaderDescriptor& graphicsShaderDescriptor) = 0;

		virtual IComputeShader* CreateComputeShaderPS(const ComputeShaderDescriptor& computeShaderDescriptor) = 0;

		virtual ICrHardwareGPUBuffer* CreateHardwareGPUBufferPS(const CrHardwareGPUBufferDescriptor& params) = 0;

		virtual ISampler* CreateSamplerPS(const crgfx::SamplerDescriptor& descriptor) = 0;

		virtual ISwapchain* CreateSwapchainPS(const crgfx::SwapchainDescriptor& swapchainDescriptor) = 0;

		virtual ITexture* CreateTexturePS(const crgfx::TextureDescriptor& descriptor) = 0;

		virtual IGraphicsPipeline* CreateGraphicsPipelinePS(const GraphicsPipelineDescriptor& psoDescriptor, const crgfx::GraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor) = 0;

		virtual IComputePipeline* CreateComputePipelinePS(const ComputeShaderHandle& computeShader) = 0;

		virtual IGPUQueryPool* CreateGPUQueryPoolPS(const GPUQueryPoolDescriptor& queryPoolDescriptor) = 0;

		virtual void FinalizeDeletionPS() {}

		//--------------------
		// GPU Synchronization
		//--------------------

		virtual crgfx::GPUFenceResult WaitForFencePS(const IGPUFence* fence, uint64_t timeoutNanoseconds) = 0;

		virtual crgfx::GPUFenceResult GetFenceStatusPS(const IGPUFence* fence) const = 0;

		virtual void SignalFencePS(crgfx::CommandQueueType::T queueType, const IGPUFence* signalFence) = 0;

		virtual void ResetFencePS(const IGPUFence* fence) = 0;

		virtual void WaitIdlePS() = 0;

		//--------------------
		// Download and Upload
		//--------------------

		// Begins a texture upload. Prepares a buffer and returns a pointer to the beginning of the memory.
		// External code then populates the given memory
		virtual uint8_t* BeginTextureUploadPS(const crgfx::ITexture* texture) = 0;

		// Ends a texture upload. The render device keeps track of the requested upload and matches it to
		// schedule an upload that is guaranteed to be visible on the next texture usage
		virtual void EndTextureUploadPS(const crgfx::ITexture* texture) = 0;

		virtual uint8_t* BeginBufferUploadPS(const ICrHardwareGPUBuffer* destinationBuffer) = 0;

		virtual void EndBufferUploadPS(const ICrHardwareGPUBuffer* destinationBuffer) = 0;

		virtual CrHardwareGPUBufferHandle DownloadBufferPS(const ICrHardwareGPUBuffer* sourceBuffer) = 0;

		virtual void SubmitCommandBufferPS(const crgfx::ICommandBuffer* commandBuffer, const IGPUSemaphore* waitSemaphore, const IGPUSemaphore* signalSemaphore, const IGPUFence* signalFence) = 0;

		void StorePipelineCache(void* pipelineCacheData, size_t pipelineCacheSize);

		void LoadPipelineCache(crstl::vector<char>& pipelineCacheData);

		crstl::unique_ptr<CrGPUDeletionQueue> m_gpuDeletionQueue;

		crstl::unique_ptr<CrGPUTransferCallbackQueue> m_gpuTransferCallbackQueue;

		DeviceProperties m_deviceProperties;

		// Texture uploads that have started but haven't been committed yet
		crstl::open_hashmap<CrHash, CrTextureUpload> m_openTextureUploads;

		// Buffer uploads that have started but haven't been committed yet
		crstl::open_hashmap<CrHash, CrBufferUpload> m_openBufferUploads;

		//--------------------------
		// Pipeline State Management
		//--------------------------

		// The platform-specific code is able to determine whether
		// the pipeline is valid or not
		bool m_isValidPipelineCache;

		crstl::string m_pipelineCacheDirectory;

		crstl::string m_pipelineCacheFilename;

		crstl::open_hashmap<uint64_t, GraphicsPipelineHandle> m_graphicsPipelines;

		crstl::open_hashmap<uint64_t, ComputePipelineHandle> m_computePipelines;

	private:

		// Auxiliary command buffers. Subclasses don't need to know about the implementation details,
		// they queue work onto the auxiliary command buffer (via the getter)
		CommandBufferHandle m_auxiliaryCommandBuffer;

		crstl::vector<CommandBufferHandle> m_auxiliaryCommandBuffers;

		uint32_t m_auxiliaryCommandBufferIndex = 0;

		uint32_t m_auxiliaryCommandBufferCount = 0;
	};

	template<typename Metadata>
	CrStructuredBuffer<Metadata>* IDevice::CreateStructuredBuffer(crgfx::MemoryAccess::T access, uint32_t numElements)
	{
		return new CrStructuredBuffer<Metadata>(this, access, numElements);
	}
};