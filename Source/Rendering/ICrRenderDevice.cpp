#include "CrRendering_pch.h"

#include "ICrRenderSystem.h"
#include "ICrRenderDevice.h"
#include "ICrCommandQueue.h"
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

ICrRenderDevice::ICrRenderDevice(const ICrRenderSystem* renderSystem)
	: m_renderSystem(renderSystem)
	, m_isValidPipelineCache(false)
{
	m_pipelineCacheDirectory = CrGlobalPaths::GetTempEngineDirectory() + "PipelineCache/";
	m_pipelineCacheDirectory += cr3d::GraphicsApi::ToString(renderSystem->GetGraphicsApi());
	m_pipelineCacheDirectory += "/";
	m_pipelineCacheFilename = "PipelineCache.bin";
	m_renderDeviceProperties.graphicsApi = renderSystem->GetGraphicsApi();
}

ICrRenderDevice::~ICrRenderDevice()
{

}

void ICrRenderDevice::InitializeDeletionQueue()
{
	m_gpuDeletionQueue.Initialize(this);
}

void ICrRenderDevice::ProcessDeletionQueue()
{
	m_gpuDeletionQueue.Process();
}

void ICrRenderDevice::FinalizeDeletionQueue()
{
	m_gpuDeletionQueue.Finalize();
}

CrCommandQueueSharedHandle ICrRenderDevice::CreateCommandQueue(CrCommandQueueType::T type)
{
	return CrCommandQueueSharedHandle(CreateCommandQueuePS(type));
}

CrGPUFenceSharedHandle ICrRenderDevice::CreateGPUFence()
{
	return CrGPUFenceSharedHandle(CreateGPUFencePS());
}

CrGPUSemaphoreSharedHandle ICrRenderDevice::CreateGPUSemaphore()
{
	return CrGPUSemaphoreSharedHandle(CreateGPUSemaphorePS());
}

CrGraphicsShaderHandle ICrRenderDevice::CreateGraphicsShader(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor) const
{
	return CrGraphicsShaderHandle(CreateGraphicsShaderPS(graphicsShaderDescriptor));
}

CrComputeShaderHandle ICrRenderDevice::CreateComputeShader(const CrComputeShaderDescriptor& computeShaderDescriptor) const
{
	return CrComputeShaderHandle(CreateComputeShaderPS(computeShaderDescriptor));
}

CrGraphicsPipelineHandle ICrRenderDevice::CreateGraphicsPipeline(const CrGraphicsPipelineDescriptor& pipelineDescriptor, const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor)
{
	CrTimer pipelineCreationTime;

	CrGraphicsPipelineHandle pipeline = CrGraphicsPipelineHandle(CreateGraphicsPipelinePS(pipelineDescriptor, graphicsShader.get(), vertexDescriptor));

	pipeline->m_shader = graphicsShader;
	pipeline->m_usedVertexStreamCount = vertexDescriptor.GetStreamCount();

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

	CrComputePipelineHandle computePipeline = CrComputePipelineHandle(CreateComputePipelinePS(pipelineDescriptor, computeShader.get()));
	computePipeline->m_shader = computeShader;

	CrFixedString128 entryPoint("(");
	entryPoint.append(computeShader->GetBytecode()->GetEntryPoint().c_str());
	entryPoint.append(")");

	CrLog("Compute Pipeline %s created (%f ms)", entryPoint.c_str(), (float)pipelineCreationTime.GetCurrent().AsMilliseconds());

	return computePipeline;
}

CrGPUQueryPoolHandle ICrRenderDevice::CreateGPUQueryPool(const CrGPUQueryPoolDescriptor& queryPoolDescriptor)
{
	return CrGPUQueryPoolHandle(CreateGPUQueryPoolPS(queryPoolDescriptor));
}

ICrHardwareGPUBuffer* ICrRenderDevice::CreateHardwareGPUBuffer(const CrHardwareGPUBufferDescriptor& descriptor)
{
	return CreateHardwareGPUBufferPS(descriptor);
}

CrIndexBufferSharedHandle ICrRenderDevice::CreateIndexBuffer(cr3d::MemoryAccess::T access, cr3d::DataFormat::T dataFormat, uint32_t numIndices)
{
	return CrIndexBufferSharedHandle(new CrIndexBufferCommon(this, access, dataFormat, numIndices), m_gpuDeletionCallback);
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
	return CrVertexBufferSharedHandle(new CrVertexBufferCommon(this, access, vertexDescriptor, numVertices), m_gpuDeletionCallback);
}

CrDataBufferSharedHandle ICrRenderDevice::CreateDataBuffer(cr3d::MemoryAccess::T access, cr3d::DataFormat::T dataFormat, uint32_t numElements)
{
	return CrDataBufferSharedHandle(new CrDataBuffer(this, access, dataFormat, numElements));
}

cr3d::GPUFenceResult ICrRenderDevice::WaitForFence(ICrGPUFence* fence, uint64_t timeoutNanoseconds) const
{
	return WaitForFencePS(fence, timeoutNanoseconds);
}

cr3d::GPUFenceResult ICrRenderDevice::GetFenceStatus(ICrGPUFence* fence) const
{
	return GetFenceStatusPS(fence);
}

void ICrRenderDevice::ResetFence(ICrGPUFence* fence)
{
	ResetFencePS(fence);
}

void ICrRenderDevice::WaitIdle()
{
	WaitIdlePS();
}

const CrRenderDeviceProperties& ICrRenderDevice::GetProperties() const
{
	return m_renderDeviceProperties;
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
