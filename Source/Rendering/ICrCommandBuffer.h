#pragma once

#include "Rendering/ICrPipeline.h"

#include "Rendering/CrGPUBuffer.h"

#include "Rendering/ICrTexture.h"

#include "Rendering/CrGPUDeletable.h"

#include "Rendering/CrRenderingStatistics.h"

#include "Rendering/CrRenderPassDescriptor.h"

#include "Rendering/CrGPUStackAllocator.h"

#include "Rendering/ICrGPUSynchronization.h"

#include "GeneratedShaders/ShaderMetadata.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

#define COMMAND_BUFFER_VALIDATION

#if defined(COMMAND_BUFFER_VALIDATION)
	#define CrCommandBufferAssertMsg(condition, message, ...) CrAssertMsg(condition, message, __VA_ARGS__)
#else
	#define CrCommandBufferAssertMsg(condition, message, ...)
#endif

struct CrCommandBufferDescriptor
{
	CrCommandQueueType::T queueType = CrCommandQueueType::Graphics;

	CrFixedString64 name;

	// Use this to stream per-frame vertex data (includes index buffer)
	uint32_t dynamicVertexBufferSizeVertices = 0;

	// Use this to stream per-frame constant buffer data
	uint32_t dynamicConstantBufferSizeBytes = 0;
};

class ICrCommandBuffer : public CrGPUDeletable
{
public:

	ICrCommandBuffer(ICrRenderDevice* renderDevice, const CrCommandBufferDescriptor& descriptor);

	virtual ~ICrCommandBuffer();

	void Begin();

	void End();

	void Submit();

	void SetViewport(const CrViewport& viewport);

	void SetScissor(const CrRectangle& scissor);

	void SetStencilRef(uint32_t stencilRef);

	//--------
	// Binding
	//--------

	// Index Buffer

	void BindIndexBuffer(const ICrHardwareGPUBuffer* indexBuffer, uint32_t byteOffset, cr3d::DataFormat::T indexFormat);

	void BindIndexBuffer(const CrGPUBufferView& indexBufferView);

	void BindIndexBuffer(const CrIndexBuffer* indexBuffer, uint32_t elementOffset = 0);

	// Vertex Buffer

	void BindVertexBuffer(const ICrHardwareGPUBuffer* vertexBuffer, uint32_t streamId, uint32_t byteOffset, uint32_t vertexCount, uint32_t stride);
	
	void BindVertexBuffer(const CrGPUBufferView& vertexBufferView, uint32_t streamId);

	void BindVertexBuffer(const CrVertexBuffer* vertexBuffer, uint32_t streamId, uint32_t elementOffset = 0);

	// Constant Buffer

	void BindConstantBuffer(cr3d::ShaderStage::T shaderStage, ConstantBuffers::T constantBufferIndex, const ICrHardwareGPUBuffer* constantBuffer, uint32_t size, uint32_t offset);

	void BindConstantBuffer(cr3d::ShaderStage::T shaderStage, const CrGPUBufferView& constantBufferView);

	// Textures and Samplers

	void BindSampler(cr3d::ShaderStage::T shaderStage, Samplers::T samplerIndex, const ICrSampler* sampler);

	void BindTexture(cr3d::ShaderStage::T shaderStage, Textures::T textureIndex, const ICrTexture* texture);

	void BindRWTexture(cr3d::ShaderStage::T shaderStage, RWTextures::T rwTextureIndex, const ICrTexture* texture, uint32_t mip);

	// Storage Buffers

	void BindStorageBuffer(cr3d::ShaderStage::T shaderStage, StorageBuffers::T storageBufferIndex, const ICrHardwareGPUBuffer* buffer, uint32_t numElements, uint32_t stride, uint32_t offset);

	void BindStorageBuffer(cr3d::ShaderStage::T shaderStage, StorageBuffers::T storageBufferIndex, const ICrHardwareGPUBuffer* buffer);

	// RW Storage Buffers

	void BindRWStorageBuffer(cr3d::ShaderStage::T shaderStage, RWStorageBuffers::T storageBufferIndex, const ICrHardwareGPUBuffer* buffer, uint32_t numElements, uint32_t stride, uint32_t offset);

	void BindRWStorageBuffer(cr3d::ShaderStage::T shaderStage, RWStorageBuffers::T rwStorageBufferIndex, const ICrHardwareGPUBuffer* buffer);

	// RW Data Buffers

	void BindRWDataBuffer(cr3d::ShaderStage::T shaderStage, RWDataBuffers::T rwDataBufferIndex, const ICrHardwareGPUBuffer* buffer, uint32_t numElements, uint32_t stride, uint32_t offset);

	void BindRWDataBuffer(cr3d::ShaderStage::T shaderStage, RWDataBuffers::T rwDataBufferIndex, const ICrHardwareGPUBuffer* buffer);

	void BindGraphicsPipelineState(const ICrGraphicsPipeline* graphicsPipeline);

	void BindComputePipelineState(const ICrComputePipeline* computePipeline);

	//---------
	// Commands
	//---------

	void ClearRenderTarget(const ICrTexture* renderTarget, const float4& color, uint32_t level, uint32_t slice, uint32_t levelCount, uint32_t sliceCount);

	void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

	void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);

	void Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ);

	void DrawIndirect(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count);

	void DrawIndexedIndirect(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count);

	void DispatchIndirect(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset);

	void BeginDebugEvent(const char* eventName, const float4& color);

	void EndDebugEvent();

	void InsertDebugMarker(const char* markerName, const float4& color);

	void BeginTimestampQuery(const ICrGPUQueryPool* queryPool, CrGPUQueryId query);

	// This function is here to cater for Vulkan where we can specify the point in the pipeline the timestamp should be taken
	void EndTimestampQuery(const ICrGPUQueryPool* queryPool, CrGPUQueryId query);

	void ResetGPUQueries(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count);

	void ResolveGPUQueries(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count);

	void BeginRenderPass(const CrRenderPassDescriptor& renderPassDescriptor);

	void EndRenderPass();

	void FlushGraphicsRenderState();

	void FlushComputeRenderState();

	template<typename MetaType>
	CrGPUBufferViewT<MetaType> AllocateConstantBuffer();

	// This function is to be used when you know exactly what the constant buffer contains, as it will
	// treat the memory as the MetaType, even though there are fewer entries than declared in HLSL.
	// However this is useful when you know the constant buffer is an array of entries in HLSL
	template<typename MetaType>
	CrGPUBufferViewT<MetaType> AllocateConstantBuffer(uint32_t sizeBytes);

	CrGPUBufferView AllocateConstantBuffer(uint32_t sizeBytes);

	CrGPUBufferView AllocateVertexBuffer(uint32_t vertexCount, uint32_t stride);

	CrGPUBufferView AllocateIndexBuffer(uint32_t indexCount, cr3d::DataFormat::T indexFormat);

protected:

	virtual void BeginPS() = 0;

	virtual void EndPS() = 0;

	virtual void ClearRenderTargetPS(const ICrTexture* renderTarget, const float4& color, uint32_t level, uint32_t slice, uint32_t levelCount, uint32_t sliceCount) = 0;

	virtual void DrawPS(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;

	virtual void DrawIndexedPS(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) = 0;

	virtual void DispatchPS(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) = 0;

	virtual void DrawIndirectPS(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count) = 0;

	virtual void DrawIndexedIndirectPS(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count) = 0;

	virtual void DispatchIndirectPS(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset) = 0;

	virtual void BeginDebugEventPS(const char* eventName, const float4& color) = 0;

	virtual void EndDebugEventPS() = 0;

	virtual void InsertDebugMarkerPS(const char* markerName, const float4& color) = 0;

	virtual void BeginTimestampQueryPS(const ICrGPUQueryPool* queryPool, CrGPUQueryId query) = 0;

	virtual void EndTimestampQueryPS(const ICrGPUQueryPool* queryPool, CrGPUQueryId query) = 0;

	virtual void ResetGPUQueriesPS(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count) = 0;

	virtual void ResolveGPUQueriesPS(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count) = 0;

	virtual void BeginRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor) = 0;

	virtual void EndRenderPassPS() = 0;

	virtual void FlushGraphicsRenderStatePS() = 0;

	virtual void FlushComputeRenderStatePS() = 0;

	// TODO Do all platforms support binding a buffer and an offset inside?
	struct ConstantBufferBinding
	{
		ConstantBufferBinding() = default;

		ConstantBufferBinding(const ICrHardwareGPUBuffer* buffer, uint32_t sizeBytes, uint32_t offsetBytes) : buffer(buffer), sizeBytes(sizeBytes), offsetBytes(offsetBytes) {}

		const ICrHardwareGPUBuffer* buffer = nullptr;
		uint32_t offsetBytes = 0;
		uint32_t sizeBytes = 0;
	};

	struct StorageBufferBinding
	{
		StorageBufferBinding() = default;

		StorageBufferBinding(const ICrHardwareGPUBuffer* buffer, uint32_t numElements, uint32_t strideBytes, uint32_t offsetBytes)
			: buffer(buffer), numElements(numElements), offsetBytes(offsetBytes), strideBytes(strideBytes)
		{
			sizeBytes = numElements * strideBytes;
		}

		const ICrHardwareGPUBuffer* buffer = nullptr;
		uint32_t numElements = 0;
		uint32_t offsetBytes = 0;
		uint32_t sizeBytes = 0;
		uint32_t strideBytes = 0;
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
		VertexBufferBinding() = default;
		VertexBufferBinding(const ICrHardwareGPUBuffer* vertexBuffer, uint32_t vertexCount, uint32_t offset, uint32_t stride)
			: vertexBuffer(vertexBuffer), vertexCount(vertexCount), offset(offset), stride(stride) {}

		const ICrHardwareGPUBuffer* vertexBuffer = nullptr;
		uint32_t vertexCount = 0;
		uint32_t offset = 0;
		uint32_t stride = 0;
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
		cr3d::DataFormat::T             m_indexBufferFormat;
		bool                            m_indexBufferDirty = false;

		VertexBufferBinding             m_vertexBuffers[cr3d::MaxVertexStreams];
		bool                            m_vertexBufferDirty = false;

		CrRectangle                     m_scissor;
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

		StorageBufferBinding			m_rwDataBuffers[cr3d::ShaderStage::Count][RWDataBuffers::Count];

		uint32_t						m_stencilRef;
		bool							m_stencilRefDirty;

		CrRenderPassDescriptor			m_currentRenderPass;

		bool							m_renderPassActive;
	};

	CurrentState					m_currentState;

	CrUniquePtr<CrGPUStackAllocator> m_constantBufferGPUStack;

	CrUniquePtr<CrGPUStackAllocator> m_vertexBufferGPUStack;

	CrUniquePtr<CrGPUStackAllocator> m_indexBufferGPUStack;

	// Signal fence when execution completes
	CrGPUFenceHandle			m_completionFence;

	CrCommandQueueType::T			m_queueType;

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

inline void ICrCommandBuffer::SetScissor(const CrRectangle& scissor)
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

inline void ICrCommandBuffer::BindIndexBuffer(const ICrHardwareGPUBuffer* indexBuffer, uint32_t byteOffset, cr3d::DataFormat::T indexFormat)
{
	CrCommandBufferAssertMsg(indexBuffer != nullptr, "Buffer is null");
	CrCommandBufferAssertMsg(indexBuffer->GetUsage() & cr3d::BufferUsage::Index, "Buffer must have index buffer flag");
	CrCommandBufferAssertMsg(indexFormat == cr3d::DataFormat::R16_Uint || indexFormat == cr3d::DataFormat::R32_Uint, "Only these formats are allowed");

	if (m_currentState.m_indexBuffer != indexBuffer ||
		m_currentState.m_indexBufferOffset != byteOffset ||
		m_currentState.m_indexBufferFormat != indexFormat)
	{
		m_currentState.m_indexBuffer = indexBuffer;
		m_currentState.m_indexBufferOffset = byteOffset;
		m_currentState.m_indexBufferFormat = indexFormat;
		m_currentState.m_indexBufferDirty = true;
	}
}

inline void ICrCommandBuffer::BindIndexBuffer(const CrGPUBufferView& indexBufferView)
{
	BindIndexBuffer(indexBufferView.GetHardwareBuffer(), indexBufferView.GetByteOffset(), indexBufferView.GetFormat());
}

inline void ICrCommandBuffer::BindIndexBuffer(const CrIndexBuffer* indexBuffer, uint32_t elementOffset)
{
	BindIndexBuffer(indexBuffer->GetHardwareBuffer(), elementOffset * indexBuffer->GetStride(), indexBuffer->GetFormat());
}

inline void ICrCommandBuffer::BindVertexBuffer(const ICrHardwareGPUBuffer* vertexBuffer, uint32_t streamId, uint32_t byteOffset, uint32_t vertexCount, uint32_t stride)
{
	CrCommandBufferAssertMsg(vertexBuffer != nullptr, "Buffer is null");
	CrCommandBufferAssertMsg(vertexBuffer->GetUsage() & cr3d::BufferUsage::Vertex, "Buffer must have vertex buffer flag");
	CrCommandBufferAssertMsg(stride < 2048, "Stride is too large");

	if (m_currentState.m_vertexBuffers[streamId].vertexBuffer != vertexBuffer || m_currentState.m_vertexBuffers[streamId].offset != byteOffset ||
		m_currentState.m_vertexBuffers[streamId].vertexCount != vertexCount || m_currentState.m_vertexBuffers[streamId].stride != stride)
	{
		m_currentState.m_vertexBuffers[streamId].vertexBuffer = vertexBuffer;
		m_currentState.m_vertexBuffers[streamId].vertexCount = vertexCount;
		m_currentState.m_vertexBuffers[streamId].offset = byteOffset;
		m_currentState.m_vertexBuffers[streamId].stride = stride;
		m_currentState.m_vertexBufferDirty = true;
	}
}

inline void ICrCommandBuffer::BindVertexBuffer(const CrGPUBufferView& vertexBufferView, uint32_t streamId)
{
	BindVertexBuffer(vertexBufferView.GetHardwareBuffer(), streamId, vertexBufferView.GetByteOffset(), vertexBufferView.GetNumElements(), vertexBufferView.GetStride());
}

inline void ICrCommandBuffer::BindVertexBuffer(const CrVertexBuffer* vertexBuffer, uint32_t streamId, uint32_t elementOffset)
{
	BindVertexBuffer(vertexBuffer->GetHardwareBuffer(), streamId, elementOffset * vertexBuffer->GetStride(), vertexBuffer->GetNumElements(), vertexBuffer->GetStride());
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
	CrRenderingStatistics::AddInstances(instanceCount);
}

inline void ICrCommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	FlushGraphicsRenderState();

	DrawIndexedPS(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);

	CrRenderingStatistics::AddDrawcall();
	CrRenderingStatistics::AddVertices(indexCount * instanceCount);
	CrRenderingStatistics::AddInstances(instanceCount);
}

inline void ICrCommandBuffer::DrawIndirect(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count)
{
	FlushGraphicsRenderState();

	DrawIndirectPS(indirectBuffer, offset, count);

	CrRenderingStatistics::AddDrawcall();
	// We cannot add the vertices here because we don't know them
}

inline void ICrCommandBuffer::DrawIndexedIndirect(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count)
{
	FlushGraphicsRenderState();

	DrawIndexedIndirectPS(indirectBuffer, offset, count);

	CrRenderingStatistics::AddDrawcall();
	// We cannot add the vertices here because we don't know them
}

inline void ICrCommandBuffer::DispatchIndirect(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset)
{
	FlushComputeRenderState();

	DispatchIndirectPS(indirectBuffer, offset);
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

inline void ICrCommandBuffer::InsertDebugMarker(const char* markerName, const float4& color)
{
	InsertDebugMarkerPS(markerName, color);
}

inline void ICrCommandBuffer::BindConstantBuffer(cr3d::ShaderStage::T shaderStage, ConstantBuffers::T constantBufferIndex, const ICrHardwareGPUBuffer* constantBuffer, uint32_t size, uint32_t offset)
{
	CrCommandBufferAssertMsg(constantBuffer != nullptr, "Buffer is null");
	CrCommandBufferAssertMsg(constantBuffer->HasUsage(cr3d::BufferUsage::Constant), "Buffer must have constant buffer flag");
	CrCommandBufferAssertMsg(constantBufferIndex < ConstantBuffers::Count, "Invalid binding index");

	m_currentState.m_constantBuffers[shaderStage][constantBufferIndex] = ConstantBufferBinding(constantBuffer, size, offset);
}

inline void ICrCommandBuffer::BindConstantBuffer(cr3d::ShaderStage::T shaderStage, const CrGPUBufferView& constantBufferView)
{
	BindConstantBuffer(shaderStage, (ConstantBuffers::T)constantBufferView.GetBindingIndex(), constantBufferView.GetHardwareBuffer(), constantBufferView.GetSize(), constantBufferView.GetByteOffset());
}

inline void ICrCommandBuffer::BindSampler(cr3d::ShaderStage::T shaderStage, const Samplers::T samplerIndex, const ICrSampler* sampler)
{
	CrCommandBufferAssertMsg(sampler != nullptr, "Sampler is null");
	CrCommandBufferAssertMsg(samplerIndex < Samplers::Count, "Invalid binding index");

	m_currentState.m_samplers[shaderStage][samplerIndex] = sampler;
}

inline void ICrCommandBuffer::BindTexture(cr3d::ShaderStage::T shaderStage, const Textures::T textureIndex, const ICrTexture* texture)
{
	CrCommandBufferAssertMsg(texture != nullptr, "Texture is null");
	CrCommandBufferAssertMsg(textureIndex < Textures::Count, "Invalid binding index");

	m_currentState.m_textures[shaderStage][textureIndex] = texture;
}

inline void ICrCommandBuffer::BindRWTexture(cr3d::ShaderStage::T shaderStage, const RWTextures::T rwTextureIndex, const ICrTexture* texture, uint32_t mip)
{
	CrCommandBufferAssertMsg(texture != nullptr, "Texture is null");
	CrCommandBufferAssertMsg(texture->IsUnorderedAccess(), "Texture must be created with UnorderedAccess flag");
	CrCommandBufferAssertMsg(mip < texture->GetMipmapCount(), "Texture doesn't have enough mipmaps!");
	CrCommandBufferAssertMsg(rwTextureIndex < RWTextures::Count, "Invalid binding index");

	m_currentState.m_rwTextures[shaderStage][rwTextureIndex] = RWTextureBinding(texture, mip);
}

inline void ICrCommandBuffer::BindStorageBuffer(cr3d::ShaderStage::T shaderStage, StorageBuffers::T storageBufferIndex, const ICrHardwareGPUBuffer* buffer, uint32_t numElements, uint32_t stride, uint32_t offset)
{
	CrCommandBufferAssertMsg(buffer != nullptr, "Buffer is null");
	CrCommandBufferAssertMsg(buffer->HasUsage(cr3d::BufferUsage::Storage), "Buffer must have storage buffer flag");
	CrCommandBufferAssertMsg(storageBufferIndex < StorageBuffers::Count, "Invalid binding index");

	m_currentState.m_storageBuffers[shaderStage][storageBufferIndex] = StorageBufferBinding(buffer, numElements, stride, offset);
}

inline void ICrCommandBuffer::BindStorageBuffer(cr3d::ShaderStage::T shaderStage, StorageBuffers::T storageBufferIndex, const ICrHardwareGPUBuffer* buffer)
{
	BindStorageBuffer(shaderStage, storageBufferIndex, buffer, buffer->GetNumElements(), buffer->GetStrideBytes(), 0);
}

inline void ICrCommandBuffer::BindRWStorageBuffer(cr3d::ShaderStage::T shaderStage, RWStorageBuffers::T rwStorageBufferIndex, const ICrHardwareGPUBuffer* buffer, uint32_t numElements, uint32_t stride, uint32_t offset)
{
	CrCommandBufferAssertMsg(buffer != nullptr, "Buffer is null");
	CrCommandBufferAssertMsg(buffer->HasUsage(cr3d::BufferUsage::Storage), "Buffer must have storage buffer flag");
	CrCommandBufferAssertMsg(buffer->HasAccess(cr3d::MemoryAccess::GPUOnlyWrite) || buffer->HasAccess(cr3d::MemoryAccess::GPUWriteCPURead), "Buffer must be GPU-writable");
	CrCommandBufferAssertMsg(rwStorageBufferIndex < RWStorageBuffers::Count, "Invalid binding index");

	m_currentState.m_rwStorageBuffers[shaderStage][rwStorageBufferIndex] = StorageBufferBinding(buffer, numElements, stride, offset);
}

inline void ICrCommandBuffer::BindRWStorageBuffer(cr3d::ShaderStage::T shaderStage, RWStorageBuffers::T rwStorageBufferIndex, const ICrHardwareGPUBuffer* buffer)
{
	BindRWStorageBuffer(shaderStage, rwStorageBufferIndex, buffer, buffer->GetNumElements(), buffer->GetStrideBytes(), 0);
}

inline void ICrCommandBuffer::BindRWDataBuffer(cr3d::ShaderStage::T shaderStage, RWDataBuffers::T rwBufferIndex, const ICrHardwareGPUBuffer* buffer, uint32_t numElements, uint32_t stride, uint32_t offset)
{
	(numElements); (stride); (offset);

	CrCommandBufferAssertMsg(buffer != nullptr, "Buffer is null");
	CrCommandBufferAssertMsg(buffer->HasUsage(cr3d::BufferUsage::Data), "Buffer must have data buffer flag");
	CrCommandBufferAssertMsg(buffer->HasAccess(cr3d::MemoryAccess::GPUOnlyWrite) || buffer->HasAccess(cr3d::MemoryAccess::GPUWriteCPURead), "Buffer must be GPU-writable");
	CrCommandBufferAssertMsg(rwBufferIndex < RWDataBuffers::Count, "Invalid binding index");

	m_currentState.m_rwDataBuffers[shaderStage][rwBufferIndex].buffer = buffer;
}

inline void ICrCommandBuffer::BindRWDataBuffer(cr3d::ShaderStage::T shaderStage, RWDataBuffers::T rwDataBufferIndex, const ICrHardwareGPUBuffer* buffer)
{
	BindRWDataBuffer(shaderStage, rwDataBufferIndex, buffer, 1, 1, 0);
}

template<typename MetaType>
inline CrGPUBufferViewT<MetaType> ICrCommandBuffer::AllocateConstantBuffer(uint32_t sizeBytes)
{
	CrStackAllocation<void> allocation = m_constantBufferGPUStack->AllocateAligned(sizeBytes, 256);

	CrGPUBufferViewT<MetaType> constantBufferView
	(
		m_constantBufferGPUStack->GetHardwareGPUBuffer(),
		1,
		sizeBytes,
		allocation.offset,
		allocation.memory
	);

	return constantBufferView;
}

template<typename MetaType>
inline CrGPUBufferViewT<MetaType> ICrCommandBuffer::AllocateConstantBuffer()
{
	return AllocateConstantBuffer<MetaType>(sizeof(MetaType));
}