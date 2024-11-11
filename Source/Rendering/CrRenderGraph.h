#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrRendering.h"

#include "Core/Function/CrFixedFunction.h"
#include "Core/Containers/CrFixedHashMap.h"
#include "Core/Containers/CrFixedVector.h"
#include "Core/String/CrFixedString.h"
#include "Core/CrHash.h"

#include "Math/CrHlslppVectorFloatType.h"

//#define RENDER_GRAPH_LOGS

#if defined(RENDER_GRAPH_LOGS)
#define CrRenderGraphLog2(format, ...) CrLog(format, __VA_ARGS__)
#else
#define CrRenderGraphLog2(format, ...)
#endif

// Objectives
// 
// There are certain features we are looking for in a render graph. Here is a non-exhaustive list
// of what the render graph wants to achieve
//
// 1) Deduce source and destination transitions for each subresource in a pass automatically
// 2) Dispatch portions of the render graph to different queues and command buffers, in different threads
// 3) Reserve render target or buffer memory automatically if needed
// 4) Express split barriers
// 5) Express memory dependencies
// 6) Express temporal or history buffers
// 7) Inject passes between existing passes, and remove passes
// 8) Handle async compute
// 9) Reuse code (e.g. blur) without having name clashes
// 10) Multiple render passes inside a rendering (e.g. Renderdoc) scope

// API and Usage
//
// 1) Add a render pass by doing renderGraph.AddPass
// 1.1) A pass has name, a type, a debug color and two functions: setup and execution
//
// 2) Resources are bound to the render graph with a similar API to the command buffer, this is tracked by the render graph
// 2.1) For regular resources, these already come with their memory and views premade
// 2.2) For transient resources, the resource itself will know whether it manages its own memory or not./ The resource does
// not assume what that is, it merely provides an API for an external system to do it
// 2.3) Subresources internally have unique ids so that we can track the transitions and barriers appropriately
// 
// 3) Execution takes in the render graph and a command buffer, where actual commands are executed
// 3.1) All passes are executed sequentially once their order has been determined. One has to think of the render graph as an
// independent execution timeline from the one where the lambda was created

class CrRenderGraph;
struct CrRenderGraphPass2;

using CrRenderGraphSetupFunction = CrFixedFunction<32, void(CrRenderGraph& renderGraph)>;
using CrRenderGraphExecutionFunction = CrFixedFunction<32, void(const CrRenderGraph& renderGraph, ICrCommandBuffer*)>;
using CrRenderGraphString = CrFixedString32;

namespace CrRenderGraphPassType
{
	enum T
	{
		Graphics, // A graphics pass only allows draws to happen within. Render targets are set up automatically. UAV access dependencies inside a pass must be handled manually
		Compute, // A compute pass only allows dispatches to happen within. UAV access dependencies inside a pass must be handled manually
		Behavior, // A Behavior render pass is used for code that needs to run in the render graph timeline, but doesn't try to initiate a render pass
	};
};

// How the texture is intended to be used by a given pass
struct CrRenderGraphTextureUsage2
{
	CrRenderGraphTextureUsage2() 
		: textureIndex((Textures::T)-1)
	{}

	ICrTexture* texture = nullptr;

	uint32_t mipmapStart = 0;
	uint32_t mipmapCount = 1;

	uint32_t sliceStart = 0;
	uint32_t sliceCount = 1;

	cr3d::TexturePlane::T texturePlane = cr3d::TexturePlane::Color;

	// TODO Move to subresource
	uint64_t subresourceId = 0xffffffff;

	union
	{
		Textures::T textureIndex;
		RWTextures::T rwTextureIndex;
	};

	float4 clearColor;
	float depthClearValue = 0.0f;
	uint8_t stencilClearValue = 0;

	CrRenderTargetLoadOp loadOp          = CrRenderTargetLoadOp::Load;
	CrRenderTargetStoreOp storeOp        = CrRenderTargetStoreOp::Store;
	CrRenderTargetLoadOp stencilLoadOp   = CrRenderTargetLoadOp::DontCare;
	CrRenderTargetStoreOp stencilStoreOp = CrRenderTargetStoreOp::DontCare;

	cr3d::TextureState state;
};

// A transition structure that describes the intended state, the state before and the intended state after
struct CrRenderGraphTextureTransitionInfo2
{
	uint32_t mipmapStart = 0;
	uint32_t mipmapCount = 1;

	uint32_t sliceStart = 0;
	uint32_t sliceCount = 1;

	cr3d::TextureState initialState; // State it was in before
	cr3d::TextureState usageState;   // State we want it to be used in
	cr3d::TextureState finalState;   // State it needs to be left in for the next pass
};

// How this buffer is intended to be used in this pass
struct CrRenderGraphBufferUsage2
{
	const ICrHardwareGPUBuffer* buffer = nullptr;

	union
	{
		StorageBuffers::T storageBufferIndex;
		RWStorageBuffers::T rwStorageBufferIndex;
		TypedBuffers::T typedBufferIndex;
		RWTypedBuffers::T rwTypedBufferIndex;
	};

	uint32_t bufferId;
	uint32_t size;
	uint32_t offset;

	cr3d::ShaderResourceType::T resourceType = cr3d::ShaderResourceType::Count;
	cr3d::BufferState::T usageState = cr3d::BufferState::Undefined;
	cr3d::ShaderStageFlags::T shaderStages = cr3d::ShaderStageFlags::None;
};

// A transition structure that describes the intended state, the state before and the intended state after
struct CrRenderGraphBufferTransitionInfo2
{
	uint32_t offset = 0;
	uint32_t size = 0;

	cr3d::BufferState::T initialState = cr3d::BufferState::Undefined; // State it comes in
	cr3d::BufferState::T usageState = cr3d::BufferState::Undefined; // State I want it to be used in
	cr3d::BufferState::T finalState = cr3d::BufferState::Undefined; // State it needs to be left in

	cr3d::ShaderStageFlags::T initialShaderStages = cr3d::ShaderStageFlags::None;
	cr3d::ShaderStageFlags::T usageShaderStages = cr3d::ShaderStageFlags::None;
	cr3d::ShaderStageFlags::T finalShaderStages = cr3d::ShaderStageFlags::None;
};

struct CrRenderGraphPass2
{
	CrRenderGraphString name;

	float4 color;

	CrRenderGraphPassType::T type;

	CrFixedVector<CrRenderGraphTextureUsage2, 16> textureUsages;

	CrFixedVector<CrRenderGraphBufferUsage2, 16> bufferUsages;

	CrFixedHashMap<uint64_t, CrRenderGraphTextureTransitionInfo2, 16> textureTransitionInfos;

	CrFixedHashMap<uint64_t, CrRenderGraphBufferTransitionInfo2, 16> bufferTransitionInfos;

	ICrTexture* depthTexture;

	CrRenderGraphExecutionFunction executionFunction;
};

struct CrRenderGraphFrameParams
{
	CrRenderGraphFrameParams()
		: commandBuffer(nullptr)
		, timingQueryTracker(nullptr)
		, frameIndex(0)
	{}

	ICrCommandBuffer* commandBuffer;
	CrGPUTimingQueryTracker* timingQueryTracker;
	uint64_t frameIndex;
};

class CrRenderGraph
{
public:

	CrRenderGraph()
		: m_workingPassIndex(0)
		, m_subresourceIdCounter(0)
		, m_bufferIdCounter(0)
	{}

	void AddRenderPass(const CrRenderGraphString& name, const float4& color, CrRenderGraphPassType::T type, const CrRenderGraphSetupFunction& setupFunction, const CrRenderGraphExecutionFunction& executionFunction);

	//----------------
	// Texture binding
	//----------------

	uint32_t GetSubresourceId(CrHash subresourceHash);

	void BindTexture(Textures::T textureIndex, ICrTexture* texture, cr3d::ShaderStageFlags::T shaderStages);

	void BindRWTexture(RWTextures::T rwTextureIndex, ICrTexture* texture, cr3d::ShaderStageFlags::T shaderStages, uint32_t mipmap = 0, uint32_t sliceStart = 0, uint32_t sliceCount = 1);

	void BindRenderTarget
	(
		ICrTexture* texture,
		CrRenderTargetLoadOp loadOp = CrRenderTargetLoadOp::Load,
		CrRenderTargetStoreOp storeOp = CrRenderTargetStoreOp::Store,
		float4 clearColor = float4(),
		uint32_t mipmap = 0, uint32_t slice = 0
	);

	void BindDepthStencilTarget
	(
		ICrTexture* texture,
		CrRenderTargetLoadOp loadOp = CrRenderTargetLoadOp::Load,
		CrRenderTargetStoreOp storeOp = CrRenderTargetStoreOp::Store,
		float depthClearValue = 0.0f,
		CrRenderTargetLoadOp stencilLoadOp = CrRenderTargetLoadOp::DontCare,
		CrRenderTargetStoreOp stencilStoreOp = CrRenderTargetStoreOp::DontCare,
		uint8_t stencilClearValue = 0,
		uint32_t mipmap = 0, uint32_t slice = 0,
		bool readOnlyDepth = false, bool readOnlyStencil = false
	);

	void BindSwapchain(ICrTexture* texture, uint32_t mipmap = 0, uint32_t slice = 0);

	//---------------
	// Buffer binding
	//---------------

	uint32_t GetUniqueBufferId(CrHash bufferHash);

	void BindStorageBuffer(StorageBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, cr3d::ShaderStageFlags::T shaderStages, uint32_t numElements, uint32_t stride, uint32_t offset);

	void BindStorageBuffer(StorageBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, cr3d::ShaderStageFlags::T shaderStages);

	void BindRWStorageBuffer(RWStorageBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, cr3d::ShaderStageFlags::T shaderStages, uint32_t numElements, uint32_t stride, uint32_t offset);

	void BindRWStorageBuffer(RWStorageBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, cr3d::ShaderStageFlags::T shaderStages);

	void BindTypedBuffer(TypedBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, cr3d::ShaderStageFlags::T shaderStages, uint32_t numElements, uint32_t stride, uint32_t offset);

	void BindTypedBuffer(TypedBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, cr3d::ShaderStageFlags::T shaderStages);

	void BindRWTypedBuffer(RWTypedBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, cr3d::ShaderStageFlags::T shaderStages, uint32_t numElements, uint32_t stride, uint32_t offset);

	void BindRWTypedBuffer(RWTypedBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, cr3d::ShaderStageFlags::T shaderStages);

	void Begin(const CrRenderGraphFrameParams& frameParams);

	void Execute();

	void End();

	template<typename FunctionT>
	void ForEachPass(const FunctionT& function)
	{
		for (size_t i = 0; i < m_workingPasses.size(); ++i)
		{
			function(m_workingPasses[i]);
		}
	}

private:

	CrRenderGraphPass2& GetWorkingRenderPass() { return m_workingPasses[m_workingPassIndex]; }

	size_t m_workingPassIndex;

	CrFixedVector<CrRenderGraphPass2, 128> m_workingPasses;

	uint32_t m_subresourceIdCounter;

	uint32_t m_bufferIdCounter;

	CrFixedHashMap<CrHash, uint32_t, 256> m_textureSubresourceIds;

	CrFixedHashMap<CrHash, uint32_t, 256> m_bufferIds;

	// Last render pass a certain subresource was used in. Null if it wasn't used yet
	CrFixedVector<CrRenderGraphPass2*, 256> m_textureLastUsedPass;

	// Last render pass a certain buffer was used in. Null if it wasn't used yet
	CrFixedVector<CrRenderGraphPass2*, 256> m_bufferLastUsedPass;

	CrRenderGraphFrameParams m_frameParams;
};