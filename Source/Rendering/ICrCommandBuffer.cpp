#include "Rendering/CrRendering_pch.h"

#include "ICrCommandBuffer.h"
#include "ICrRenderDevice.h"
#include "ICrPipeline.h"

#include "ICrShader.h"
#include "CrGPUStackAllocator.h"
#include "CrCPUStackAllocator.h"

#include "Core/CrMacros.h"

ICrCommandBuffer::ICrCommandBuffer(ICrRenderDevice* renderDevice, const CrCommandBufferDescriptor& descriptor) : CrGPUAutoDeletable(renderDevice)
	, m_queueType(descriptor.queueType)
	, m_submitted(false)
	, m_recording(false)
{
	// Initialize GPU buffer stack allocators - for streaming
	// TODO it is possible to create these lazily on allocation instead
	// to avoid having every command buffer allocate these if they
	// aren't going to be used
	{
		if (descriptor.dynamicConstantBufferSizeBytes > 0)
		{
			CrHardwareGPUBufferDescriptor constantBufferStack(cr3d::BufferUsage::Constant, cr3d::MemoryAccess::CPUStreamToGPU, descriptor.dynamicConstantBufferSizeBytes);
			constantBufferStack.name = "Constant Buffer Stack";
			m_constantBufferGPUStack = crstl::unique_ptr<CrGPUStackAllocator>(new CrGPUStackAllocator(m_renderDevice, constantBufferStack));
		}

		// Allocate memory for transient vertex and index data. This is just an approximation as it depends on the size of each vertex and index
		// TODO We need a more flexible approach as this takes up a lot of memory per command buffer. It would be smart to have a pool in the device
		// and lazily allocate them or something similar
		if (descriptor.dynamicVertexBufferSizeVertices > 0)
		{
			uint32_t maxVertices = descriptor.dynamicVertexBufferSizeVertices;
			uint32_t maxIndices = maxVertices * 3;

			CrHardwareGPUBufferDescriptor vertexBufferStack(cr3d::BufferUsage::Vertex, cr3d::MemoryAccess::CPUStreamToGPU, maxVertices, 4);
			vertexBufferStack.name = "Vertex Buffer Stack";
			m_vertexBufferGPUStack = crstl::unique_ptr<CrGPUStackAllocator>(new CrGPUStackAllocator(m_renderDevice, vertexBufferStack));

			CrHardwareGPUBufferDescriptor indexBufferStack(cr3d::BufferUsage::Index, cr3d::MemoryAccess::CPUStreamToGPU, maxIndices, cr3d::DataFormats[cr3d::DataFormat::R16_Uint].dataOrBlockSize);
			indexBufferStack.name = "Index Buffer Stack";
			m_indexBufferGPUStack = crstl::unique_ptr<CrGPUStackAllocator>(new CrGPUStackAllocator(m_renderDevice, indexBufferStack));
		}
	}

	m_completionFence = m_renderDevice->CreateGPUFence();
}

ICrCommandBuffer::~ICrCommandBuffer()
{

}

void ICrCommandBuffer::Begin()
{
	CrAssertMsg(!m_recording, "Cannot begin a command buffer while it is recording");

	m_recording = true;

	// Reset current state. When beginning a new command buffer 
	// any bound state is also reset, and our tracking must match
	m_currentState = CurrentState();

	if (m_constantBufferGPUStack)
	{
		m_constantBufferGPUStack->Begin();
	}

	if (m_vertexBufferGPUStack)
	{
		m_vertexBufferGPUStack->Begin();
		m_indexBufferGPUStack->Begin();
	}

	// If we previously submitted this command buffer, we need to wait
	// for the fence to become signaled before we can start recording
	if (m_submitted)
	{
		cr3d::GPUFenceResult result = m_renderDevice->WaitForFence(m_completionFence.get(), UINT64_MAX);
		CrAssertMsg(result == cr3d::GPUFenceResult::Success, "Failed waiting for fence");
		m_renderDevice->ResetFence(m_completionFence.get());
		m_submitted = false;
	}

	BeginPS();
}

void ICrCommandBuffer::End()
{
	if (m_constantBufferGPUStack)
	{
		m_constantBufferGPUStack->End();
	}

	if (m_vertexBufferGPUStack)
	{
		m_vertexBufferGPUStack->End();
		m_indexBufferGPUStack->End();
	}

	EndPS();

	m_recording = false;
}

void ICrCommandBuffer::Submit()
{
	// We need this flag to be able to reset fences properly during the begin
	m_submitted = true;

	// Submission will signal the internal semaphore of this command buffer
	m_renderDevice->SubmitCommandBuffer(this, nullptr, nullptr, m_completionFence.get());
}

void ICrCommandBuffer::BeginTimestampQuery(const ICrGPUQueryPool* queryPool, CrGPUQueryId query)
{
	BeginTimestampQueryPS(queryPool, query);
}

void ICrCommandBuffer::EndTimestampQuery(const ICrGPUQueryPool* queryPool, CrGPUQueryId query)
{
	EndTimestampQueryPS(queryPool, query);
}

void ICrCommandBuffer::ResetGPUQueries(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count)
{
	ResetGPUQueriesPS(queryPool, start, count);
}

void ICrCommandBuffer::ResolveGPUQueries(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count)
{
	ResolveGPUQueriesPS(queryPool, start, count);
}

CrGPUBufferView ICrCommandBuffer::AllocateConstantBuffer(uint32_t sizeBytes)
{
	CrStackAllocation<void> allocation = m_constantBufferGPUStack->AllocateAligned(sizeBytes, 256);

	CrGPUBufferView constantBufferView
	(
		m_constantBufferGPUStack->GetHardwareGPUBuffer(),
		1,
		sizeBytes,
		allocation.offset,
		allocation.memory
	);

	return constantBufferView;
}

CrGPUBufferView ICrCommandBuffer::AllocateVertexBuffer(uint32_t vertexCount, uint32_t stride)
{
	uint32_t sizeBytes = vertexCount * stride;

	// TODO Fix alignment
	CrStackAllocation<void> allocation = m_vertexBufferGPUStack->AllocateAligned(sizeBytes, 256);

	CrGPUBufferView vertexBufferView
	(
		m_vertexBufferGPUStack->GetHardwareGPUBuffer(),
		vertexCount,
		stride,
		allocation.offset,
		allocation.memory
	);

	return vertexBufferView;
}

CrGPUBufferView ICrCommandBuffer::AllocateIndexBuffer(uint32_t indexCount, cr3d::DataFormat::T indexFormat)
{
	uint32_t sizeBytes = indexCount * cr3d::DataFormats[indexFormat].dataOrBlockSize;

	// TODO Fix alignment
	CrStackAllocation<void> allocation = m_indexBufferGPUStack->AllocateAligned(sizeBytes, 256);
	
	CrGPUBufferView indexBufferView
	(
		m_indexBufferGPUStack->GetHardwareGPUBuffer(),
		indexCount,
		indexFormat,
		allocation.offset,
		allocation.memory
	);

	return indexBufferView;
}

void ICrCommandBuffer::BeginRenderPass(const CrRenderPassDescriptor& renderPassDescriptor)
{
	CrCommandBufferAssertMsg(!m_currentState.m_renderPassActive, "Render pass already active. Have you forgotten to close a render pass?");

	m_currentState.m_currentRenderPass = renderPassDescriptor;
	m_currentState.m_renderPassActive = true;

	if (m_currentState.m_currentRenderPass.debugName.size() > 0)
	{
		BeginDebugEvent(m_currentState.m_currentRenderPass.debugName.c_str(), m_currentState.m_currentRenderPass.debugColor);
	}

#if defined(COMMAND_BUFFER_VALIDATION)
	for (uint32_t i = 0; i < renderPassDescriptor.color.size(); ++i)
	{
		const CrRenderTargetDescriptor& renderTargetDescriptor = renderPassDescriptor.color[i];
		if (renderTargetDescriptor.loadOp == CrRenderTargetLoadOp::Load && renderTargetDescriptor.initialState.layout == cr3d::TextureLayout::Undefined)
		{
			CrCommandBufferAssertMsg(false, "Invalid combination");
		}

		CrCommandBufferAssertMsg(renderTargetDescriptor.initialState.stages != cr3d::ShaderStageFlags::None, "");
		CrCommandBufferAssertMsg(renderTargetDescriptor.usageState.stages != cr3d::ShaderStageFlags::None, "");
		CrCommandBufferAssertMsg(renderTargetDescriptor.finalState.stages != cr3d::ShaderStageFlags::None, "");
	}

	if (renderPassDescriptor.depth.texture)
	{
		CrCommandBufferAssertMsg(renderPassDescriptor.depth.initialState.stages != cr3d::ShaderStageFlags::None, "");
		CrCommandBufferAssertMsg(renderPassDescriptor.depth.usageState.stages != cr3d::ShaderStageFlags::None, "");
		CrCommandBufferAssertMsg(renderPassDescriptor.depth.finalState.stages != cr3d::ShaderStageFlags::None, "");
	}
#endif

	BeginRenderPassPS(renderPassDescriptor);
}

void ICrCommandBuffer::EndRenderPass()
{
	CrCommandBufferAssertMsg(m_currentState.m_renderPassActive, "Render pass not active. Have you forgotten to start a render pass?");
	
	EndRenderPassPS();

	if (m_currentState.m_currentRenderPass.debugName.size() > 0)
	{
		EndDebugEvent();
	}

	m_currentState.m_renderPassActive = false;
}