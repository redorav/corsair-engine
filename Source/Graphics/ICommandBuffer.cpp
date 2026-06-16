#include "Graphics/CrRendering_pch.h"

#include "ICommandBuffer.h"
#include "IDevice.h"
#include "ICrPipeline.h"

#include "ICrShader.h"
#include "CrGPUStackAllocator.h"
#include "CrCPUStackAllocator.h"

#include "Core/CrMacros.h"

namespace crgfx
{
	ICommandBuffer::ICommandBuffer(crgfx::IDevice* renderDevice, const crgfx::CommandBufferDescriptor& descriptor) : CrGPUAutoDeletable(renderDevice)
		, m_queueType(descriptor.queueType)
		, m_submitted(false)
		, m_recording(false)
	{
		// Initialize GPU buffer stack allocators - for streaming
		// TODO it is possible to create these lazily on allocation instead to avoid having every command buffer allocate these if they aren't going to be used
		// TODO Create a single buffer that supports vertex, index and GPU buffer data. We don't really care where things come from, just need a buffer
		// Investigate whether it's actually possible to do that if certain buffers need to be in certain states via transitions
		{
			if (descriptor.dynamicBufferSizeBytes > 0)
			{
				CrHardwareGPUBufferDescriptor gpuBufferStack(crgfx::BufferUsage::Constant | crgfx::BufferUsage::Structured, crgfx::MemoryAccess::CPUStreamToGPU, descriptor.dynamicBufferSizeBytes);
				gpuBufferStack.name = "GPU Buffer Stack";
				m_bufferGPUStack = crstl::unique_ptr<CrGPUStackAllocator>(new CrGPUStackAllocator(m_renderDevice, gpuBufferStack));
			}

			// Allocate memory for transient vertex and index data. This is just an approximation as it depends on the size of each vertex and index
			// TODO We need a more flexible approach as this takes up a lot of memory per command buffer. It would be smart to have a pool in the device
			// and lazily allocate them or something similar
			if (descriptor.dynamicVertexBufferSizeVertices > 0)
			{
				uint32_t maxVertices = descriptor.dynamicVertexBufferSizeVertices;
				uint32_t maxIndices = maxVertices * 3;

				CrHardwareGPUBufferDescriptor vertexBufferStack(crgfx::BufferUsage::Vertex, crgfx::MemoryAccess::CPUStreamToGPU, maxVertices, 4);
				vertexBufferStack.name = "Vertex Buffer Stack";
				m_vertexBufferGPUStack = crstl::unique_ptr<CrGPUStackAllocator>(new CrGPUStackAllocator(m_renderDevice, vertexBufferStack));

				CrHardwareGPUBufferDescriptor indexBufferStack(crgfx::BufferUsage::Index, crgfx::MemoryAccess::CPUStreamToGPU, maxIndices, crgfx::DataFormats[crgfx::DataFormat::R16_Uint].dataOrBlockSize);
				indexBufferStack.name = "Index Buffer Stack";
				m_indexBufferGPUStack = crstl::unique_ptr<CrGPUStackAllocator>(new CrGPUStackAllocator(m_renderDevice, indexBufferStack));
			}
		}

		m_completionFence = m_renderDevice->CreateGPUFence();
	}

	ICommandBuffer::~ICommandBuffer()
	{

	}

	void ICommandBuffer::Begin()
	{
		CrAssertMsg(!m_recording, "Cannot begin a command buffer while it is recording");

		m_recording = true;

		// Reset current state. When beginning a new command buffer 
		// any bound state is also reset, and our tracking must match
		m_currentState = CurrentState();

		if (m_bufferGPUStack)
		{
			m_bufferGPUStack->Begin();
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
			crgfx::GPUFenceResult result = m_renderDevice->WaitForFence(m_completionFence.get(), UINT64_MAX);
			CrAssertMsg(result == crgfx::GPUFenceResult::Success, "Failed waiting for fence");
			m_renderDevice->ResetFence(m_completionFence.get());
			m_submitted = false;
		}

		BeginPS();
	}

	void ICommandBuffer::End()
	{
		if (m_bufferGPUStack)
		{
			m_bufferGPUStack->End();
		}

		if (m_vertexBufferGPUStack)
		{
			m_vertexBufferGPUStack->End();
			m_indexBufferGPUStack->End();
		}

		EndPS();

		m_recording = false;
	}

	void ICommandBuffer::Submit()
	{
		// We need this flag to be able to reset fences properly during the begin
		m_submitted = true;

		// Submission will signal the internal semaphore of this command buffer
		m_renderDevice->SubmitCommandBuffer(this, nullptr, nullptr, m_completionFence.get());
	}

	void ICommandBuffer::BeginTimestampQuery(const ICrGPUQueryPool* queryPool, CrGPUQueryId query)
	{
		BeginTimestampQueryPS(queryPool, query);
	}

	void ICommandBuffer::EndTimestampQuery(const ICrGPUQueryPool* queryPool, CrGPUQueryId query)
	{
		EndTimestampQueryPS(queryPool, query);
	}

	void ICommandBuffer::ResetGPUQueries(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count)
	{
		ResetGPUQueriesPS(queryPool, start, count);
	}

	void ICommandBuffer::ResolveGPUQueries(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count)
	{
		ResolveGPUQueriesPS(queryPool, start, count);
	}

	CrGPUBufferView ICommandBuffer::AllocateConstantBuffer(uint32_t sizeBytes)
	{
		CrStackAllocation<void> allocation = m_bufferGPUStack->AllocateAligned(sizeBytes, 256);

		CrGPUBufferView constantBufferView
		(
			m_bufferGPUStack->GetHardwareGPUBuffer(),
			1,
			sizeBytes,
			allocation.offset,
			allocation.memory
		);

		return constantBufferView;
	}

	CrGPUBufferView ICommandBuffer::AllocateVertexBuffer(uint32_t vertexCount, uint32_t stride)
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

	CrGPUBufferView ICommandBuffer::AllocateIndexBuffer(uint32_t indexCount, crgfx::DataFormat::T indexFormat)
	{
		uint32_t sizeBytes = indexCount * crgfx::DataFormats[indexFormat].dataOrBlockSize;

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

	void ICommandBuffer::BeginRenderPass(const crgfx::RenderPassDescriptor& renderPassDescriptor)
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
			const RenderTargetDescriptor& renderTargetDescriptor = renderPassDescriptor.color[i];
			if (renderTargetDescriptor.loadOp == crgfx::RenderTargetLoadOp::Load && renderTargetDescriptor.initialState.layout == crgfx::TextureLayout::Undefined)
			{
				CrCommandBufferAssertMsg(false, "Invalid combination");
			}

			if (renderTargetDescriptor.initialState.layout != crgfx::TextureLayout::Undefined)
			{
				CrCommandBufferAssertMsg(renderTargetDescriptor.initialState.stages != crgfx::ShaderStageFlags::None, "");
			}

			CrCommandBufferAssertMsg(renderTargetDescriptor.usageState.stages != crgfx::ShaderStageFlags::None, "");
			CrCommandBufferAssertMsg(renderTargetDescriptor.finalState.stages != crgfx::ShaderStageFlags::None, "");
		}

		if (renderPassDescriptor.depth.texture)
		{
			if (renderPassDescriptor.depth.initialState.layout != crgfx::TextureLayout::Undefined)
			{
				CrCommandBufferAssertMsg(renderPassDescriptor.depth.initialState.stages != crgfx::ShaderStageFlags::None, "");
			}

			CrCommandBufferAssertMsg(renderPassDescriptor.depth.usageState.stages != crgfx::ShaderStageFlags::None, "");
			CrCommandBufferAssertMsg(renderPassDescriptor.depth.finalState.stages != crgfx::ShaderStageFlags::None, "");
		}
#endif

		BeginRenderPassPS(renderPassDescriptor);
	}

	void ICommandBuffer::EndRenderPass()
	{
		CrCommandBufferAssertMsg(m_currentState.m_renderPassActive, "Render pass not active. Have you forgotten to start a render pass?");

		EndRenderPassPS();

		if (m_currentState.m_currentRenderPass.debugName.size() > 0)
		{
			EndDebugEvent();
		}

		m_currentState.m_renderPassActive = false;
	}
};