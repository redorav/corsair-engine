#include "CrRendering_pch.h"

#include "ICrCommandQueue.h"
#include "ICrCommandBuffer.h"
#include "ICrRenderDevice.h"
#include "ICrPipeline.h"

#include "ICrShader.h"
#include "CrGPUStackAllocator.h"
#include "CrCPUStackAllocator.h"

#include "Core/CrMacros.h"

ICrCommandBuffer::ICrCommandBuffer(ICrCommandQueue* commandQueue)
{
	// Set the owner. The command buffer needs to know its owner to be able to assert deletion later (Vulkan and DX12 have the concept
	// of a command buffer pool and it needs to point to this pool)
	m_ownerCommandQueue = commandQueue;

	m_renderDevice = commandQueue->GetRenderDevice();

	m_constantBufferGPUStack = CrUniquePtr<CrGPUStackAllocator>(new CrGPUStackAllocator(m_renderDevice));

	m_constantBufferGPUStack->Initialize(8 * 1024 * 1024); // 8 MB

	m_completionSemaphore = m_renderDevice->CreateGPUSemaphore();

	m_completionFence = m_renderDevice->CreateGPUFence();

	m_submitted = false;
}

ICrCommandBuffer::~ICrCommandBuffer()
{

}

void ICrCommandBuffer::Begin()
{
	// Reset current state. When beginning a new command buffer 
	// any bound state is also reset, and our tracking must match
	m_currentState = CurrentState();
	m_constantBufferGPUStack->Begin();

	// If we previously submitted this command buffer, we need to wait
	// for the fence to become signaled before we can start recording
	if (m_submitted)
	{
		m_renderDevice->WaitForFence(m_completionFence.get(), UINT64_MAX);
		m_renderDevice->ResetFence(m_completionFence.get());
		m_submitted = false;
	}

	BeginPS();
}

void ICrCommandBuffer::End()
{
	m_constantBufferGPUStack->End();
	EndPS();
}

void ICrCommandBuffer::Submit(const ICrGPUSemaphore* waitSemaphore)
{
	// We need this flag to be able to reset fences properly during the begin
	m_submitted = true;

	// Submission will signal the internal semaphore of this command buffer
	m_ownerCommandQueue->SubmitCommandBuffer(this, waitSemaphore, m_completionSemaphore.get(), m_completionFence.get());
}

CrGPUBufferDescriptor ICrCommandBuffer::AllocateConstantBufferParameters(uint32_t size)
{
	GPUStackAllocation<void> allocation = m_constantBufferGPUStack->Allocate(size);

	CrGPUBufferDescriptor params(cr3d::BufferUsage::Constant, cr3d::BufferAccess::CPUWrite);
	params.existingHardwareGPUBuffer = m_constantBufferGPUStack->GetHardwareGPUBuffer();
	params.memory = allocation.memory;
	params.offset = allocation.offset;
	return params;
}

CrGPUBuffer ICrCommandBuffer::AllocateConstantBuffer(uint32_t size)
{
	return CrGPUBuffer(m_renderDevice, AllocateConstantBufferParameters(size), size);
}

void ICrCommandBuffer::BindConstantBuffer(const CrGPUBuffer* constantBuffer)
{
	BindConstantBuffer(constantBuffer, constantBuffer->GetGlobalIndex());
}

void ICrCommandBuffer::BindConstantBuffer(const CrGPUBuffer* constantBuffer, int32_t globalIndex)
{
	CrAssertMsg(constantBuffer->HasUsage(cr3d::BufferUsage::Constant), "Buffer must be set to Constant");
	CrAssertMsg(globalIndex != -1, "Global index not set");

	const CrGraphicsShaderHandle& currentShader = m_currentState.m_graphicsPipeline->m_shader;
	
	for (const CrShaderStageInfo& stageInfo : currentShader->GetStages())
	{
		m_currentState.m_constantBuffers[stageInfo.stage][globalIndex] = ConstantBufferBinding(constantBuffer->GetHardwareBuffer(), constantBuffer->GetByteOffset());
	}
}