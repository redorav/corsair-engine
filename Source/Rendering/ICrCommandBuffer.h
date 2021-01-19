#pragma once

#include "ICrPipelineStateManager.h"

#include "CrGPUBuffer.h"

#include "ShaderResources.h"

class CrPipelineStateManagerVulkan;
class ICrGPUSemaphore;
class ICrGPUFence;
class CrIndexBufferCommon;
class CrVertexBufferCommon;

class ICrTexture;
using CrTextureSharedHandle = CrSharedPtr<ICrTexture>;

class ICrSampler;
using CrSamplerSharedHandle = CrSharedPtr<ICrSampler>;

struct CrGraphicsPipelineDescriptor;
class ICrGraphicsPipeline;
class CrComputePipeline;
class CrConstantBufferBase;
class ICrHardwareGPUBuffer;
class ICrRenderPass;
class ICrFramebuffer;
struct CrRenderPassBeginParams;
class CrGPUStackAllocator;
class CrCPUStackAllocator;

struct CrViewport;

class ICrCommandBuffer
{
public:

	ICrCommandBuffer(ICrCommandQueue* commandQueue);

	virtual ~ICrCommandBuffer();

	void Begin();

	void End();

	void Submit(const ICrGPUSemaphore* waitSemaphore = nullptr, const ICrGPUSemaphore* signalSemaphore = nullptr, const ICrGPUFence* signalFence = nullptr);

	void SetViewport(const CrViewport& viewport);

	void SetScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

	void BindIndexBuffer(const CrIndexBufferCommon* indexBuffer);
	
	void BindVertexBuffer(const CrVertexBufferCommon* vertexBuffer, uint32_t bindPoint);

	void BindGraphicsPipeline(const CrGraphicsPipelineDescriptor& pipelineDescriptor);

	void BindGraphicsPipelineState(const ICrGraphicsPipeline* pipelineState);

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

	void BindConstantBuffer(const CrGPUBuffer* constantBuffer);

	void BindConstantBuffer(const CrGPUBuffer* constantBuffer, int32_t globalIndex);

	void BindTexture(cr3d::ShaderStage::T shaderStage, const Textures::T textureIndex, const ICrTexture* texture);

	void BindSampler(cr3d::ShaderStage::T shaderStage, const Samplers::T samplerIndex, const ICrSampler* sampler);

	void BeginRenderPass(const ICrRenderPass* renderPass, const ICrFramebuffer* frameBuffer, const CrRenderPassBeginParams& renderPassParams);

	void EndRenderPass(const ICrRenderPass* renderPass);

	const ICrCommandQueue* GetCommandQueue() const;

protected:

	virtual void BeginPS() = 0;

	virtual void EndPS() = 0;

	virtual void SetViewportPS(const CrViewport& viewport) = 0;

	virtual void SetScissorPS(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

	virtual void BindIndexBufferPS(const ICrHardwareGPUBuffer* indexBuffer) = 0;

	virtual void BindVertexBuffersPS(const ICrHardwareGPUBuffer* vertexBuffer, uint32_t bindPoint) = 0;

	virtual void BindGraphicsPipelineStatePS(const ICrGraphicsPipeline* pipelineState) = 0;

	virtual void ClearRenderTargetPS(const ICrTexture* renderTarget, const float4& color, uint32_t level, uint32_t slice, uint32_t levelCount, uint32_t sliceCount) = 0;

	virtual void DrawPS(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;

	virtual void DrawIndexedPS(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) = 0;

	virtual void DispatchPS(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) = 0;

	virtual void BeginDebugEventPS(const char* eventName, const float4& color) = 0;

	virtual void EndDebugEventPS() = 0;

	virtual void TransitionTexturePS(const ICrTexture* texture, cr3d::ResourceState::T initialState, cr3d::ResourceState::T destinationState) = 0;

	virtual void BeginRenderPassPS(const ICrRenderPass* renderPass, const ICrFramebuffer* frameBuffer, const CrRenderPassBeginParams& renderPassParams) = 0;

	virtual void EndRenderPassPS(const ICrRenderPass* renderPass) = 0;

	virtual void UpdateResourceTablesPS() = 0;

	CrGPUBufferDescriptor AllocateConstantBufferParameters(uint32_t size);

	// TODO Do all platforms support binding a buffer and an offset inside?
	struct ConstantBufferBinding
	{
		const ICrHardwareGPUBuffer* buffer = nullptr;
		uint32_t byteOffset = 0;
	};

	struct CurrentState
	{
		const CrIndexBufferCommon*		m_indexBuffer;
		const CrVertexBufferCommon*		m_vertexBuffer;

		CrGraphicsPipelineDescriptor	m_graphicsPipelineDescriptor;
		const ICrGraphicsPipeline*		m_graphicsPipeline;
		const CrComputePipeline*		m_computePipeline;

		ConstantBufferBinding			m_constantBuffers[cr3d::ShaderStage::Count][ConstantBuffers::Count];

		const ICrTexture*				m_textures[cr3d::ShaderStage::Count][Textures::Count];
		const ICrSampler*				m_samplers[cr3d::ShaderStage::Count][Samplers::Count];

		// Structured Buffers
	};

	CurrentState					m_currentState = {};

	ICrRenderDevice*				m_renderDevice = nullptr;

	ICrCommandQueue*				m_ownerCommandQueue = nullptr;

	CrUniquePtr<CrGPUStackAllocator> m_constantBufferGPUStack;
};

inline void ICrCommandBuffer::SetViewport(const CrViewport& viewport)
{
	SetViewportPS(viewport);
}

inline void ICrCommandBuffer::SetScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	SetScissorPS(x, y, width, height);
}

inline void ICrCommandBuffer::BindIndexBuffer(const CrIndexBufferCommon* indexBuffer)
{
	m_currentState.m_indexBuffer = indexBuffer;

	// TODO Move to flush
	BindIndexBufferPS(indexBuffer->GetHardwareBuffer());
}

inline void ICrCommandBuffer::BindVertexBuffer(const CrVertexBufferCommon* vertexBuffer, uint32_t bindPoint)
{
	m_currentState.m_vertexBuffer = vertexBuffer;

	// TODO Move to flush
	BindVertexBuffersPS(vertexBuffer->GetHardwareBuffer(), bindPoint);
}

inline void ICrCommandBuffer::BindGraphicsPipeline(const CrGraphicsPipelineDescriptor& pipelineDescriptor)
{
	m_currentState.m_graphicsPipelineDescriptor = pipelineDescriptor;
}

inline void ICrCommandBuffer::BindGraphicsPipelineState(const ICrGraphicsPipeline* pipelineState)
{
	m_currentState.m_graphicsPipeline = pipelineState;

	// TODO Move to flush
	BindGraphicsPipelineStatePS(pipelineState);
}

inline void ICrCommandBuffer::ClearRenderTarget(const ICrTexture* renderTarget, const float4& color, uint32_t level, uint32_t slice, uint32_t levelCount, uint32_t sliceCount)
{
	ClearRenderTargetPS(renderTarget, color, level, slice, levelCount, sliceCount);
}

inline void ICrCommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	//UpdateGraphicsPipelineState();

	UpdateResourceTablesPS();

	DrawPS(vertexCount, instanceCount, firstVertex, firstInstance);
}

inline void ICrCommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	//UpdateGraphicsPipelineState();

	UpdateResourceTablesPS();

	DrawIndexedPS(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

inline void ICrCommandBuffer::Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
{
	//UpdateComputePipelineState();

	UpdateResourceTablesPS();

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

inline void ICrCommandBuffer::BindTexture(cr3d::ShaderStage::T shaderStage, const Textures::T textureIndex, const ICrTexture* texture)
{
	m_currentState.m_textures[shaderStage][textureIndex] = texture;
}

inline void ICrCommandBuffer::BindSampler(cr3d::ShaderStage::T shaderStage, const Samplers::T samplerIndex, const ICrSampler* sampler)
{
	m_currentState.m_samplers[shaderStage][samplerIndex] = sampler;
}

inline void ICrCommandBuffer::BeginRenderPass(const ICrRenderPass* renderPass, const ICrFramebuffer* frameBuffer, const CrRenderPassBeginParams& renderPassParams)
{
	BeginRenderPassPS(renderPass, frameBuffer, renderPassParams);
}

inline void ICrCommandBuffer::EndRenderPass(const ICrRenderPass* renderPass)
{
	EndRenderPassPS(renderPass);
}

inline const ICrCommandQueue* ICrCommandBuffer::GetCommandQueue() const
{
	return m_ownerCommandQueue;
}

template<typename MetaType>
CrGPUBufferType<MetaType> ICrCommandBuffer::AllocateConstantBuffer()
{
	return CrGPUBufferType<MetaType>(m_renderDevice, AllocateConstantBufferParameters(sizeof(MetaType)));
}