#include "CrRendering_pch.h"

#include "ICrRenderSystem.h"
#include "ICrRenderDevice.h"
#include "ICrCommandQueue.h"
#include "ICrSwapchain.h"
#include "ICrSampler.h"
#include "ICrTexture.h"
#include "ICrShader.h"
#include "ICrPipeline.h"
#include "CrGPUBuffer.h"
#include "CrGPUStackAllocator.h"

#include "Core/CrMacros.h"
#include "Core/FileSystem/ICrFile.h"
#include "Core/Time/CrTimer.h"
#include "Core/Logging/ICrDebug.h"

#include "GlobalVariables.h"

ICrRenderDevice::ICrRenderDevice(const ICrRenderSystem* renderSystem) : m_renderSystem(renderSystem)
{
	m_pipelineCacheDirectory = CrString(GlobalPaths::ShaderSourceDirectory);
	m_pipelineCacheDirectory += cr3d::GraphicsApi::ToString(renderSystem->GetGraphicsApi());
	m_pipelineCacheDirectory += "/";
	m_pipelineCacheFilename = "PipelineCache.bin";
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

CrGraphicsPipelineHandle ICrRenderDevice::CreateGraphicsPipeline(const CrGraphicsPipelineDescriptor& pipelineDescriptor, const ICrGraphicsShader* graphicsShader, const CrVertexDescriptor& vertexDescriptor)
{
	CrTimer pipelineCreationTime;

	CrGraphicsPipelineHandle pipeline = CrGraphicsPipelineHandle(CreateGraphicsPipelinePS(pipelineDescriptor, graphicsShader, vertexDescriptor));

	CrLog("Pipeline created (%f ms)", pipelineCreationTime.GetCurrent().AsMilliseconds());

	return pipeline;
}

CrComputePipelineHandle ICrRenderDevice::CreateComputePipeline(const CrComputePipelineDescriptor& pipelineDescriptor, const ICrComputeShader* computeShader)
{
	return CrComputePipelineHandle(CreateComputePipelinePS(pipelineDescriptor, computeShader));
}

ICrHardwareGPUBuffer* ICrRenderDevice::CreateHardwareGPUBuffer(const CrHardwareGPUBufferDescriptor& descriptor)
{
	return CreateHardwareGPUBufferPS(descriptor);
}

CrIndexBufferSharedHandle ICrRenderDevice::CreateIndexBuffer(cr3d::DataFormat::T dataFormat, uint32_t numIndices)
{
	return CrIndexBufferSharedHandle(new CrIndexBuffer(this, dataFormat, numIndices), m_gpuDeletionCallback);
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
	return CrTextureSharedHandle(CreateTexturePS(descriptor));
}

CrVertexBufferSharedHandle ICrRenderDevice::CreateVertexBuffer(uint32_t numVertices, const CrVertexDescriptor& vertexDescriptor)
{
	return CrVertexBufferSharedHandle(new CrVertexBufferCommon(this, numVertices, vertexDescriptor), m_gpuDeletionCallback);
}

CrDataBufferSharedHandle ICrRenderDevice::CreateDataBuffer(cr3d::BufferAccess::T access, cr3d::DataFormat::T dataFormat, uint32_t numElements)
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
	CrString pipelineCachePath = m_pipelineCacheDirectory + m_pipelineCacheFilename;

	if (ICrFile::CreateFolder(m_pipelineCacheDirectory.c_str()))
	{
		CrFileSharedHandle file = ICrFile::OpenFile(pipelineCachePath.c_str(), FileOpenFlags::Write | FileOpenFlags::Create);
		file->Write(pipelineCacheData, pipelineCacheSize);
	}
}

void ICrRenderDevice::LoadPipelineCache(CrVector<char>& pipelineCacheData)
{
	CrString pipelineCachePath = m_pipelineCacheDirectory + m_pipelineCacheFilename;
	CrFileSharedHandle file = ICrFile::OpenFile(pipelineCachePath.c_str(), FileOpenFlags::Read);

	if (file)
	{
		pipelineCacheData.resize(file->GetSize());
		file->Read(pipelineCacheData.data(), pipelineCacheData.size());
	}
}
