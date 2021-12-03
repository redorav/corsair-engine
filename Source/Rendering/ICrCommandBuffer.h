#pragma once

#include "Rendering/ICrPipeline.h"

#include "Rendering/CrGPUBuffer.h"

#include "Rendering/ICrTexture.h"

#include "Rendering/CrGPUDeletable.h"

#include "Rendering/CrRenderingStatistics.h"

#include "Rendering/CrRenderPassDescriptor.h"

#include "GeneratedShaders/ShaderMetadata.h"

#include "CrRenderingForwardDeclarations.h"

class ICrCommandBuffer : public CrGPUDeletable
{
public:

	ICrCommandBuffer(ICrCommandQueue* commandQueue);

	virtual ~ICrCommandBuffer();

	void Begin();

	void End();

	void Submit(const ICrGPUSemaphore* waitSemaphore = nullptr);

	void SetViewport(const CrViewport& viewport);

	void SetScissor(const CrScissor& scissor);

	void SetStencilRef(uint32_t stencilRef);

	// Binding functions

	void BindIndexBuffer(const CrGPUBuffer* indexBuffer);
	
	void BindVertexBuffer(const CrGPUBuffer* vertexBuffer, uint32_t streamId);

	void BindGraphicsPipelineState(const ICrGraphicsPipeline* graphicsPipeline);

	void BindComputePipelineState(const ICrComputePipeline* computePipeline);

	void BindConstantBuffer(const CrGPUBuffer* constantBuffer);

	void BindConstantBuffer(const CrGPUBuffer* constantBuffer, int32_t globalIndex);

	void BindSampler(cr3d::ShaderStage::T shaderStage, const Samplers::T samplerIndex, const ICrSampler* sampler);

	void BindTexture(cr3d::ShaderStage::T shaderStage, const Textures::T textureIndex, const ICrTexture* texture);

	void BindRWTexture(cr3d::ShaderStage::T shaderStage, const RWTextures::T rwTextureIndex, const ICrTexture* texture, uint32_t mip);

	void BindStorageBuffer(cr3d::ShaderStage::T shaderStage, const StorageBuffers::T storageBufferIndex, const CrGPUBuffer* buffer);

	void BindRWStorageBuffer(cr3d::ShaderStage::T shaderStage, const RWStorageBuffers::T rwStorageBufferIndex, const CrGPUBuffer* buffer);

	void BindRWDataBuffer(cr3d::ShaderStage::T shaderStage, const RWDataBuffers::T rwDataBufferIndex, const CrGPUBuffer* buffer);

	// Command functions

	void ClearRenderTarget(const ICrTexture* renderTarget, const float4& color, uint32_t level, uint32_t slice, uint32_t levelCount, uint32_t sliceCount);

	void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

	void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);

	void FlushGraphicsRenderState();

	void Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ);

	void FlushComputeRenderState();

	void BeginDebugEvent(const char* eventName, const float4& color);

	void EndDebugEvent();

	template<typename MetaType>
	CrGPUBufferType<MetaType> AllocateConstantBuffer();

	// This function is to be used when you know exactly what the constant buffer contains, as it will
	// treat the memory as the MetaType, even though there are fewer entries than declared in HLSL.
	// However this is useful when you know the constant buffer is an array of entries in HLSL
	template<typename MetaType>
	CrGPUBufferType<MetaType> AllocateConstantBuffer(uint32_t size);

	CrGPUBuffer AllocateConstantBuffer(uint32_t size);

	CrGPUBuffer AllocateVertexBuffer(uint32_t size);

	CrGPUBuffer AllocateIndexBuffer(uint32_t size);

	void BeginRenderPass(const CrRenderPassDescriptor& renderPassDescriptor);

	void EndRenderPass();

	const ICrCommandQueue* GetCommandQueue() const;

	const CrGPUSemaphoreSharedHandle& GetCompletionSemaphore() const;

	const CrGPUFenceSharedHandle& GetCompletionFence() const;

protected:

	virtual void BeginPS() = 0;

	virtual void EndPS() = 0;

	virtual void ClearRenderTargetPS(const ICrTexture* renderTarget, const float4& color, uint32_t level, uint32_t slice, uint32_t levelCount, uint32_t sliceCount) = 0;

	virtual void DrawPS(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;

	virtual void DrawIndexedPS(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) = 0;

	virtual void DispatchPS(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) = 0;

	virtual void BeginDebugEventPS(const char* eventName, const float4& color) = 0;

	virtual void EndDebugEventPS() = 0;

	virtual void BeginRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor) = 0;

	virtual void EndRenderPassPS() = 0;

	virtual void FlushGraphicsRenderStatePS() = 0;

	virtual void FlushComputeRenderStatePS() = 0;

	CrGPUBufferDescriptor AllocateFromGPUStack(CrGPUStackAllocator* stackAllocator, uint32_t size);

	// TODO Do all platforms support binding a buffer and an offset inside?
	struct ConstantBufferBinding
	{
		ConstantBufferBinding() {}

		ConstantBufferBinding(const ICrHardwareGPUBuffer* buffer, uint32_t byteOffset) : buffer(buffer), byteOffset(byteOffset) {}

		const ICrHardwareGPUBuffer* buffer = nullptr;
		uint32_t byteOffset = 0;
	};

	struct StorageBufferBinding
	{
		StorageBufferBinding() {}

		StorageBufferBinding(const ICrHardwareGPUBuffer* buffer, uint32_t size, uint32_t byteOffset) : buffer(buffer), size(size), byteOffset(byteOffset) {}

		const ICrHardwareGPUBuffer* buffer = nullptr;
		uint32_t byteOffset = 0;
		uint32_t size = 0;
	};

	struct RWTextureBinding
	{
		RWTextureBinding() {}

		RWTextureBinding(const ICrTexture* texture, uint32_t mip) : texture(texture), mip(mip) {}

		const ICrTexture* texture = nullptr;
		uint32_t mip = 0;
	};

	struct VertexBufferBinding
	{
		VertexBufferBinding() {}
		VertexBufferBinding(const ICrHardwareGPUBuffer* vertexBuffer, uint32_t offset)
			: vertexBuffer(vertexBuffer), offset(offset) {}

		const ICrHardwareGPUBuffer* vertexBuffer = nullptr;
		uint32_t offset = 0;
	};

	// TODO Have inline accessors here instead. We need to be able to tell if we're missing
	// a resource and even bind a dummy one if we have a safe mode

	struct CurrentState
	{
		const ConstantBufferBinding& GetConstantBufferBinding(cr3d::ShaderStage::T stage, ConstantBuffers::T id)
		{
			return m_constantBuffers[stage][id];
		}

		const ICrHardwareGPUBuffer*     m_indexBuffer;
		uint32_t                        m_indexBufferOffset;
		bool                            m_indexBufferDirty = true;

		VertexBufferBinding             m_vertexBuffers[cr3d::MaxVertexStreams];
		bool                            m_vertexBufferDirty = true;

		CrScissor                       m_scissor;
		bool                            m_scissorDirty = true;

		CrViewport                      m_viewport;
		bool                            m_viewportDirty = true;

		CrGraphicsPipelineDescriptor    m_graphicsPipelineDescriptor;
		const ICrGraphicsPipeline*      m_graphicsPipeline;
		bool                            m_graphicsPipelineDirty;

		const ICrComputePipeline*       m_computePipeline;
		bool                            m_computePipelineDirty;

		ConstantBufferBinding			m_constantBuffers[cr3d::ShaderStage::Count][ConstantBuffers::Count];

		const ICrSampler*				m_samplers[cr3d::ShaderStage::Count][Samplers::Count];

		const ICrTexture*				m_textures[cr3d::ShaderStage::Count][Textures::Count];

		RWTextureBinding				m_rwTextures[cr3d::ShaderStage::Count][RWTextures::Count];

		StorageBufferBinding			m_storageBuffers[cr3d::ShaderStage::Count][RWStorageBuffers::Count];

		StorageBufferBinding			m_rwStorageBuffers[cr3d::ShaderStage::Count][RWStorageBuffers::Count];

		const CrGPUBuffer*				m_rwDataBuffers[cr3d::ShaderStage::Count][RWDataBuffers::Count];

		uint32_t						m_stencilRef;
		bool							m_stencilRefDirty;

		CrRenderPassDescriptor			m_currentRenderPass;

		bool							m_renderPassActive;
	};

	CurrentState					m_currentState;

	ICrRenderDevice*				m_renderDevice = nullptr;

	ICrCommandQueue*				m_ownerCommandQueue = nullptr;

	CrUniquePtr<CrGPUStackAllocator> m_constantBufferGPUStack;

	CrUniquePtr<CrGPUStackAllocator> m_vertexBufferGPUStack;

	CrUniquePtr<CrGPUStackAllocator> m_indexBufferGPUStack;

	// Signal semaphore when execution completes
	CrGPUSemaphoreSharedHandle		m_completionSemaphore;

	// Signal fence when execution completes
	CrGPUFenceSharedHandle			m_completionFence;

	bool							m_submitted;
};

inline void ICrCommandBuffer::SetViewport(const CrViewport& viewport)
{
	if (m_currentState.m_viewport != viewport)
	{
		m_currentState.m_viewport = viewport;
		m_currentState.m_viewportDirty = true;
	}
}

inline void ICrCommandBuffer::SetScissor(const CrScissor& scissor)
{
	if (m_currentState.m_scissor != scissor)
	{
		m_currentState.m_scissor = scissor;
		m_currentState.m_scissorDirty = true;
	}
}

inline void ICrCommandBuffer::SetStencilRef(uint32_t stencilRef)
{
	if (m_currentState.m_stencilRef != stencilRef)
	{
		m_currentState.m_stencilRef = stencilRef;
		m_currentState.m_stencilRefDirty = true;
	}
}

inline void ICrCommandBuffer::BindIndexBuffer(const CrGPUBuffer* indexBuffer)
{
	if (m_currentState.m_indexBuffer != indexBuffer->GetHardwareBuffer() ||
		m_currentState.m_indexBufferOffset != indexBuffer->GetByteOffset())
	{
		m_currentState.m_indexBuffer = indexBuffer->GetHardwareBuffer();
		m_currentState.m_indexBufferOffset = indexBuffer->GetByteOffset();
		m_currentState.m_indexBufferDirty = true;
	}
}

inline void ICrCommandBuffer::BindVertexBuffer(const CrGPUBuffer* vertexBuffer, uint32_t streamId)
{
	if (m_currentState.m_vertexBuffers[streamId].vertexBuffer != vertexBuffer->GetHardwareBuffer() ||
		m_currentState.m_vertexBuffers[streamId].offset != vertexBuffer->GetByteOffset())
	{
		m_currentState.m_vertexBuffers[streamId].vertexBuffer = vertexBuffer->GetHardwareBuffer();
		m_currentState.m_vertexBuffers[streamId].offset = vertexBuffer->GetByteOffset();
		m_currentState.m_vertexBufferDirty = true;
	}
}

inline void ICrCommandBuffer::BindGraphicsPipelineState(const ICrGraphicsPipeline* graphicsPipeline)
{
	if (m_currentState.m_graphicsPipeline != graphicsPipeline)
	{
		m_currentState.m_graphicsPipeline = graphicsPipeline;
		m_currentState.m_graphicsPipelineDirty = true;
	}
}

inline void ICrCommandBuffer::BindComputePipelineState(const ICrComputePipeline* computePipeline)
{
	if (m_currentState.m_computePipeline != computePipeline)
	{
		m_currentState.m_computePipeline = computePipeline;
		m_currentState.m_computePipelineDirty = true;
	}	
}

inline void ICrCommandBuffer::ClearRenderTarget(const ICrTexture* renderTarget, const float4& color, uint32_t level, uint32_t slice, uint32_t levelCount, uint32_t sliceCount)
{
	ClearRenderTargetPS(renderTarget, color, level, slice, levelCount, sliceCount);
}

inline void ICrCommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	FlushGraphicsRenderState();

	DrawPS(vertexCount, instanceCount, firstVertex, firstInstance);

	CrRenderingStatistics::AddDrawcall();
	CrRenderingStatistics::AddVertices(vertexCount);
}

inline void ICrCommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	FlushGraphicsRenderState();

	DrawIndexedPS(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);

	CrRenderingStatistics::AddDrawcall();
	CrRenderingStatistics::AddVertices(indexCount * instanceCount);
}

inline void ICrCommandBuffer::FlushGraphicsRenderState()
{
	FlushGraphicsRenderStatePS();
}

inline void ICrCommandBuffer::Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
{
	FlushComputeRenderState();

	DispatchPS(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

inline void ICrCommandBuffer::FlushComputeRenderState()
{
	FlushComputeRenderStatePS();
}

inline void ICrCommandBuffer::BeginDebugEvent(const char* eventName, const float4& color)
{
	BeginDebugEventPS(eventName, color);
}

inline void ICrCommandBuffer::EndDebugEvent()
{
	EndDebugEventPS();
}

inline void ICrCommandBuffer::BindSampler(cr3d::ShaderStage::T shaderStage, const Samplers::T samplerIndex, const ICrSampler* sampler)
{
	m_currentState.m_samplers[shaderStage][samplerIndex] = sampler;
}

inline void ICrCommandBuffer::BindTexture(cr3d::ShaderStage::T shaderStage, const Textures::T textureIndex, const ICrTexture* texture)
{
	m_currentState.m_textures[shaderStage][textureIndex] = texture;
}

inline void ICrCommandBuffer::BindRWTexture(cr3d::ShaderStage::T shaderStage, const RWTextures::T textureIndex, const ICrTexture* texture, uint32_t mip)
{
	CrAssertMsg(texture->IsUnorderedAccess(), "Texture must be created with UnorderedAccess flag!");
	CrAssertMsg(texture->GetMipmapCount() > mip, "Texture doesn't have enough mipmaps!");

	m_currentState.m_rwTextures[shaderStage][textureIndex] = RWTextureBinding(texture, mip);
}

inline void ICrCommandBuffer::BindStorageBuffer(cr3d::ShaderStage::T shaderStage, const StorageBuffers::T storageBufferIndex, const CrGPUBuffer* buffer)
{
	m_currentState.m_storageBuffers[shaderStage][storageBufferIndex] = StorageBufferBinding(buffer->GetHardwareBuffer(), buffer->GetSize(), buffer->GetByteOffset());
}

inline void ICrCommandBuffer::BindRWStorageBuffer(cr3d::ShaderStage::T shaderStage, const RWStorageBuffers::T rwStorageBufferIndex, const CrGPUBuffer* buffer)
{
	m_currentState.m_rwStorageBuffers[shaderStage][rwStorageBufferIndex] = StorageBufferBinding(buffer->GetHardwareBuffer(), buffer->GetSize(), buffer->GetByteOffset());
}

inline void ICrCommandBuffer::BindRWDataBuffer(cr3d::ShaderStage::T shaderStage, const RWDataBuffers::T rwBufferIndex, const CrGPUBuffer* buffer)
{
	m_currentState.m_rwDataBuffers[shaderStage][rwBufferIndex] = buffer;
}

inline const ICrCommandQueue* ICrCommandBuffer::GetCommandQueue() const
{
	return m_ownerCommandQueue;
}

inline const CrGPUSemaphoreSharedHandle& ICrCommandBuffer::GetCompletionSemaphore() const
{
	return m_completionSemaphore;
}

inline const CrGPUFenceSharedHandle& ICrCommandBuffer::GetCompletionFence() const
{
	return m_completionFence;
}

template<typename MetaType>
inline CrGPUBufferType<MetaType> ICrCommandBuffer::AllocateConstantBuffer()
{
	return CrGPUBufferType<MetaType>(m_renderDevice, AllocateFromGPUStack(m_constantBufferGPUStack.get(), sizeof(MetaType)), 1);
}

template<typename MetaType>
inline CrGPUBufferType<MetaType> ICrCommandBuffer::AllocateConstantBuffer(uint32_t size)
{
	return CrGPUBufferType<MetaType>(m_renderDevice, AllocateFromGPUStack(m_constantBufferGPUStack.get(), size), 1);
}