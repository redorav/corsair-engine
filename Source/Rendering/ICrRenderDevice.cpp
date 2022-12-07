#include "CrRendering_pch.h"

#include "ICrRenderSystem.h"
#include "ICrRenderDevice.h"
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
#include "Core/FileSystem/ICrFile.h"
#include "Core/Time/CrTimer.h"
#include "Core/Logging/ICrDebug.h"
#include "Core/CrGlobalPaths.h"

#include "Rendering/CrGPUDeletionQueue.h"
#include "Rendering/CrGPUTransferCallbackQueue.h"

ICrRenderDevice::ICrRenderDevice(const ICrRenderSystem* renderSystem, const CrRenderDeviceDescriptor& descriptor)
	: m_renderSystem(renderSystem)
	, m_isValidPipelineCache(false)
{
	m_pipelineCacheDirectory = CrGlobalPaths::GetTempEngineDirectory() + "Pipeline Cache/";
	m_pipelineCacheDirectory += cr3d::GraphicsApi::ToString(renderSystem->GetGraphicsApi());
	m_pipelineCacheDirectory += "/";
	m_pipelineCacheFilename = "PipelineCache.bin";
	m_renderDeviceProperties.graphicsApi = renderSystem->GetGraphicsApi();
	m_renderDeviceProperties.preferredVendor = descriptor.preferredVendor;

	m_gpuDeletionCallback = [this](CrGPUDeletable* deletable) { m_gpuDeletionQueue->AddToQueue(deletable); };
	m_gpuDeletionQueue = CrUniquePtr<CrGPUDeletionQueue>(new CrGPUDeletionQueue());

	m_gpuTransferCallbackQueue = CrUniquePtr<CrGPUTransferCallbackQueue>(new CrGPUTransferCallbackQueue());
}

ICrRenderDevice::~ICrRenderDevice()
{

}

void ICrRenderDevice::Initialize()
{
	m_gpuDeletionQueue->Initialize(this);

	m_gpuTransferCallbackQueue->Initialize(this);

	m_auxiliaryCommandBufferCount = 3; // TODO Pass in or make dynamic

	for (uint32_t i = 0; i < m_auxiliaryCommandBufferCount; ++i)
	{
		CrCommandBufferDescriptor descriptor;
		descriptor.name.append("Render Device Auxiliary Command Buffer %i", i);
		m_auxiliaryCommandBuffers.push_back(CreateCommandBuffer(descriptor));
	}
}

const CrCommandBufferSharedHandle& ICrRenderDevice::GetAuxiliaryCommandBuffer()
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

void ICrRenderDevice::ProcessDeletionQueue()
{
	m_gpuDeletionQueue->Process();
}

void ICrRenderDevice::ProcessQueuedCommands()
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

void ICrRenderDevice::FinalizeDeletion()
{
	m_auxiliaryCommandBuffer = nullptr;

	m_gpuDeletionQueue->Finalize();
}

CrCommandBufferSharedHandle ICrRenderDevice::CreateCommandBuffer(const CrCommandBufferDescriptor& descriptor)
{
	return CrCommandBufferSharedHandle(CreateCommandBufferPS(descriptor));
}

CrGPUFenceSharedHandle ICrRenderDevice::CreateGPUFence()
{
	return CrGPUFenceSharedHandle(CreateGPUFencePS());
}

CrGPUSemaphoreSharedHandle ICrRenderDevice::CreateGPUSemaphore()
{
	return CrGPUSemaphoreSharedHandle(CreateGPUSemaphorePS(), m_gpuDeletionCallback);
}

CrGraphicsShaderHandle ICrRenderDevice::CreateGraphicsShader(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor)
{
	return CrGraphicsShaderHandle(CreateGraphicsShaderPS(graphicsShaderDescriptor));
}

CrComputeShaderHandle ICrRenderDevice::CreateComputeShader(const CrComputeShaderDescriptor& computeShaderDescriptor)
{
	return CrComputeShaderHandle(CreateComputeShaderPS(computeShaderDescriptor));
}

CrGraphicsPipelineHandle ICrRenderDevice::CreateGraphicsPipeline(const CrGraphicsPipelineDescriptor& pipelineDescriptor, const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor)
{
	CrTimer pipelineCreationTime;

	CrGraphicsPipelineHandle pipeline = CrGraphicsPipelineHandle(CreateGraphicsPipelinePS(pipelineDescriptor, graphicsShader, vertexDescriptor));

	// Print out a message that includes meaningful information
	const CrVector<CrShaderBytecodeSharedHandle>& bytecodes = graphicsShader->GetBytecodes();
	
	// Add entry point names
	CrFixedString128 entryPoints("(");
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

	CrLog("Graphics Pipeline %s created (%f ms)", entryPoints.c_str(), (float)pipelineCreationTime.GetCurrent().AsMilliseconds());

	return pipeline;
}

CrComputePipelineHandle ICrRenderDevice::CreateComputePipeline(const CrComputePipelineDescriptor& pipelineDescriptor, const CrComputeShaderHandle& computeShader)
{
	CrTimer pipelineCreationTime;

	CrComputePipelineHandle computePipeline = CrComputePipelineHandle(CreateComputePipelinePS(pipelineDescriptor, computeShader));

	CrFixedString128 entryPoint("(");
	entryPoint.append(computeShader->GetBytecode()->GetEntryPoint().c_str());
	entryPoint.append(")");

	CrLog("Compute Pipeline %s created (%f ms)", entryPoint.c_str(), (float)pipelineCreationTime.GetCurrent().AsMilliseconds());

	return computePipeline;
}

CrGPUQueryPoolHandle ICrRenderDevice::CreateGPUQueryPool(const CrGPUQueryPoolDescriptor& queryPoolDescriptor)
{
	return CrGPUQueryPoolHandle(CreateGPUQueryPoolPS(queryPoolDescriptor), m_gpuDeletionCallback);
}

CrHardwareGPUBufferHandle ICrRenderDevice::CreateHardwareGPUBuffer(const CrHardwareGPUBufferDescriptor& descriptor)
{
	return CrHardwareGPUBufferHandle(CreateHardwareGPUBufferPS(descriptor), m_gpuDeletionCallback);
}

ICrHardwareGPUBuffer* ICrRenderDevice::CreateHardwareGPUBufferPointer(const CrHardwareGPUBufferDescriptor& descriptor)
{
	return CreateHardwareGPUBufferPS(descriptor);
}

CrIndexBufferSharedHandle ICrRenderDevice::CreateIndexBuffer(cr3d::MemoryAccess::T access, cr3d::DataFormat::T dataFormat, uint32_t numIndices)
{
	return CrIndexBufferSharedHandle(new CrIndexBufferCommon(this, access, dataFormat, numIndices));
}

CrSamplerSharedHandle ICrRenderDevice::CreateSampler(const CrSamplerDescriptor& descriptor)
{
	return CrSamplerSharedHandle(CreateSamplerPS(descriptor), m_gpuDeletionCallback);
}

CrSwapchainSharedHandle ICrRenderDevice::CreateSwapchain(const CrSwapchainDescriptor& swapchainDescriptor)
{
	CrAssertMsg(swapchainDescriptor.platformWindow, "Platform window cannot be null");
	CrAssertMsg(swapchainDescriptor.platformHandle, "Platform handle cannot be null");
	CrAssertMsg(swapchainDescriptor.format != cr3d::DataFormat::Invalid, "Must set a data format");

	CrSwapchainSharedHandle swapchain = CrSwapchainSharedHandle(CreateSwapchainPS(swapchainDescriptor));

	CrAssertMsg(swapchain->GetWidth() > 0, "Swapchain must have a width");
	CrAssertMsg(swapchain->GetHeight() > 0, "Swapchain must have a height");
	CrAssertMsg(swapchain->GetImageCount() > 0, "Swapchain must have at least one image");
	CrAssertMsg(swapchain->GetFormat() != cr3d::DataFormat::Invalid, "Swapchain must have a texture format");

	return swapchain;
}

CrTextureSharedHandle ICrRenderDevice::CreateTexture(const CrTextureDescriptor& descriptor)
{
	return CrTextureSharedHandle(CreateTexturePS(descriptor), m_gpuDeletionCallback);
}

CrVertexBufferSharedHandle ICrRenderDevice::CreateVertexBuffer(cr3d::MemoryAccess::T access, const CrVertexDescriptor& vertexDescriptor, uint32_t numVertices)
{
	return CrVertexBufferSharedHandle(new CrVertexBufferCommon(this, access, vertexDescriptor, numVertices));
}

CrDataBufferSharedHandle ICrRenderDevice::CreateDataBuffer(cr3d::MemoryAccess::T access, cr3d::DataFormat::T dataFormat, uint32_t numElements)
{
	return CrDataBufferSharedHandle(new CrDataBuffer(this, access, dataFormat, numElements));
}

cr3d::GPUFenceResult ICrRenderDevice::WaitForFence(ICrGPUFence* fence, uint64_t timeoutNanoseconds)
{
	return WaitForFencePS(fence, timeoutNanoseconds);
}

cr3d::GPUFenceResult ICrRenderDevice::GetFenceStatus(ICrGPUFence* fence) const
{
	return GetFenceStatusPS(fence);
}

void ICrRenderDevice::SignalFence(CrCommandQueueType::T queueType, const ICrGPUFence* signalFence)
{
	return SignalFencePS(queueType, signalFence);
}

void ICrRenderDevice::ResetFence(ICrGPUFence* fence)
{
	ResetFencePS(fence);
}

void ICrRenderDevice::WaitIdle()
{
	WaitIdlePS();
}

uint8_t* ICrRenderDevice::BeginTextureUpload(const ICrTexture* texture)
{
	return BeginTextureUploadPS(texture);
}

void ICrRenderDevice::EndTextureUpload(const ICrTexture* texture)
{
	return EndTextureUploadPS(texture);
}

uint8_t* ICrRenderDevice::BeginBufferUpload(const ICrHardwareGPUBuffer* destinationBuffer)
{
	CrAssertMsg(destinationBuffer->GetUsage() & cr3d::BufferUsage::TransferDst, "Buffer must have transfer destination usage enabled");

	return BeginBufferUploadPS(destinationBuffer);
}

void ICrRenderDevice::EndBufferUpload(const ICrHardwareGPUBuffer* destinationBuffer)
{
	EndBufferUploadPS(destinationBuffer);
}

void ICrRenderDevice::DownloadBuffer(const ICrHardwareGPUBuffer* sourceBuffer, const CrGPUTransferCallbackType& callback)
{
	CrAssertMsg(sourceBuffer->GetUsage() & cr3d::BufferUsage::TransferSrc, "Buffer must have transfer source usage enabled");

	// Queue the download operation and return the buffer that contains the data for the CPU
	CrHardwareGPUBufferHandle buffer = DownloadBufferPS(sourceBuffer);

	// Create a callback that will be called by the render device during processing once it's done
	// We store the callback and the buffer as we will most likely want to map it
	CrGPUDownloadCallback downloadCallback;
	downloadCallback.callback = callback;
	downloadCallback.buffer = buffer;
	m_gpuTransferCallbackQueue->AddToQueue(downloadCallback);
}

const CrRenderDeviceProperties& ICrRenderDevice::GetProperties() const
{
	return m_renderDeviceProperties;
}

void ICrRenderDevice::SubmitCommandBuffer(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence)
{
	SubmitCommandBufferPS(commandBuffer, waitSemaphore, signalSemaphore, signalFence);
}

void ICrRenderDevice::StorePipelineCache(void* pipelineCacheData, size_t pipelineCacheSize)
{
	if (m_isValidPipelineCache)
	{
		CrString pipelineCachePath = m_pipelineCacheDirectory + m_pipelineCacheFilename;

		// We assume the directory has been created by now
		if (ICrFile::CreateDirectories(m_pipelineCacheDirectory.c_str()))
		{
			CrFileSharedHandle file = ICrFile::OpenFile(pipelineCachePath.c_str(), FileOpenFlags::Write | FileOpenFlags::Create);
			file->Write(pipelineCacheData, pipelineCacheSize);
		}
		else
		{
			CrLog("Could not create folder %s for shader cache", m_pipelineCacheDirectory.c_str());
		}
	}
}

void ICrRenderDevice::LoadPipelineCache(CrVector<char>& pipelineCacheData)
{
	if (m_isValidPipelineCache)
	{
		CrPath pipelineCachePath = m_pipelineCacheDirectory + m_pipelineCacheFilename;
		CrFileSharedHandle file = ICrFile::OpenFile(pipelineCachePath.c_str(), FileOpenFlags::Read);

		if (file)
		{
			pipelineCacheData.resize(file->GetSize());
			file->Read(pipelineCacheData.data(), pipelineCacheData.size());
			CrLog("Successfully loaded serialized pipeline cache from %s", pipelineCachePath.c_str());
		}
	}
}
