#include "Graphics/CrRendering_pch.h"

#include "IGraphicsSystem.h"
#include "IDevice.h"
#include "ICrCommandBuffer.h"
#include "ICrSwapchain.h"
#include "ICrSampler.h"
#include "ICrTexture.h"
#include "ICrShader.h"
#include "ICrPipeline.h"
#include "ICrGPUQueryPool.h"
#include "CrGPUBuffer.h"
#include "CrGPUStackAllocator.h"

#include "Core/CrMacros.h"
#include "Core/Logging/ICrDebug.h"
#include "Core/CrGlobalPaths.h"

#include "Graphics/CrGPUDeletionQueue.h"
#include "Graphics/CrGPUTransferCallbackQueue.h"

#include "crstl/timer.h"

#if !defined(CR_CONFIG_FINAL)
#define RENDER_DEVICE_LOGS
#endif

namespace crgfx
{
	IDevice::IDevice(IGraphicsSystem* renderSystem, const crgfx::DeviceDescriptor& descriptor)
		: m_isValidPipelineCache(false)
	{
		m_pipelineCacheDirectory = CrGlobalPaths::GetTempEngineDirectory() + "Pipeline Cache/";
		m_pipelineCacheDirectory += crgfx::GraphicsApi::ToString(renderSystem->GetGraphicsApi());
		m_pipelineCacheDirectory += "/";
		m_pipelineCacheFilename = "PipelineCache.bin";
		m_deviceProperties.graphicsApi = renderSystem->GetGraphicsApi();
		m_deviceProperties.preferredVendor = descriptor.preferredVendor;

		m_gpuDeletionQueue = crstl::unique_ptr<CrGPUDeletionQueue>(new CrGPUDeletionQueue());

		m_gpuTransferCallbackQueue = crstl::unique_ptr<CrGPUTransferCallbackQueue>(new CrGPUTransferCallbackQueue());
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
			CrCommandBufferDescriptor descriptor;
			descriptor.name.append_sprintf("Render Device Auxiliary Command Buffer %i", i);
			m_auxiliaryCommandBuffers.push_back(CreateCommandBuffer(descriptor));
		}
	}

	const CrCommandBufferHandle& IDevice::GetAuxiliaryCommandBuffer()
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

	ICrCommandBuffer* IDevice::CreateCommandBuffer(const CrCommandBufferDescriptor& descriptor)
	{
		return CreateCommandBufferPS(descriptor);
	}

	ICrGPUFence* IDevice::CreateGPUFence(bool signaled)
	{
		return CreateGPUFencePS(signaled);
	}

	ICrGPUSemaphore* IDevice::CreateGPUSemaphore()
	{
		return CreateGPUSemaphorePS();
	}

	void IDevice::AddToDeletionQueue(CrGPUDeletable* resource)
	{
		m_gpuDeletionQueue->AddToQueue(resource);
	}

	ICrGraphicsShader* IDevice::CreateGraphicsShader(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor)
	{
		return CreateGraphicsShaderPS(graphicsShaderDescriptor);
	}

	ICrComputeShader* IDevice::CreateComputeShader(const CrComputeShaderDescriptor& computeShaderDescriptor)
	{
		return CreateComputeShaderPS(computeShaderDescriptor);
	}

	CrGraphicsPipelineHandle IDevice::CreateGraphicsPipeline(const CrGraphicsPipelineDescriptor& pipelineDescriptor, const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor)
	{
		CrAssertMsg(graphicsShader != nullptr, "Invalid graphics shader passed to pipeline creation");
		CrAssertMsg(pipelineDescriptor.rasterizerState.conservativeRasterization ? SupportsConservativeRasterization() : true, "Must support conservative rasterization");

		const CrHash pipelineHash = pipelineDescriptor.ComputeHash();
		const CrHash graphicsShaderHash = graphicsShader->GetHash();
		const CrHash vertexDescriptorHash = vertexDescriptor.ComputeHash();

		const CrHash combinedHash = pipelineHash + graphicsShaderHash + vertexDescriptorHash;

		const auto& pipelineIter = m_graphicsPipelines.find(combinedHash.GetHash());

		CrGraphicsPipelineHandle graphicsPipeline;

		if (pipelineIter != m_graphicsPipelines.end())
		{
			graphicsPipeline = pipelineIter->second;
		}
		else
		{
			crstl::timer pipelineCreationTime;

			graphicsPipeline = CrGraphicsPipelineHandle(CreateGraphicsPipelinePS(pipelineDescriptor, graphicsShader, vertexDescriptor));

#if defined(RENDER_DEVICE_LOGS)

			// Print out a message that includes meaningful information
			const crstl::vector<CrShaderBytecodeHandle>& bytecodes = graphicsShader->GetBytecodes();

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

	CrComputePipelineHandle IDevice::CreateComputePipeline(const CrComputeShaderHandle& computeShader)
	{
		CrAssertMsg(computeShader != nullptr, "Invalid compute shader passed to pipeline creation");

		const CrHash computeShaderHash = computeShader->GetHash();

		const auto& pipelineIter = m_computePipelines.find(computeShaderHash.GetHash());
		CrComputePipelineHandle computePipeline;

		if (pipelineIter != m_computePipelines.end())
		{
			computePipeline = pipelineIter->second;
		}
		else
		{
			crstl::timer pipelineCreationTime;

			computePipeline = CrComputePipelineHandle(CreateComputePipelinePS(computeShader));

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

	ICrGPUQueryPool* IDevice::CreateGPUQueryPool(const CrGPUQueryPoolDescriptor& queryPoolDescriptor)
	{
		return CreateGPUQueryPoolPS(queryPoolDescriptor);
	}

	ICrHardwareGPUBuffer* IDevice::CreateHardwareGPUBuffer(const CrHardwareGPUBufferDescriptor& descriptor)
	{
		return CreateHardwareGPUBufferPS(descriptor);
	}

	CrIndexBuffer* IDevice::CreateIndexBuffer(crgfx::MemoryAccess::T access, crgfx::DataFormat::T dataFormat, uint32_t numIndices)
	{
		return new CrIndexBuffer(this, access, dataFormat, numIndices);
	}

	ICrSampler* IDevice::CreateSampler(const CrSamplerDescriptor& descriptor)
	{
		return CreateSamplerPS(descriptor);
	}

	ICrSwapchain* IDevice::CreateSwapchain(const CrSwapchainDescriptor& swapchainDescriptor)
	{
		CrAssertMsg(swapchainDescriptor.window, "Window cannot be null");
		CrAssertMsg(swapchainDescriptor.format != crgfx::DataFormat::Invalid, "Must set a data format");

		ICrSwapchain* swapchain = CreateSwapchainPS(swapchainDescriptor);

		CrAssertMsg(swapchain->GetWidth() > 0, "Swapchain must have a width");
		CrAssertMsg(swapchain->GetHeight() > 0, "Swapchain must have a height");
		CrAssertMsg(swapchain->GetImageCount() > 0, "Swapchain must have at least one image");
		CrAssertMsg(swapchain->GetFormat() != crgfx::DataFormat::Invalid, "Swapchain must have a texture format");

		return swapchain;
	}

	ICrTexture* IDevice::CreateTexture(const CrTextureDescriptor& descriptor)
	{
		return CreateTexturePS(descriptor);
	}

	CrVertexBuffer* IDevice::CreateVertexBuffer(crgfx::MemoryAccess::T access, const CrVertexDescriptor& vertexDescriptor, uint32_t numVertices)
	{
		return new CrVertexBuffer(this, access, vertexDescriptor, numVertices);
	}

	CrTypedBuffer* IDevice::CreateTypedBuffer(crgfx::MemoryAccess::T access, crgfx::DataFormat::T dataFormat, uint32_t numElements)
	{
		return new CrTypedBuffer(this, access, dataFormat, numElements);
	}

	crgfx::GPUFenceResult IDevice::WaitForFence(ICrGPUFence* fence, uint64_t timeoutNanoseconds)
	{
		return WaitForFencePS(fence, timeoutNanoseconds);
	}

	crgfx::GPUFenceResult IDevice::GetFenceStatus(ICrGPUFence* fence) const
	{
		return GetFenceStatusPS(fence);
	}

	void IDevice::SignalFence(crgfx::CommandQueueType::T queueType, const ICrGPUFence* signalFence)
	{
		return SignalFencePS(queueType, signalFence);
	}

	void IDevice::ResetFence(ICrGPUFence* fence)
	{
		ResetFencePS(fence);
	}

	void IDevice::WaitIdle()
	{
		WaitIdlePS();
	}

	uint8_t* IDevice::BeginTextureUpload(const ICrTexture* texture)
	{
		return BeginTextureUploadPS(texture);
	}

	void IDevice::EndTextureUpload(const ICrTexture* texture)
	{
		return EndTextureUploadPS(texture);
	}

	uint8_t* IDevice::BeginBufferUpload(const ICrHardwareGPUBuffer* destinationBuffer)
	{
		CrAssertMsg(destinationBuffer->GetUsage() & crgfx::BufferUsage::TransferDst, "Buffer must have transfer destination usage enabled");

		return BeginBufferUploadPS(destinationBuffer);
	}

	void IDevice::EndBufferUpload(const ICrHardwareGPUBuffer* destinationBuffer)
	{
		EndBufferUploadPS(destinationBuffer);
	}

	void IDevice::DownloadBuffer(const ICrHardwareGPUBuffer* sourceBuffer, const CrGPUTransferCallbackType& callback)
	{
		CrAssertMsg(sourceBuffer->GetUsage() & crgfx::BufferUsage::TransferSrc, "Buffer must have transfer source usage enabled");

		// Queue the download operation and return the buffer that contains the data for the CPU
		CrHardwareGPUBufferHandle buffer = DownloadBufferPS(sourceBuffer);

		// Create a callback that will be called by the render device during processing once it's done
		// We store the callback and the buffer as we will most likely want to map it
		CrGPUDownloadCallback downloadCallback;
		downloadCallback.callback = callback;
		downloadCallback.buffer = buffer;
		m_gpuTransferCallbackQueue->AddToQueue(downloadCallback);
	}

	const DeviceProperties& IDevice::GetProperties() const
	{
		return m_deviceProperties;
	}

	void IDevice::SubmitCommandBuffer(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence)
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
};