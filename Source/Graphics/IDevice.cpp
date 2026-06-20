#include "Graphics/CrRendering_pch.h"

#include "IGraphicsSystem.h"
#include "IDevice.h"
#include "ICommandBuffer.h"
#include "ISwapchain.h"
#include "ISampler.h"
#include "ITexture.h"
#include "IShader.h"
#include "IPipeline.h"
#include "IGPUQueryPool.h"
#include "GPUBuffer.h"
#include "CrGPUStackAllocator.h"

#include "Core/CrMacros.h"
#include "Core/Logging/ICrDebug.h"
#include "Core/CrGlobalPaths.h"

// TODO Improve this, how to best deal with the frame counter?
// Seems like the wrong place to have it here
#include "Core/CrFrameTime.h"

#include "crstl/deque.h"
#include "crstl/fixed_function.h"
#include "crstl/timer.h"

#if !defined(CR_CONFIG_FINAL)
#define RENDER_DEVICE_LOGS
#endif

namespace crgfx
{
	struct GPUDownloadCallback
	{
		crgfx::GPUTransferCallback callback;
		crgfx::HardwareGPUBufferHandle buffer;
	};

	struct DownloadCallbackList
	{
		crgfx::GPUFenceHandle fence;
		crstl::vector<GPUDownloadCallback> callbacks;
	};

	class GPUTransferCallbackQueue
	{
	public:

		static const uint32_t MaximumCallbackLists = 4;

		void Initialize(crgfx::IDevice* renderDevice);

		void AddToQueue(const GPUDownloadCallback& callback);

		void Process();

	private:

		crgfx::IDevice* m_renderDevice = nullptr;

		// Callbacks to be executed after a successful download operation, copying from GPU to CPU
		DownloadCallbackList* m_currentCallbackList = nullptr;

		// Active callback lists have had fence signal commands scheduled on
		// the GPU, and are waiting to receive that on the CPU. We want to
		// push back, but then get front to query the fence
		crstl::deque<DownloadCallbackList*> m_activeCallbackLists;

		// Available callback lists are there to be reused
		crstl::fixed_vector<DownloadCallbackList*, MaximumCallbackLists> m_availableCallbackLists;

		crstl::fixed_vector<DownloadCallbackList, MaximumCallbackLists> m_callbackLists;
	};

	struct CrDeletionList
	{
		crgfx::GPUFenceHandle fence;
		crstl::vector<GPUDeletable*> deletables;
	};

	// A queue that manages deletion of GPU objects. Anything added to this queue
	// will eventually get destroyed
	class GPUDeletionQueue
	{
	public:

		~GPUDeletionQueue();

		static const uint32_t MaximumDeletionLists = 4;

		void Initialize(crgfx::IDevice* renderDevice);

		void AddToQueue(GPUDeletable* deletable);

		// Processes pending requests and adds current requests so that they're waited on
		// This is intended for in-flight resources during the frame
		void Process();

		void Finalize();

	private:

		crgfx::IDevice* m_renderDevice = nullptr;

		// The current deletion list points to all the resources being deleted before we execute the fence. Once we execute the fence, we add it
		// to the active deletion lists to be waited on
		CrDeletionList* m_currentDeletionList = nullptr;

		// Active deletion lists have had fence signal commands scheduled on the GPU, and are waiting to receive that on the CPU. We want to
		// push back, but then get front to query the fence
		crstl::deque<CrDeletionList*> m_activeDeletionLists;

		// Available deletion lists are there to be reused
		crstl::fixed_vector<CrDeletionList*, MaximumDeletionLists> m_availableDeletionLists;

		// These are the actual contents of the CrDeletionList. The other lists point to the contents here. We cannot
		// have reallocation on these lists so we use fixed size vectors
		crstl::fixed_vector<CrDeletionList, MaximumDeletionLists> m_deletionLists;
	};

	IDevice::IDevice(IGraphicsSystem* renderSystem, const crgfx::DeviceDescriptor& descriptor)
		: m_isValidPipelineCache(false)
	{
		m_pipelineCacheDirectory = CrGlobalPaths::GetTempEngineDirectory() + "Pipeline Cache/";
		m_pipelineCacheDirectory += crgfx::GraphicsApi::ToString(renderSystem->GetGraphicsApi());
		m_pipelineCacheDirectory += "/";
		m_pipelineCacheFilename = "PipelineCache.bin";
		m_deviceProperties.graphicsApi = renderSystem->GetGraphicsApi();
		m_deviceProperties.preferredVendor = descriptor.preferredVendor;

		m_gpuDeletionQueue = crstl::unique_ptr<GPUDeletionQueue>(new GPUDeletionQueue());

		m_gpuTransferCallbackQueue = crstl::unique_ptr<crgfx::GPUTransferCallbackQueue>(new GPUTransferCallbackQueue());
	}

	IDevice::~IDevice()
	{

	}

	void IDevice::Initialize()
	{
		m_gpuDeletionQueue->Initialize(this);

		m_gpuTransferCallbackQueue->Initialize(this);

		m_auxiliaryCommandBufferCount = 3; // TODO Pass in or make dynamic

		for (uint32_t i = 0; i < m_auxiliaryCommandBufferCount; ++i)
		{
			crgfx::CommandBufferDescriptor descriptor;
			descriptor.name.append_sprintf("Render Device Auxiliary Command Buffer %i", i);
			m_auxiliaryCommandBuffers.push_back(CreateCommandBuffer(descriptor));
		}
	}

	const CommandBufferHandle& IDevice::GetAuxiliaryCommandBuffer()
	{
		// If we don't have an auxiliary command buffer, get one from the pool
		if (!m_auxiliaryCommandBuffer)
		{
			m_auxiliaryCommandBufferIndex = (m_auxiliaryCommandBufferIndex + 1) % m_auxiliaryCommandBufferCount;

			m_auxiliaryCommandBuffer = m_auxiliaryCommandBuffers[m_auxiliaryCommandBufferIndex];

			m_auxiliaryCommandBuffer->Begin();
		}

		return m_auxiliaryCommandBuffer;
	}

	void IDevice::ProcessDeletionQueue()
	{
		m_gpuDeletionQueue->Process();
	}

	void IDevice::ProcessQueuedCommands()
	{
		// If we have an auxiliary command buffer, it means we queued some commands
		// and we need to submit the buffer now
		if (m_auxiliaryCommandBuffer)
		{
			m_auxiliaryCommandBuffer->End();
			m_auxiliaryCommandBuffer->Submit();
			m_auxiliaryCommandBuffer = nullptr;
		}

		m_gpuTransferCallbackQueue->Process();
	}

	// We need a custom deleter for the render device because we cannot call functions that use virtual methods during the destruction
	// process. It's an unfortunate consequence of the virtual function abstraction. We need to make sure all render device GPU resources
	// are manually destroyed here, as well as giving an opportunity to the platform-specific render devices to do so
	void IDevice::FinalizeDeletion()
	{
		// Finalize any resources that are platform-specific, such as custom fences
		FinalizeDeletionPS();

		m_graphicsPipelines.clear();

		m_computePipelines.clear();

		// Process any pending transfers
		m_gpuTransferCallbackQueue->Process();
		m_gpuTransferCallbackQueue = nullptr;

		// Delete all resources that belong to the device. This puts them into the deletion queues
		for (uint32_t i = 0; i < m_auxiliaryCommandBuffers.size(); ++i)
		{
			m_auxiliaryCommandBuffers[i] = nullptr;
		}

		m_auxiliaryCommandBuffer = nullptr;

		// The last thing we do is process the deletion queue. It will take care of its own resources too
		m_gpuDeletionQueue->Finalize();
	}

	crgfx::ICommandBuffer* IDevice::CreateCommandBuffer(const crgfx::CommandBufferDescriptor& descriptor)
	{
		return CreateCommandBufferPS(descriptor);
	}

	IGPUFence* IDevice::CreateGPUFence(bool signaled)
	{
		return CreateGPUFencePS(signaled);
	}

	IGPUSemaphore* IDevice::CreateGPUSemaphore()
	{
		return CreateGPUSemaphorePS();
	}

	void IDevice::AddToDeletionQueue(GPUDeletable* resource)
	{
		m_gpuDeletionQueue->AddToQueue(resource);
	}

	IGraphicsShader* IDevice::CreateGraphicsShader(const GraphicsShaderDescriptor& graphicsShaderDescriptor)
	{
		return CreateGraphicsShaderPS(graphicsShaderDescriptor);
	}

	IComputeShader* IDevice::CreateComputeShader(const ComputeShaderDescriptor& computeShaderDescriptor)
	{
		return CreateComputeShaderPS(computeShaderDescriptor);
	}

	GraphicsPipelineHandle IDevice::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& pipelineDescriptor, const crgfx::GraphicsShaderHandle& graphicsShader, const VertexDescriptor& vertexDescriptor)
	{
		CrAssertMsg(graphicsShader != nullptr, "Invalid graphics shader passed to pipeline creation");
		CrAssertMsg(pipelineDescriptor.rasterizerState.conservativeRasterization ? SupportsConservativeRasterization() : true, "Must support conservative rasterization");

		const CrHash pipelineHash = pipelineDescriptor.ComputeHash();
		const CrHash graphicsShaderHash = graphicsShader->GetHash();
		const CrHash vertexDescriptorHash = vertexDescriptor.ComputeHash();

		const CrHash combinedHash = pipelineHash + graphicsShaderHash + vertexDescriptorHash;

		const auto& pipelineIter = m_graphicsPipelines.find(combinedHash.GetHash());

		GraphicsPipelineHandle graphicsPipeline;

		if (pipelineIter != m_graphicsPipelines.end())
		{
			graphicsPipeline = pipelineIter->second;
		}
		else
		{
			crstl::timer pipelineCreationTime;

			graphicsPipeline = GraphicsPipelineHandle(CreateGraphicsPipelinePS(pipelineDescriptor, graphicsShader, vertexDescriptor));

#if defined(RENDER_DEVICE_LOGS)

			// Print out a message that includes meaningful information
			const crstl::vector<ShaderBytecodeHandle>& bytecodes = graphicsShader->GetBytecodes();

			// Add entry point names
			crstl::fixed_string128 entryPoints("(");
			entryPoints.append(bytecodes[0]->GetEntryPoint().c_str());

			if (bytecodes.size() > 1)
			{
				for (uint32_t i = 1; i < bytecodes.size(); ++i)
				{
					entryPoints.append(", ");
					entryPoints.append(bytecodes[i]->GetEntryPoint().c_str());
				}
			}

			entryPoints.append(")");

			CrLog("Graphics Pipeline %s created (%f ms)", entryPoints.c_str(), (float)pipelineCreationTime.elapsed().milliseconds());

#endif

			m_graphicsPipelines.insert(combinedHash.GetHash(), graphicsPipeline); // Insert in the hashmap
		}

		return graphicsPipeline;
	}

	ComputePipelineHandle IDevice::CreateComputePipeline(const ComputeShaderHandle& computeShader)
	{
		CrAssertMsg(computeShader != nullptr, "Invalid compute shader passed to pipeline creation");

		const CrHash computeShaderHash = computeShader->GetHash();

		const auto& pipelineIter = m_computePipelines.find(computeShaderHash.GetHash());
		ComputePipelineHandle computePipeline;

		if (pipelineIter != m_computePipelines.end())
		{
			computePipeline = pipelineIter->second;
		}
		else
		{
			crstl::timer pipelineCreationTime;

			computePipeline = ComputePipelineHandle(CreateComputePipelinePS(computeShader));

#if defined(RENDER_DEVICE_LOGS)

			crstl::fixed_string128 entryPoint("(");
			entryPoint.append(computeShader->GetBytecode()->GetEntryPoint().c_str());
			entryPoint.append(")");

			CrLog("Compute Pipeline %s created (%f ms)", entryPoint.c_str(), (float)pipelineCreationTime.elapsed().milliseconds());

#endif

			m_computePipelines.insert(computeShaderHash.GetHash(), computePipeline); // Insert in the hashmap
		}

		return computePipeline;
	}

	IGPUQueryPool* IDevice::CreateGPUQueryPool(const GPUQueryPoolDescriptor& queryPoolDescriptor)
	{
		return CreateGPUQueryPoolPS(queryPoolDescriptor);
	}

	IHardwareGPUBuffer* IDevice::CreateHardwareGPUBuffer(const HardwareGPUBufferDescriptor& descriptor)
	{
		return CreateHardwareGPUBufferPS(descriptor);
	}

	IndexBuffer* IDevice::CreateIndexBuffer(crgfx::MemoryAccess::T access, crgfx::DataFormat::T dataFormat, uint32_t numIndices)
	{
		return new IndexBuffer(this, access, dataFormat, numIndices);
	}

	ISampler* IDevice::CreateSampler(const SamplerDescriptor& descriptor)
	{
		return CreateSamplerPS(descriptor);
	}

	ISwapchain* IDevice::CreateSwapchain(const SwapchainDescriptor& swapchainDescriptor)
	{
		CrAssertMsg(swapchainDescriptor.window, "Window cannot be null");
		CrAssertMsg(swapchainDescriptor.format != crgfx::DataFormat::Invalid, "Must set a data format");

		ISwapchain* swapchain = CreateSwapchainPS(swapchainDescriptor);

		CrAssertMsg(swapchain->GetWidth() > 0, "Swapchain must have a width");
		CrAssertMsg(swapchain->GetHeight() > 0, "Swapchain must have a height");
		CrAssertMsg(swapchain->GetImageCount() > 0, "Swapchain must have at least one image");
		CrAssertMsg(swapchain->GetFormat() != crgfx::DataFormat::Invalid, "Swapchain must have a texture format");

		return swapchain;
	}

	ITexture* IDevice::CreateTexture(const TextureDescriptor& descriptor)
	{
		return CreateTexturePS(descriptor);
	}

	VertexBuffer* IDevice::CreateVertexBuffer(crgfx::MemoryAccess::T access, const VertexDescriptor& vertexDescriptor, uint32_t numVertices)
	{
		return new VertexBuffer(this, access, vertexDescriptor, numVertices);
	}

	TypedBuffer* IDevice::CreateTypedBuffer(crgfx::MemoryAccess::T access, crgfx::DataFormat::T dataFormat, uint32_t numElements)
	{
		return new TypedBuffer(this, access, dataFormat, numElements);
	}

	crgfx::GPUFenceResult IDevice::WaitForFence(IGPUFence* fence, uint64_t timeoutNanoseconds)
	{
		return WaitForFencePS(fence, timeoutNanoseconds);
	}

	crgfx::GPUFenceResult IDevice::GetFenceStatus(IGPUFence* fence) const
	{
		return GetFenceStatusPS(fence);
	}

	void IDevice::SignalFence(crgfx::CommandQueueType::T queueType, const IGPUFence* signalFence)
	{
		return SignalFencePS(queueType, signalFence);
	}

	void IDevice::ResetFence(IGPUFence* fence)
	{
		ResetFencePS(fence);
	}

	void IDevice::WaitIdle()
	{
		WaitIdlePS();
	}

	uint8_t* IDevice::BeginTextureUpload(const ITexture* texture)
	{
		return BeginTextureUploadPS(texture);
	}

	void IDevice::EndTextureUpload(const ITexture* texture)
	{
		return EndTextureUploadPS(texture);
	}

	uint8_t* IDevice::BeginBufferUpload(const IHardwareGPUBuffer* destinationBuffer)
	{
		CrAssertMsg(destinationBuffer->GetUsage() & crgfx::BufferUsage::TransferDst, "Buffer must have transfer destination usage enabled");

		return BeginBufferUploadPS(destinationBuffer);
	}

	void IDevice::EndBufferUpload(const IHardwareGPUBuffer* destinationBuffer)
	{
		EndBufferUploadPS(destinationBuffer);
	}

	void IDevice::DownloadBuffer(const IHardwareGPUBuffer* sourceBuffer, const GPUTransferCallback& callback)
	{
		CrAssertMsg(sourceBuffer->GetUsage() & crgfx::BufferUsage::TransferSrc, "Buffer must have transfer source usage enabled");

		// Queue the download operation and return the buffer that contains the data for the CPU
		HardwareGPUBufferHandle buffer = DownloadBufferPS(sourceBuffer);

		// Create a callback that will be called by the render device during processing once it's done
		// We store the callback and the buffer as we will most likely want to map it
		GPUDownloadCallback downloadCallback;
		downloadCallback.callback = callback;
		downloadCallback.buffer = buffer;
		m_gpuTransferCallbackQueue->AddToQueue(downloadCallback);
	}

	const DeviceProperties& IDevice::GetProperties() const
	{
		return m_deviceProperties;
	}

	void IDevice::SubmitCommandBuffer(const ICommandBuffer* commandBuffer, const IGPUSemaphore* waitSemaphore, const IGPUSemaphore* signalSemaphore, const IGPUFence* signalFence)
	{
		SubmitCommandBufferPS(commandBuffer, waitSemaphore, signalSemaphore, signalFence);
	}

	void IDevice::StorePipelineCache(void* pipelineCacheData, size_t pipelineCacheSize)
	{
		if (m_isValidPipelineCache)
		{
			crstl::string pipelineCachePath = m_pipelineCacheDirectory + m_pipelineCacheFilename;

			if (crstl::create_directories(m_pipelineCacheDirectory.c_str()))
			{
				crstl::file pipelineCacheFile = crstl::file(pipelineCachePath.c_str(), crstl::file_flags::write | crstl::file_flags::create);
				pipelineCacheFile.write(pipelineCacheData, pipelineCacheSize);
			}
			else
			{
				CrLog("Could not create folder %s for shader cache", m_pipelineCacheDirectory.c_str());
			}
		}
	}

	void IDevice::LoadPipelineCache(crstl::vector<char>& pipelineCacheData)
	{
		if (m_isValidPipelineCache)
		{
			CrFixedPath pipelineCachePath = m_pipelineCacheDirectory + m_pipelineCacheFilename;

			if (crstl::file file = crstl::file(pipelineCachePath.c_str(), crstl::file_flags::read))
			{
				pipelineCacheData.resize(file.get_size());
				file.read(pipelineCacheData.data(), pipelineCacheData.size());
				CrLog("Successfully loaded serialized pipeline cache from %s", pipelineCachePath.c_str());
			}
		}
	}

	void GPUTransferCallbackQueue::Initialize(IDevice* renderDevice)
	{
		m_renderDevice = renderDevice;

		// We need as many as the system requests plus an extra one for when they're
		// all in flight (and the system reserves "the next one"). We could lazily
		// allocate in the AddToQueue function but this is simpler to reason about
		m_callbackLists.resize(MaximumCallbackLists);

		for (uint32_t i = 0; i < m_callbackLists.size(); ++i)
		{
			m_callbackLists[i].fence = m_renderDevice->CreateGPUFence();

			m_availableCallbackLists.push_back(&m_callbackLists[i]);
		}

		// Pop from the vector to get an available list
		m_currentCallbackList = m_availableCallbackLists.back();
		m_availableCallbackLists.pop_back();
	}

	void GPUTransferCallbackQueue::AddToQueue(const GPUDownloadCallback& callback)
	{
		// TODO Add synchronization for multithreading
		m_currentCallbackList->callbacks.push_back(callback);
	}

	void GPUTransferCallbackQueue::Process()
	{
		while (!m_activeCallbackLists.empty())
		{
			// Get the last list we pushed into the active list
			DownloadCallbackList* callbackList = m_activeCallbackLists.front();

			if (m_renderDevice->GetFenceStatus(callbackList->fence.get()) == crgfx::GPUFenceResult::Success)
			{
				for (const GPUDownloadCallback& callback : callbackList->callbacks)
				{
					callback.callback(callback.buffer);
				}

				// Reset the deletion list's properties
				callbackList->callbacks.clear();
				m_renderDevice->ResetFence(callbackList->fence.get());

				// Put back in the available list and remove from active
				m_availableCallbackLists.push_back(callbackList);
				m_activeCallbackLists.pop_front();
			}
			else
			{
				// Our lists are sequential, i.e. if this one has not been signaled,
				// it is guaranteed the others won't have been
				break;
			}
		}

		// 2. If there are any elements to delete, add the current list to the active
		// list and submit a fence signal to the queue
		if (!m_currentCallbackList->callbacks.empty())
		{
			m_activeCallbackLists.push_back(m_currentCallbackList);
			m_renderDevice->SignalFence(crgfx::CommandQueueType::Graphics, m_currentCallbackList->fence.get());
			m_currentCallbackList = nullptr;
		}

		if (!m_currentCallbackList && !m_availableCallbackLists.empty())
		{
			m_currentCallbackList = m_availableCallbackLists.back();
			m_availableCallbackLists.pop_back();
		}

		CrAssertMsg(m_currentCallbackList, "Current callback list cannot be null");
	}

	static const bool DebugDeletionQueues = false;

	GPUDeletionQueue::~GPUDeletionQueue()
	{

	}

	void GPUDeletionQueue::Initialize(crgfx::IDevice* renderDevice)
	{
		m_renderDevice = renderDevice;

		// We need as many as the system requests plus an extra one for when they're
		// all in flight (and the system reserves "the next one"). We could lazily
		// allocate in the AddToQueue function but this is simpler to reason about
		m_deletionLists.resize(MaximumDeletionLists);

		for (uint32_t i = 0; i < m_deletionLists.size(); ++i)
		{
			m_deletionLists[i].fence = m_renderDevice->CreateGPUFence();

			m_availableDeletionLists.push_back(&m_deletionLists[i]);
		}

		// Pop from the vector to get an available list
		m_currentDeletionList = m_availableDeletionLists.back();
		m_availableDeletionLists.pop_back();
	}

	void GPUDeletionQueue::AddToQueue(GPUDeletable* deletable)
	{
		// TODO Add synchronization for multithreading
		m_currentDeletionList->deletables.push_back(deletable);
	}

	void GPUDeletionQueue::Process()
	{
		// 1. Loop through the active lists and delete any objects that are guaranteed
		// to have been processed by the GPU

		if (DebugDeletionQueues)
		{
			// TODO Remove this. Should we track the frame index internally, passed into the deletion queue?
			// Or update the device
			CrLog("CrGPUDeletionQueue Current Frame %i", CrFrameTime::GetFrameIndex());
			CrLog("%i active deletion lists", m_activeDeletionLists.size());
		}

		while (!m_activeDeletionLists.empty())
		{
			// Get the last list we pushed into the active list
			CrDeletionList* deletionList = m_activeDeletionLists.front();

			crgfx::GPUFenceResult fenceResult = m_renderDevice->GetFenceStatus(deletionList->fence.get());

			if (fenceResult == crgfx::GPUFenceResult::Success)
			{
				for (GPUDeletable* deletable : deletionList->deletables)
				{
					delete deletable;
				}

				// Reset the deletion list's properties
				deletionList->deletables.clear();
				m_renderDevice->ResetFence(deletionList->fence.get());

				// Put back in the available list and remove from active
				m_availableDeletionLists.push_back(deletionList);
				m_activeDeletionLists.pop_front();

				if (DebugDeletionQueues)
				{
					CrLog("Deletion list emptied");
				}
			}
			else
			{
				// Our lists are sequential, i.e. if this one has not been signaled,
				// it is guaranteed the others won't have been
				break;
			}
		}

		// 2. If there are any elements to delete, add the current list to the active
		// list and submit a fence signal to the queue
		if (!m_currentDeletionList->deletables.empty())
		{
			if (DebugDeletionQueues)
			{
				CrLog("Added current deletion list to active lists");
			}

			m_activeDeletionLists.push_back(m_currentDeletionList);
			m_renderDevice->SignalFence(crgfx::CommandQueueType::Graphics, m_currentDeletionList->fence.get());
			m_currentDeletionList = nullptr;
		}

		// 3. Find an available list and assign it to the current deletion list
		if (!m_currentDeletionList && !m_availableDeletionLists.empty())
		{
			if (DebugDeletionQueues)
			{
				CrLog("Deletion list reserved from available list");
			}

			m_currentDeletionList = m_availableDeletionLists.back();
			m_availableDeletionLists.pop_back();
		}

		CrAssertMsg(m_currentDeletionList, "Current deletion list cannot be null");
	}

	void GPUDeletionQueue::Finalize()
	{
		// Delete all the fence that aren't currently in an available deletion list. It won't matter
		// who waits for them. We cannot do this with the deletion lists that are currently active
		// as we need to wait for the fences to finish
		for (CrDeletionList* deletionList : m_availableDeletionLists)
		{
			deletionList->fence = nullptr;
		}

		// Push current list to the main queue and signal it. These are the last remaining resources in flight
		m_activeDeletionLists.push_back(m_currentDeletionList);
		m_renderDevice->SignalFence(crgfx::CommandQueueType::Graphics, m_currentDeletionList->fence.get());

		// Clear the rest of the resources that have been added to the lists, and wait for them
		// instead of just checking whether the fence has been signaled at this point. There is
		// no real risk of locking up here because we have certainty that all fences were queued
		for (CrDeletionList* deletionList : m_activeDeletionLists)
		{
			if (m_renderDevice->WaitForFence(deletionList->fence.get(), UINT64_MAX) == crgfx::GPUFenceResult::Success)
			{
				// Add current fence to the deletion list. We can now guarantee this it the last usage of this list
				deletionList->fence = nullptr;

				// We need to get the last element, pop it out, and then delete it, because resources can be holding on
				// to resources that get added to the list while we start deleting, for example a command buffer that
				// holds reference to an auxiliary buffer. We assume in this model that the lifetimes for these are tied
				// such that one of them won't be in flight at the same time that the parent has been synchronized
				while (!deletionList->deletables.empty())
				{
					GPUDeletable* deletable = deletionList->deletables.back();
					deletionList->deletables.pop_back();
					delete deletable;
				}

				deletionList->deletables.clear();
			}
		}

		m_currentDeletionList = nullptr;
		m_activeDeletionLists.clear();
	}
};