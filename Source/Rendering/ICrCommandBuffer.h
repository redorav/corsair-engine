#pragma once

#include "Rendering/ICrPipeline.h"

#include "Rendering/CrGPUBuffer.h"

#include "Rendering/ICrTexture.h"

#include "GeneratedShaders/ShaderMetadata.h"

#include "CrRenderingForwardDeclarations.h"

class ICrCommandBuffer
{
public:

	ICrCommandBuffer(ICrCommandQueue* commandQueue);

	virtual ~ICrCommandBuffer();

	void Begin();

	void End();

	void Submit(const ICrGPUSemaphore* waitSemaphore = nullptr, const ICrGPUSemaphore* signalSemaphore = nullptr, const ICrGPUFence* signalFence = nullptr);

	void SetViewport(const CrViewport& viewport);

	void SetScissor(const CrScissor& scissor);

	// Binding functions

	void BindIndexBuffer(const CrIndexBufferCommon* indexBuffer);
	
	void BindVertexBuffer(const CrVertexBufferCommon* vertexBuffer, uint32_t bindPoint);

	void BindGraphicsPipelineState(const ICrGraphicsPipeline* graphicsPipeline);

	void BindComputePipelineState(const ICrComputePipeline* computePipeline);

	void BindConstantBuffer(const CrGPUBuffer* constantBuffer);

	void BindConstantBuffer(const CrGPUBuffer* constantBuffer, int32_t globalIndex);

	void BindSampler(cr3d::ShaderStage::T shaderStage, const Samplers::T samplerIndex, const ICrSampler* sampler);

	void BindTexture(cr3d::ShaderStage::T shaderStage, const Textures::T textureIndex, const ICrTexture* texture);

	void BindRWTexture(cr3d::ShaderStage::T shaderStage, const RWTextures::T rwTextureIndex, const ICrTexture* texture, uint32_t mip);

	void BindRWStorageBuffer(cr3d::ShaderStage::T shaderStage, const RWStorageBuffers::T rwStorageBufferIndex, const CrGPUBuffer* buffer);

	void BindRWDataBuffer(cr3d::ShaderStage::T shaderStage, const RWDataBuffers::T rwDataBufferIndex, const CrGPUBuffer* buffer);

	// Command functions

	void ClearRenderTarget(const ICrTexture* renderTarget, const float4& color, uint32_t level, uint32_t slice, uint32_t levelCount, uint32_t sliceCount);

	void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

	void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);

	void Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ);

	void BeginDebugEvent(const char* eventName, const float4& color);

	void EndDebugEvent();

	void TransitionTexture(const ICrTexture* texture, cr3d::ResourceState::T initialState, cr3d::ResourceState::T destinationState);

	template<typename MetaType>
	CrGPUBufferType<MetaType> AllocateConstantBuffer();

	CrGPUBuffer AllocateConstantBuffer(uint32_t size);

	void BeginRenderPass(const ICrRenderPass* renderPass, const ICrFramebuffer* frameBuffer, const CrRenderPassBeginParams& renderPassParams);

	void EndRenderPass();

	const ICrCommandQueue* GetCommandQueue() const;

protected:

	virtual void BeginPS() = 0;

	virtual void EndPS() = 0;

	virtual void BindGraphicsPipelineStatePS(const ICrGraphicsPipeline* graphicsPipeline) = 0;

	virtual void BindComputePipelineStatePS(const ICrComputePipeline* computePipeline) = 0;

	virtual void ClearRenderTargetPS(const ICrTexture* renderTarget, const float4& color, uint32_t level, uint32_t slice, uint32_t levelCount, uint32_t sliceCount) = 0;

	virtual void DrawPS(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;

	virtual void DrawIndexedPS(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) = 0;

	virtual void DispatchPS(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) = 0;

	virtual void BeginDebugEventPS(const char* eventName, const float4& color) = 0;

	virtual void EndDebugEventPS() = 0;

	virtual void TransitionTexturePS(const ICrTexture* texture, cr3d::ResourceState::T initialState, cr3d::ResourceState::T destinationState) = 0;

	virtual void BeginRenderPassPS(const ICrRenderPass* renderPass, const ICrFramebuffer* frameBuffer, const CrRenderPassBeginParams& renderPassParams) = 0;

	virtual void EndRenderPassPS() = 0;

	virtual void FlushGraphicsRenderStatePS() = 0;

	virtual void FlushComputeRenderStatePS() = 0;

	CrGPUBufferDescriptor AllocateConstantBufferParameters(uint32_t size);

	// TODO Do all platforms support binding a buffer and an offset inside?
	struct ConstantBufferBinding
	{
		ConstantBufferBinding() = default;

		ConstantBufferBinding(const ICrHardwareGPUBuffer* buffer, uint32_t byteOffset) : buffer(buffer), byteOffset(byteOffset) {}

		const ICrHardwareGPUBuffer* buffer = nullptr;
		uint32_t byteOffset = 0;
	};

	struct RWStorageBufferBinding
	{
		RWStorageBufferBinding() = default;

		RWStorageBufferBinding(const ICrHardwareGPUBuffer* buffer, uint32_t size, uint32_t byteOffset) : buffer(buffer), size(size), byteOffset(byteOffset) {}

		const ICrHardwareGPUBuffer* buffer = nullptr;
		uint32_t byteOffset = 0;
		uint32_t size = 0;
	};

	struct RWTextureBinding
	{
		RWTextureBinding() = default;

		RWTextureBinding(const ICrTexture* texture, uint32_t mip) : texture(texture), mip(mip) {}

		const ICrTexture* texture = nullptr;
		uint32_t mip = 0;
	};

	// TODO Have inline accessors here instead. We need to be able to tell if we're missing
	// a resource and even bind a dummy one if we have a safe mode

	struct CurrentState
	{
		const ConstantBufferBinding& GetConstantBufferBinding(cr3d::ShaderStage::T stage, ConstantBuffers::T id)
		{
			return m_constantBuffers[stage][id];
		}

		const CrIndexBufferCommon*      m_indexBuffer;
		bool                            m_indexBufferDirty = true;

		const CrVertexBufferCommon*     m_vertexBuffer;
		bool                            m_vertexBufferDirty = true;

		CrScissor                       m_scissor;
		bool                            m_scissorDirty = true;

		CrViewport                      m_viewport;
		bool                            m_viewportDirty = true;

		CrGraphicsPipelineDescriptor    m_graphicsPipelineDescriptor;
		const ICrGraphicsPipeline*      m_graphicsPipeline;
		const ICrComputePipeline*       m_computePipeline;

		ConstantBufferBinding			m_constantBuffers[cr3d::ShaderStage::Count][ConstantBuffers::Count];

		const ICrSampler*				m_samplers[cr3d::ShaderStage::Count][Samplers::Count];

		const ICrTexture*				m_textures[cr3d::ShaderStage::Count][Textures::Count];

		RWTextureBinding				m_rwTextures[cr3d::ShaderStage::Count][RWTextures::Count];

		RWStorageBufferBinding			m_rwStorageBuffers[cr3d::ShaderStage::Count][RWStorageBuffers::Count];

		const CrGPUBuffer*				m_rwDataBuffers[cr3d::ShaderStage::Count][RWDataBuffers::Count];
	};

	CurrentState					m_currentState = {};

	ICrRenderDevice*				m_renderDevice = nullptr;

	ICrCommandQueue*				m_ownerCommandQueue = nullptr;

	CrUniquePtr<CrGPUStackAllocator> m_constantBufferGPUStack;
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

inline void ICrCommandBuffer::BindIndexBuffer(const CrIndexBufferCommon* indexBuffer)
{
	if (m_currentState.m_indexBuffer != indexBuffer)
	{
		m_currentState.m_indexBuffer = indexBuffer;
		m_currentState.m_indexBufferDirty = true;
	}
}

inline void ICrCommandBuffer::BindVertexBuffer(const CrVertexBufferCommon* vertexBuffer, uint32_t /*bindPoint*/)
{
	if (m_currentState.m_vertexBuffer != vertexBuffer)
	{
		m_currentState.m_vertexBuffer = vertexBuffer;
		m_currentState.m_vertexBufferDirty = true;
	}
}

inline void ICrCommandBuffer::BindGraphicsPipelineState(const ICrGraphicsPipeline* graphicsPipeline)
{
	m_currentState.m_graphicsPipeline = graphicsPipeline;

	// TODO Move to flush
	BindGraphicsPipelineStatePS(graphicsPipeline);
}

inline void ICrCommandBuffer::BindComputePipelineState(const ICrComputePipeline* computePipeline)
{
	m_currentState.m_computePipeline = computePipeline;

	// TODO Move to flush
	BindComputePipelineStatePS(computePipeline);
}

inline void ICrCommandBuffer::ClearRenderTarget(const ICrTexture* renderTarget, const float4& color, uint32_t level, uint32_t slice, uint32_t levelCount, uint32_t sliceCount)
{
	ClearRenderTargetPS(renderTarget, color, level, slice, levelCount, sliceCount);
}

inline void ICrCommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	FlushGraphicsRenderStatePS(); // TODO Put in platform-independent code

	DrawPS(vertexCount, instanceCount, firstVertex, firstInstance);
}

inline void ICrCommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	FlushGraphicsRenderStatePS(); // TODO Put in platform-independent code

	DrawIndexedPS(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

inline void ICrCommandBuffer::Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
{
	FlushComputeRenderStatePS(); // TODO Put in platform-independent code

	DispatchPS(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

inline void ICrCommandBuffer::BeginDebugEvent(const char* eventName, const float4& color)
{
	BeginDebugEventPS(eventName, color);
}

inline void ICrCommandBuffer::EndDebugEvent()
{
	EndDebugEventPS();
}

inline void ICrCommandBuffer::TransitionTexture(const ICrTexture* texture, cr3d::ResourceState::T initialState, cr3d::ResourceState::T destinationState)
{
	TransitionTexturePS(texture, initialState, destinationState);
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

inline void ICrCommandBuffer::BindRWStorageBuffer(cr3d::ShaderStage::T shaderStage, const RWStorageBuffers::T rwStorageBufferIndex, const CrGPUBuffer* buffer)
{
	m_currentState.m_rwStorageBuffers[shaderStage][rwStorageBufferIndex] = RWStorageBufferBinding(buffer->GetHardwareBuffer(), buffer->GetSize(), buffer->GetByteOffset());
}

inline void ICrCommandBuffer::BindRWDataBuffer(cr3d::ShaderStage::T shaderStage, const RWDataBuffers::T rwBufferIndex, const CrGPUBuffer* buffer)
{
	m_currentState.m_rwDataBuffers[shaderStage][rwBufferIndex] = buffer;
}

inline void ICrCommandBuffer::BeginRenderPass(const ICrRenderPass* renderPass, const ICrFramebuffer* frameBuffer, const CrRenderPassBeginParams& renderPassParams)
{
	BeginRenderPassPS(renderPass, frameBuffer, renderPassParams);
}

inline void ICrCommandBuffer::EndRenderPass()
{
	EndRenderPassPS();
}

inline const ICrCommandQueue* ICrCommandBuffer::GetCommandQueue() const
{
	return m_ownerCommandQueue;
}

template<typename MetaType>
inline CrGPUBufferType<MetaType> ICrCommandBuffer::AllocateConstantBuffer()
{
	return CrGPUBufferType<MetaType>(m_renderDevice, AllocateConstantBufferParameters(sizeof(MetaType)), 1);
}