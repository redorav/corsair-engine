#pragma once

#include "Graphics/IPipeline.h"
#include "Graphics/GPUBuffer.h"
#include "Graphics/ITexture.h"
#include "Graphics/GPUDeletable.h"
#include "Graphics/CrRenderingStatistics.h"
#include "Graphics/RenderPassDescriptor.h"
#include "Graphics/CrGPUStackAllocator.h"
#include "Graphics/IGPUSynchronization.h"
#include "Graphics/CrGraphicsForwardDeclarations.h"

#include "GeneratedShaders/ShaderMetadata.h"

#define COMMAND_BUFFER_VALIDATION

#if defined(COMMAND_BUFFER_VALIDATION)
	#define CrCommandBufferAssertMsg(condition, message, ...) CrAssertMsg(condition, message, __VA_ARGS__)
#else
	#define CrCommandBufferAssertMsg(condition, message, ...)
#endif

namespace crgfx
{
	struct CommandBufferDescriptor
	{
		CommandQueueType::T queueType = CommandQueueType::Graphics;

		crstl::fixed_string64 name;

		// Use this to stream per-frame vertex data (includes index buffer)
		uint32_t dynamicVertexBufferSizeVertices = 0;

		// Use this to stream per-frame constant buffer data
		uint32_t dynamicBufferSizeBytes = 0;
	};

	// TODO Do all platforms support binding a buffer and an offset inside?
	struct ConstantBufferBinding
	{
		ConstantBufferBinding() = default;

		ConstantBufferBinding(const IHardwareGPUBuffer* buffer, uint32_t sizeBytes, uint32_t offsetBytes) : buffer(buffer), sizeBytes(sizeBytes), offsetBytes(offsetBytes) {}

		const IHardwareGPUBuffer* buffer = nullptr;
		uint32_t sizeBytes = 0;
		uint32_t offsetBytes = 0;
	};

	struct StorageBufferBinding
	{
		StorageBufferBinding() = default;

		StorageBufferBinding(const IHardwareGPUBuffer* buffer, uint32_t numElements, uint32_t strideBytes, uint32_t offsetBytes)
			: buffer(buffer), numElements(numElements), offsetBytes(offsetBytes), strideBytes(strideBytes)
		{
			sizeBytes = numElements * strideBytes;
		}

		const IHardwareGPUBuffer* buffer = nullptr;
		uint32_t numElements = 0;
		uint32_t offsetBytes = 0;
		uint32_t sizeBytes = 0;
		uint32_t strideBytes = 0;
	};

	struct TextureBinding
	{
		TextureBinding() = default;

		TextureBinding(const ITexture* texture, TextureView view) : texture(texture), view(view) {}

		const ITexture* texture = nullptr;

		TextureView view;
	};

	struct RWTextureBinding
	{
		RWTextureBinding() {}

		RWTextureBinding(const ITexture* texture, uint32_t mip) : texture(texture), mip(mip) {}

		const ITexture* texture = nullptr;
		uint32_t mip = 0;
	};

	struct VertexBufferBinding
	{
		VertexBufferBinding() = default;
		VertexBufferBinding(const IHardwareGPUBuffer* vertexBuffer, uint32_t vertexCount, uint32_t offset, uint32_t stride)
			: vertexBuffer(vertexBuffer), vertexCount(vertexCount), offset(offset), stride(stride) {
		}

		const IHardwareGPUBuffer* vertexBuffer = nullptr;
		uint32_t vertexCount = 0;
		uint32_t offset = 0;
		uint32_t stride = 0;
	};

	class ICommandBuffer : public CrGPUAutoDeletable
	{
	public:

		ICommandBuffer(IDevice* renderDevice, const CommandBufferDescriptor& descriptor);

		virtual ~ICommandBuffer();

		void Begin();

		void End();

		void Submit();

		void SetViewport(const Viewport& viewport);

		void SetScissor(const Rectangle& scissor);

		void SetStencilRef(uint32_t stencilRef);

		//--------
		// Binding
		//--------

		// Index Buffer

		void BindIndexBuffer(const IHardwareGPUBuffer* indexBuffer, uint32_t byteOffset, uint32_t sizeBytes, DataFormat::T indexFormat);

		void BindIndexBuffer(const CrGPUBufferView& indexBufferView);

		void BindIndexBuffer(const IndexBuffer* indexBuffer, uint32_t elementOffset = 0);

		// Vertex Buffer

		void BindVertexBuffer(const IHardwareGPUBuffer* vertexBuffer, uint32_t streamId, uint32_t byteOffset, uint32_t vertexCount, uint32_t stride);

		void BindVertexBuffer(const CrGPUBufferView& vertexBufferView, uint32_t streamId);

		void BindVertexBuffer(const VertexBuffer* vertexBuffer, uint32_t streamId, uint32_t elementOffset = 0);

		// Constant Buffer

		void BindConstantBuffer(ConstantBuffers::T constantBufferIndex, const IHardwareGPUBuffer* constantBuffer, uint32_t size, uint32_t offset);

		void BindConstantBuffer(const CrGPUBufferView& constantBufferView);

		// Textures and Samplers

		void BindSampler(Samplers::T samplerIndex, const ISampler* sampler);

		void BindTexture(Textures::T textureIndex, const ITexture* texture, TextureView view = TextureView());

		void BindRWTexture(RWTextures::T rwTextureIndex, const ITexture* texture, uint32_t mip);

		// Storage Buffers

		void BindStorageBuffer(StorageBuffers::T storageBufferIndex, const IHardwareGPUBuffer* buffer, uint32_t numElements, uint32_t stride, uint32_t offset);

		void BindStorageBuffer(StorageBuffers::T storageBufferIndex, const IHardwareGPUBuffer* buffer);

		void BindStorageBuffer(const CrGPUBufferView& storageBufferView);

		// RW Storage Buffers

		void BindRWStorageBuffer(RWStorageBuffers::T storageBufferIndex, const IHardwareGPUBuffer* buffer, uint32_t numElements, uint32_t stride, uint32_t offset);

		void BindRWStorageBuffer(RWStorageBuffers::T rwStorageBufferIndex, const IHardwareGPUBuffer* buffer);

		// RW Data Buffers

		void BindRWTypedBuffer(RWTypedBuffers::T rwTypedBufferIndex, const IHardwareGPUBuffer* buffer, uint32_t numElements, uint32_t stride, uint32_t offset);

		void BindRWTypedBuffer(RWTypedBuffers::T rwTypedBufferIndex, const IHardwareGPUBuffer* buffer);

		void BindGraphicsPipelineState(const IGraphicsPipeline* graphicsPipeline);

		void BindComputePipelineState(const IComputePipeline* computePipeline);

		//---------
		// Commands
		//---------

		void ClearRenderTarget(const ITexture* renderTarget, const float4& color, uint32_t mip, uint32_t slice, uint32_t mipCount, uint32_t sliceCount);

		void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);

		void Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ);

		void DrawIndirect(const IHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count);

		void DispatchTexture1D(uint32_t textureWidth);

		void DispatchTexture2D(uint32_t textureWidth, uint32_t textureHeight);

		void DispatchTexture3D(uint32_t textureWidth, uint32_t textureHeight, uint32_t textureDepth);

		void DrawIndexedIndirect(const IHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count);

		void DispatchIndirect(const IHardwareGPUBuffer* indirectBuffer, uint32_t offset);

		void BeginDebugEvent(const char* eventName, const float4& color);

		void EndDebugEvent();

		void InsertDebugMarker(const char* markerName, const float4& color);

		void BeginTimestampQuery(const IGPUQueryPool* queryPool, CrGPUQueryId query);

		// This function is here to cater for Vulkan where we can specify the point in the pipeline the timestamp should be taken
		void EndTimestampQuery(const IGPUQueryPool* queryPool, CrGPUQueryId query);

		void ResetGPUQueries(const IGPUQueryPool* queryPool, uint32_t start, uint32_t count);

		void ResolveGPUQueries(const IGPUQueryPool* queryPool, uint32_t start, uint32_t count);

		void BeginRenderPass(const RenderPassDescriptor& renderPassDescriptor);

		void EndRenderPass();

		void FlushGraphicsRenderState();

		void FlushComputeRenderState();

		template<typename MetaType>
		CrGPUBufferViewT<MetaType> AllocateConstantBuffer();

		// This function is to be used when you know exactly what the constant buffer contains, as it will
		// treat the memory as the MetaType, even though there are fewer entries than declared in HLSL.
		// However this is useful when you know the constant buffer is an array of entries in HLSL
		template<typename MetaType>
		CrGPUBufferViewT<MetaType> AllocateConstantBuffer(uint32_t instanceCount);

		template<typename MetaType>
		CrGPUBufferViewT<MetaType> AllocateConstantBuffer(uint32_t instanceSizeBytes, uint32_t instanceCount);

		CrGPUBufferView AllocateConstantBuffer(uint32_t sizeBytes);

		template<typename MetaType>
		inline CrGPUBufferViewT<MetaType> AllocateStorageBuffer(uint32_t instanceCount);

		CrGPUBufferView AllocateVertexBuffer(uint32_t vertexCount, uint32_t stride);

		CrGPUBufferView AllocateIndexBuffer(uint32_t indexCount, DataFormat::T indexFormat);

	protected:

		virtual void BeginPS() = 0;

		virtual void EndPS() = 0;

		virtual void ClearRenderTargetPS(const ITexture* renderTarget, const float4& color, uint32_t mip, uint32_t slice, uint32_t mipCount, uint32_t sliceCount) = 0;

		virtual void DrawPS(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;

		virtual void DrawIndexedPS(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) = 0;

		virtual void DispatchPS(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) = 0;

		virtual void DrawIndirectPS(const IHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count) = 0;

		virtual void DrawIndexedIndirectPS(const IHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count) = 0;

		virtual void DispatchIndirectPS(const IHardwareGPUBuffer* indirectBuffer, uint32_t offset) = 0;

		virtual void BeginDebugEventPS(const char* eventName, const float4& color) = 0;

		virtual void EndDebugEventPS() = 0;

		virtual void InsertDebugMarkerPS(const char* markerName, const float4& color) = 0;

		virtual void BeginTimestampQueryPS(const IGPUQueryPool* queryPool, CrGPUQueryId query) = 0;

		virtual void EndTimestampQueryPS(const IGPUQueryPool* queryPool, CrGPUQueryId query) = 0;

		virtual void ResetGPUQueriesPS(const IGPUQueryPool* queryPool, uint32_t start, uint32_t count) = 0;

		virtual void ResolveGPUQueriesPS(const IGPUQueryPool* queryPool, uint32_t start, uint32_t count) = 0;

		virtual void BeginRenderPassPS(const RenderPassDescriptor& renderPassDescriptor) = 0;

		virtual void EndRenderPassPS() = 0;

		virtual void FlushGraphicsRenderStatePS() = 0;

		virtual void FlushComputeRenderStatePS() = 0;

		// TODO Have inline accessors here instead. We need to be able to tell if we're missing
		// a resource and even bind a dummy one if we have a safe mode

		struct CurrentState
		{
			const IHardwareGPUBuffer* m_indexBuffer;
			uint32_t                        m_indexBufferOffset;
			uint32_t                        m_indexBufferSize;
			crgfx::DataFormat::T            m_indexBufferFormat;
			bool                            m_indexBufferDirty = false;

			VertexBufferBinding           m_vertexBuffers[crgfx::MaxVertexStreams];
			bool                            m_vertexBufferDirty = false;

			crgfx::Rectangle                m_scissor;
			bool                            m_scissorDirty = true;

			crgfx::Viewport                 m_viewport;
			bool                            m_viewportDirty = true;

			const IGraphicsPipeline* m_graphicsPipeline;
			bool                            m_graphicsPipelineDirty;

			const IComputePipeline* m_computePipeline;
			bool                            m_computePipelineDirty;

			ConstantBufferBinding         m_constantBuffers[ConstantBuffers::Count];

			const crgfx::ISampler* m_samplers[Samplers::Count];

			TextureBinding                m_textures[Textures::Count];

			RWTextureBinding              m_rwTextures[RWTextures::Count];

			StorageBufferBinding          m_storageBuffers[StorageBuffers::Count];

			StorageBufferBinding          m_rwStorageBuffers[RWStorageBuffers::Count];

			StorageBufferBinding          m_rwTypedBuffers[RWTypedBuffers::Count];

			uint32_t                        m_stencilRef;
			bool                            m_stencilRefDirty;

			crgfx::RenderPassDescriptor   m_currentRenderPass;

			bool                            m_renderPassActive;
		};

		CurrentState m_currentState;

		crstl::unique_ptr<CrGPUStackAllocator> m_bufferGPUStack;

		crstl::unique_ptr<CrGPUStackAllocator> m_vertexBufferGPUStack;

		crstl::unique_ptr<CrGPUStackAllocator> m_indexBufferGPUStack;

		// Signal fence when execution completes
		crgfx::GPUFenceHandle      m_completionFence;

		crgfx::CommandQueueType::T   m_queueType;

		bool                         m_submitted;

		// If this command buffer is recording commands. We cannot begin a command list twice
		bool                         m_recording;
	};

	inline void ICommandBuffer::SetViewport(const crgfx::Viewport& viewport)
	{
		if (m_currentState.m_viewport != viewport)
		{
			m_currentState.m_viewport = viewport;
			m_currentState.m_viewportDirty = true;
		}
	}

	inline void ICommandBuffer::SetScissor(const crgfx::Rectangle& scissor)
	{
		if (m_currentState.m_scissor != scissor)
		{
			m_currentState.m_scissor = scissor;
			m_currentState.m_scissorDirty = true;
		}
	}

	inline void ICommandBuffer::SetStencilRef(uint32_t stencilRef)
	{
		if (m_currentState.m_stencilRef != stencilRef)
		{
			m_currentState.m_stencilRef = stencilRef;
			m_currentState.m_stencilRefDirty = true;
		}
	}

	inline void ICommandBuffer::BindIndexBuffer(const IHardwareGPUBuffer* indexBuffer, uint32_t byteOffset, uint32_t sizeBytes, crgfx::DataFormat::T indexFormat)
	{
		CrCommandBufferAssertMsg(indexBuffer != nullptr, "Buffer is null");
		CrCommandBufferAssertMsg(indexBuffer->GetUsage() & crgfx::BufferUsage::Index, "Buffer must have index buffer flag");
		CrCommandBufferAssertMsg(indexFormat == crgfx::DataFormat::R16_Uint || indexFormat == crgfx::DataFormat::R32_Uint, "Only these formats are allowed");

		if (m_currentState.m_indexBuffer != indexBuffer ||
			m_currentState.m_indexBufferOffset != byteOffset ||
			m_currentState.m_indexBufferSize != sizeBytes ||
			m_currentState.m_indexBufferFormat != indexFormat)
		{
			m_currentState.m_indexBuffer = indexBuffer;
			m_currentState.m_indexBufferOffset = byteOffset;
			m_currentState.m_indexBufferSize = sizeBytes;
			m_currentState.m_indexBufferFormat = indexFormat;
			m_currentState.m_indexBufferDirty = true;
		}
	}

	inline void ICommandBuffer::BindIndexBuffer(const CrGPUBufferView& indexBufferView)
	{
		BindIndexBuffer(indexBufferView.GetHardwareBuffer(), indexBufferView.GetByteOffset(), indexBufferView.GetSizeBytes(), indexBufferView.GetFormat());
	}

	inline void ICommandBuffer::BindIndexBuffer(const IndexBuffer* indexBuffer, uint32_t elementOffset)
	{
		BindIndexBuffer(indexBuffer->GetHardwareBuffer(), elementOffset * indexBuffer->GetStride(), indexBuffer->GetNumElements() * indexBuffer->GetStride(), indexBuffer->GetFormat());
	}

	inline void ICommandBuffer::BindVertexBuffer(const IHardwareGPUBuffer* vertexBuffer, uint32_t streamId, uint32_t byteOffset, uint32_t vertexCount, uint32_t stride)
	{
		CrCommandBufferAssertMsg(vertexBuffer != nullptr, "Buffer is null");
		CrCommandBufferAssertMsg(vertexBuffer->GetUsage() & crgfx::BufferUsage::Vertex, "Buffer must have vertex buffer flag");
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

	inline void ICommandBuffer::BindVertexBuffer(const CrGPUBufferView& vertexBufferView, uint32_t streamId)
	{
		BindVertexBuffer(vertexBufferView.GetHardwareBuffer(), streamId, vertexBufferView.GetByteOffset(), vertexBufferView.GetNumElements(), vertexBufferView.GetStride());
	}

	inline void ICommandBuffer::BindVertexBuffer(const VertexBuffer* vertexBuffer, uint32_t streamId, uint32_t elementOffset)
	{
		BindVertexBuffer(vertexBuffer->GetHardwareBuffer(), streamId, elementOffset * vertexBuffer->GetStride(), vertexBuffer->GetNumElements(), vertexBuffer->GetStride());
	}

	inline void ICommandBuffer::BindGraphicsPipelineState(const IGraphicsPipeline* graphicsPipeline)
	{
		if (m_currentState.m_graphicsPipeline != graphicsPipeline)
		{
			m_currentState.m_graphicsPipeline = graphicsPipeline;
			m_currentState.m_graphicsPipelineDirty = true;
		}
	}

	inline void ICommandBuffer::BindComputePipelineState(const IComputePipeline* computePipeline)
	{
		if (m_currentState.m_computePipeline != computePipeline)
		{
			m_currentState.m_computePipeline = computePipeline;
			m_currentState.m_computePipelineDirty = true;
		}
	}

	inline void ICommandBuffer::ClearRenderTarget(const crgfx::ITexture* renderTarget, const float4& color, uint32_t mip, uint32_t slice, uint32_t mipCount, uint32_t sliceCount)
	{
		ClearRenderTargetPS(renderTarget, color, mip, slice, mipCount, sliceCount);
	}

	inline void ICommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
	{
		CrCommandBufferAssertMsg(m_currentState.m_currentRenderPass.type == crgfx::RenderPassType::Graphics, "Render pass type must be Graphics");

		FlushGraphicsRenderState();

		DrawPS(vertexCount, instanceCount, firstVertex, firstInstance);

		CrRenderingStatistics::AddDrawcall();
		CrRenderingStatistics::AddVertices(vertexCount);
		CrRenderingStatistics::AddInstances(instanceCount);
	}

	inline void ICommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
	{
		CrCommandBufferAssertMsg(m_currentState.m_currentRenderPass.type == crgfx::RenderPassType::Graphics, "Render pass type must be Graphics");

		FlushGraphicsRenderState();

		DrawIndexedPS(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);

		CrRenderingStatistics::AddDrawcall();
		CrRenderingStatistics::AddVertices(indexCount * instanceCount);
		CrRenderingStatistics::AddInstances(instanceCount);
	}

	inline void ICommandBuffer::DrawIndirect(const IHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count)
	{
		CrCommandBufferAssertMsg(m_currentState.m_currentRenderPass.type == crgfx::RenderPassType::Graphics, "Render pass type must be Graphics");

		FlushGraphicsRenderState();

		DrawIndirectPS(indirectBuffer, offset, count);

		CrRenderingStatistics::AddDrawcall();
		// We cannot add the vertices here because we don't know them
	}

	inline void ICommandBuffer::DispatchTexture1D(uint32_t textureWidth)
	{
		uint dispatchThreadX = m_currentState.m_computePipeline->GetGroupSizeX();

		Dispatch((textureWidth + dispatchThreadX - 1) / dispatchThreadX, 1, 1);
	}

	inline void ICommandBuffer::DispatchTexture2D(uint32_t textureWidth, uint32_t textureHeight)
	{
		uint dispatchThreadX = m_currentState.m_computePipeline->GetGroupSizeX();
		uint dispatchThreadY = m_currentState.m_computePipeline->GetGroupSizeY();

		Dispatch((textureWidth + dispatchThreadX - 1) / dispatchThreadX, (textureHeight + dispatchThreadY - 1) / dispatchThreadY, 1);
	}

	inline void ICommandBuffer::DispatchTexture3D(uint32_t textureWidth, uint32_t textureHeight, uint32_t textureDepth)
	{
		uint dispatchThreadX = m_currentState.m_computePipeline->GetGroupSizeX();
		uint dispatchThreadY = m_currentState.m_computePipeline->GetGroupSizeY();
		uint dispatchThreadZ = m_currentState.m_computePipeline->GetGroupSizeZ();

		Dispatch((textureWidth + dispatchThreadX - 1) / dispatchThreadX, (textureHeight + dispatchThreadY - 1) / dispatchThreadY, (textureDepth + dispatchThreadZ - 1) / dispatchThreadZ);
	}

	inline void ICommandBuffer::DrawIndexedIndirect(const IHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count)
	{
		FlushGraphicsRenderState();

		DrawIndexedIndirectPS(indirectBuffer, offset, count);

		CrRenderingStatistics::AddDrawcall();
		// We cannot add the vertices here because we don't know them
	}

	inline void ICommandBuffer::DispatchIndirect(const IHardwareGPUBuffer* indirectBuffer, uint32_t offset)
	{
		CrCommandBufferAssertMsg(m_currentState.m_currentRenderPass.type == crgfx::RenderPassType::Compute, "Render pass type must be Compute");

		FlushComputeRenderState();

		DispatchIndirectPS(indirectBuffer, offset);
	}

	inline void ICommandBuffer::FlushGraphicsRenderState()
	{
		FlushGraphicsRenderStatePS();
	}

	inline void ICommandBuffer::Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
	{
		CrCommandBufferAssertMsg(m_currentState.m_currentRenderPass.type == crgfx::RenderPassType::Compute, "Render pass type must be Compute");

		FlushComputeRenderState();

		DispatchPS(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
	}

	inline void ICommandBuffer::FlushComputeRenderState()
	{
		FlushComputeRenderStatePS();
	}

	inline void ICommandBuffer::BeginDebugEvent(const char* eventName, const float4& color)
	{
		BeginDebugEventPS(eventName, color);
	}

	inline void ICommandBuffer::EndDebugEvent()
	{
		EndDebugEventPS();
	}

	inline void ICommandBuffer::InsertDebugMarker(const char* markerName, const float4& color)
	{
		InsertDebugMarkerPS(markerName, color);
	}

	inline void ICommandBuffer::BindConstantBuffer(ConstantBuffers::T constantBufferIndex, const IHardwareGPUBuffer* constantBuffer, uint32_t size, uint32_t offset)
	{
		CrCommandBufferAssertMsg(constantBuffer != nullptr, "Buffer is null");
		CrCommandBufferAssertMsg(constantBuffer->HasUsage(crgfx::BufferUsage::Constant), "Buffer must have constant buffer flag");
		CrCommandBufferAssertMsg(constantBufferIndex < ConstantBuffers::Count, "Invalid binding index");

		m_currentState.m_constantBuffers[constantBufferIndex] = ConstantBufferBinding(constantBuffer, size, offset);
	}

	inline void ICommandBuffer::BindConstantBuffer(const CrGPUBufferView& constantBufferView)
	{
		BindConstantBuffer((ConstantBuffers::T)constantBufferView.GetBindingIndex(), constantBufferView.GetHardwareBuffer(), constantBufferView.GetSizeBytes(), constantBufferView.GetByteOffset());
	}

	inline void ICommandBuffer::BindSampler(Samplers::T samplerIndex, const crgfx::ISampler* sampler)
	{
		CrCommandBufferAssertMsg(sampler != nullptr, "Sampler is null");
		CrCommandBufferAssertMsg(samplerIndex < Samplers::Count, "Invalid binding index");

		m_currentState.m_samplers[samplerIndex] = sampler;
	}

	inline void ICommandBuffer::BindTexture(Textures::T textureIndex, const crgfx::ITexture* texture, crgfx::TextureView view)
	{
		CrCommandBufferAssertMsg(texture != nullptr, "Texture is null");
		CrCommandBufferAssertMsg(textureIndex < Textures::Count, "Invalid binding index");
		CrCommandBufferAssertMsg((view.plane == crgfx::TexturePlane::Plane0) ? true : crgfx::IsDepthStencilFormat(texture->GetFormat()), "Invalid plane specified");

		m_currentState.m_textures[textureIndex] = TextureBinding(texture, view);
	}

	inline void ICommandBuffer::BindRWTexture(RWTextures::T rwTextureIndex, const crgfx::ITexture* texture, uint32_t mip)
	{
		CrCommandBufferAssertMsg(texture != nullptr, "Texture is null");
		CrCommandBufferAssertMsg(texture->IsUnorderedAccess(), "Texture must be created with UnorderedAccess flag");
		CrCommandBufferAssertMsg(mip < texture->GetMipmapCount(), "Texture doesn't have enough mipmaps!");
		CrCommandBufferAssertMsg(rwTextureIndex < RWTextures::Count, "Invalid binding index");

		m_currentState.m_rwTextures[rwTextureIndex] = RWTextureBinding(texture, mip);
	}

	inline void ICommandBuffer::BindStorageBuffer(StorageBuffers::T storageBufferIndex, const IHardwareGPUBuffer* buffer, uint32_t numElements, uint32_t stride, uint32_t offset)
	{
		CrCommandBufferAssertMsg(buffer != nullptr, "Buffer is null");
		CrCommandBufferAssertMsg(buffer->HasUsage(crgfx::BufferUsage::Storage), "Buffer must have storage buffer flag");
		CrCommandBufferAssertMsg(storageBufferIndex < StorageBuffers::Count, "Invalid binding index");
		CrCommandBufferAssertMsg(numElements * stride <= buffer->GetSizeBytes(), "Bound size too large");

		m_currentState.m_storageBuffers[storageBufferIndex] = StorageBufferBinding(buffer, numElements, stride, offset);
	}

	inline void ICommandBuffer::BindStorageBuffer(StorageBuffers::T storageBufferIndex, const IHardwareGPUBuffer* buffer)
	{
		BindStorageBuffer(storageBufferIndex, buffer, buffer->GetNumElements(), buffer->GetStrideBytes(), 0);
	}

	inline void ICommandBuffer::BindStorageBuffer(const CrGPUBufferView& structuredBufferView)
	{
		BindStorageBuffer((StorageBuffers::T)structuredBufferView.GetBindingIndex(), structuredBufferView.GetHardwareBuffer(), structuredBufferView.GetNumElements(), structuredBufferView.GetStride(), structuredBufferView.GetByteOffset());
	}

	inline void ICommandBuffer::BindRWStorageBuffer(RWStorageBuffers::T rwStorageBufferIndex, const IHardwareGPUBuffer* buffer, uint32_t numElements, uint32_t stride, uint32_t offset)
	{
		CrCommandBufferAssertMsg(buffer != nullptr, "Buffer is null");
		CrCommandBufferAssertMsg(buffer->HasUsage(crgfx::BufferUsage::Storage), "Buffer must have storage buffer flag");
		CrCommandBufferAssertMsg(buffer->HasAccess(crgfx::MemoryAccess::GPUOnlyWrite) || buffer->HasAccess(crgfx::MemoryAccess::GPUWriteCPURead), "Buffer must be GPU-writable");
		CrCommandBufferAssertMsg(rwStorageBufferIndex < RWStorageBuffers::Count, "Invalid binding index");
		CrCommandBufferAssertMsg(numElements * stride <= buffer->GetSizeBytes(), "Bound size too large");

		m_currentState.m_rwStorageBuffers[rwStorageBufferIndex] = StorageBufferBinding(buffer, numElements, stride, offset);
	}

	inline void ICommandBuffer::BindRWStorageBuffer(RWStorageBuffers::T rwStorageBufferIndex, const IHardwareGPUBuffer* buffer)
	{
		BindRWStorageBuffer(rwStorageBufferIndex, buffer, buffer->GetNumElements(), buffer->GetStrideBytes(), 0);
	}

	inline void ICommandBuffer::BindRWTypedBuffer(RWTypedBuffers::T rwBufferIndex, const IHardwareGPUBuffer* buffer, uint32_t numElements, uint32_t stride, uint32_t offset)
	{
		unused_parameter(numElements); unused_parameter(stride); unused_parameter(offset);

		CrCommandBufferAssertMsg(buffer != nullptr, "Buffer is null");
		CrCommandBufferAssertMsg(buffer->HasUsage(crgfx::BufferUsage::Typed), "Buffer must have typed buffer flag");
		CrCommandBufferAssertMsg(buffer->HasAccess(crgfx::MemoryAccess::GPUOnlyWrite) || buffer->HasAccess(crgfx::MemoryAccess::GPUWriteCPURead), "Buffer must be GPU-writable");
		CrCommandBufferAssertMsg(rwBufferIndex < RWTypedBuffers::Count, "Invalid binding index");
		CrCommandBufferAssertMsg(numElements * stride <= buffer->GetSizeBytes(), "Bound size too large");

		m_currentState.m_rwTypedBuffers[rwBufferIndex].buffer = buffer;
	}

	inline void ICommandBuffer::BindRWTypedBuffer(RWTypedBuffers::T rwTypedBufferIndex, const IHardwareGPUBuffer* buffer)
	{
		BindRWTypedBuffer(rwTypedBufferIndex, buffer, buffer->GetNumElements(), buffer->GetStrideBytes(), 0);
	}

	template<typename MetaType>
	inline CrGPUBufferViewT<MetaType> ICommandBuffer::AllocateConstantBuffer(uint32_t instanceCount)
	{
		CrStackAllocation<void> allocation = m_bufferGPUStack->AllocateAligned(instanceCount * sizeof(MetaType), 256);

		CrGPUBufferViewT<MetaType> constantBufferView
		(
			m_bufferGPUStack->GetHardwareGPUBuffer(),
			instanceCount,
			sizeof(MetaType),
			allocation.offset,
			allocation.memory
		);

		return constantBufferView;
	}

	template<typename MetaType>
	inline CrGPUBufferViewT<MetaType> ICommandBuffer::AllocateConstantBuffer(uint32_t instanceCount, uint32_t instanceSizeBytes)
	{
		CrAssertMsg(((sizeof(MetaType) / instanceSizeBytes) * instanceSizeBytes) == sizeof(MetaType), "Instance size must be a multiple of the size of MetaType");

		CrStackAllocation<void> allocation = m_bufferGPUStack->AllocateAligned(instanceCount * instanceSizeBytes, 256);

		CrGPUBufferViewT<MetaType> constantBufferView
		(
			m_bufferGPUStack->GetHardwareGPUBuffer(),
			instanceCount,
			instanceSizeBytes,
			allocation.offset,
			allocation.memory
		);

		return constantBufferView;
	}

	template<typename MetaType>
	inline CrGPUBufferViewT<MetaType> ICommandBuffer::AllocateConstantBuffer()
	{
		return AllocateConstantBuffer<MetaType>(1, sizeof(MetaType));
	}

	template<typename MetaType>
	inline CrGPUBufferViewT<MetaType> ICommandBuffer::AllocateStorageBuffer(uint32_t instanceCount)
	{
		CrStackAllocation<void> allocation = m_bufferGPUStack->AllocateAligned(instanceCount * sizeof(MetaType), 256);

		CrGPUBufferViewT<MetaType> structuredBufferView
		(
			m_bufferGPUStack->GetHardwareGPUBuffer(),
			instanceCount,
			sizeof(MetaType),
			allocation.offset,
			allocation.memory
		);

		return structuredBufferView;
	}
};