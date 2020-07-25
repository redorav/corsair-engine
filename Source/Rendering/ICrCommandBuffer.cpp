#include "CrRendering_pch.h"

#include "ICrCommandQueue.h"
#include "ICrCommandBuffer.h"
#include "ICrRenderDevice.h"

#include "CrShaderGen.h"
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

	m_constantBufferGPUStack->Init();
}

ICrCommandBuffer::~ICrCommandBuffer()
{

}

void ICrCommandBuffer::Begin()
{
	m_constantBufferGPUStack->Begin();
	BeginPS();
}

void ICrCommandBuffer::End()
{
	m_constantBufferGPUStack->End();
	EndPS();
}

void ICrCommandBuffer::Submit(const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence)
{
	m_ownerCommandQueue->SubmitCommandBuffer(this, waitSemaphore, signalSemaphore, signalFence);
}

CrGPUBufferDescriptor ICrCommandBuffer::AllocateConstantBufferParameters(uint32_t size)
{
	GPUStackAllocation<void> allocation = m_constantBufferGPUStack->Allocate(size);

	CrGPUBufferDescriptor params(cr3d::BufferUsage::Constant, cr3d::BufferAccess::CPUWrite, size);
	params.existingHardwareGPUBuffer = m_constantBufferGPUStack->GetHardwareGPUBuffer();
	params.memory = allocation.memory;
	params.offset = allocation.offset;
	return params;
}

CrGPUBuffer ICrCommandBuffer::AllocateConstantBuffer(uint32_t size)
{
	return CrGPUBuffer(m_renderDevice, AllocateConstantBufferParameters(size));
}

void ICrCommandBuffer::BindConstantBuffer(const CrGPUBuffer* constantBuffer)
{
	BindConstantBuffer(constantBuffer, constantBuffer->GetGlobalIndex());
}

void ICrCommandBuffer::BindConstantBuffer(const CrGPUBuffer* constantBuffer, int32_t globalIndex)
{
	CrAssertMsg(constantBuffer->HasUsage(cr3d::BufferUsage::Constant), "Buffer must be set to Constant");
	CrAssertMsg(globalIndex != -1, "Global index not set");
	// TODO subclass CrGPUBuffer* to have constant buffers to

	const CrGraphicsShaderHandle& currentShader = m_currentState.m_graphicsPipeline->m_shader;
	
	for (const CrShaderStageInfo& stageDesc : currentShader->m_shaderStages)
	{
		m_currentState.m_constantBuffers[stageDesc.m_stage][globalIndex].buffer = constantBuffer->GetHardwareBuffer();
		m_currentState.m_constantBuffers[stageDesc.m_stage][globalIndex].byteOffset = constantBuffer->GetByteOffset();
	}
}