#pragma once

#include "Rendering/CrRendering.h"
#include "Rendering/CrRenderingForwardDeclarations.h"

#include "Core/Containers/CrHashMap.h"
#include "Core/Containers/CrVector.h"
#include "Core/String/CrFixedString.h"
#include "Core/Function/CrFixedFunction.h"
#include "Core/CrTypedId.h"
#include "Core/Logging/ICrDebug.h"

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

struct CrRenderGraphTextureDescriptor
{
	cr3d::DataFormat::T format = cr3d::DataFormat::RGBA8_Unorm;
	int usage = 0;
	uint32_t width = 1;
	uint32_t height = 1;
	uint32_t mipmapCount = 1;
	uint32_t sliceCount = 1;

	// Physical texture. Can be assigned manually or pulled from a pool
	ICrTexture* texture = nullptr;
};

struct CrRenderGraphBufferDescriptor
{
	// TODO Add more properties when the buffer is dynamically allocated

	CrGPUBuffer* buffer = nullptr;
};

// How the texture is used by a given pass
struct CrRenderGraphTextureUsage
{
	CrRenderGraphTextureId textureId;

	uint32_t mipmapStart = 0;
	uint32_t mipmapCount = 1;

	uint32_t sliceStart = 0;
	uint32_t sliceCount = 1;

	float4 clearColor;
	float depthClearValue = 0.0f;
	uint32_t stencilClearValue = 0;

	CrRenderTargetLoadOp loadOp = CrRenderTargetLoadOp::Load;
	CrRenderTargetStoreOp storeOp = CrRenderTargetStoreOp::Store;
	CrRenderTargetLoadOp stencilLoadOp = CrRenderTargetLoadOp::DontCare;
	CrRenderTargetStoreOp stencilStoreOp = CrRenderTargetStoreOp::DontCare;

	cr3d::TextureState::T state = cr3d::TextureState::Undefined;
	cr3d::ShaderStageFlags::T shaderStages = cr3d::ShaderStageFlags::None;
};

struct CrRenderGraphBufferUsage
{
	CrRenderGraphBufferId bufferId;

	cr3d::BufferState::T usageState = cr3d::BufferState::Undefined;
	cr3d::ShaderStageFlags::T shaderStages = cr3d::ShaderStageFlags::None;
};

// Description of a texture in the render graph. The properties in the descriptor
// are used to allocate the temporary resource as and when it's required.
struct CrRenderGraphTextureResource
{
	CrRenderGraphTextureResource(const CrRenderGraphString& name, CrRenderGraphTextureId id, const CrRenderGraphTextureDescriptor& descriptor)
		: name(name), id(id), descriptor(descriptor) {}

	CrRenderGraphString name;
	CrRenderGraphTextureId id;
	CrRenderGraphTextureDescriptor descriptor;
};

struct CrRenderGraphTextureTransition
{
	CrRenderGraphString name;
	ICrTexture* texture = nullptr;

	uint32_t mipmapStart = 0;
	uint32_t mipmapCount = 1;

	uint32_t sliceStart = 0;
	uint32_t sliceCount = 1;

	cr3d::TextureState::T initialState = cr3d::TextureState::Undefined; // State it comes in
	cr3d::TextureState::T usageState = cr3d::TextureState::Undefined; // State I want it to be used in
	cr3d::TextureState::T finalState = cr3d::TextureState::Undefined; // State it needs to be left in

	cr3d::ShaderStageFlags::T initialShaderStages = cr3d::ShaderStageFlags::None;
	cr3d::ShaderStageFlags::T usageShaderStages = cr3d::ShaderStageFlags::None;
	cr3d::ShaderStageFlags::T finalShaderStages = cr3d::ShaderStageFlags::None;
};

struct CrRenderGraphBufferTransition
{
	CrRenderGraphString name;
	CrGPUBuffer* buffer = nullptr;

	uint32_t offset = 0;
	uint32_t size = 0;

	cr3d::BufferState::T initialState = cr3d::BufferState::Undefined; // State it comes in
	cr3d::BufferState::T usageState = cr3d::BufferState::Undefined; // State I want it to be used in
	cr3d::BufferState::T finalState = cr3d::BufferState::Undefined; // State it needs to be left in

	cr3d::ShaderStageFlags::T initialShaderStages = cr3d::ShaderStageFlags::None;
	cr3d::ShaderStageFlags::T usageShaderStages = cr3d::ShaderStageFlags::None;
	cr3d::ShaderStageFlags::T finalShaderStages = cr3d::ShaderStageFlags::None;
};

struct CrRenderGraphBufferResource
{
	CrRenderGraphBufferResource(const CrRenderGraphString& name, CrRenderGraphBufferId id, const CrRenderGraphBufferDescriptor& descriptor)
		: name(name), id(id), descriptor(descriptor) {}

	CrRenderGraphString name;
	CrRenderGraphBufferId id;
	CrRenderGraphBufferDescriptor descriptor;
};

struct CrRenderGraphPass
{
	CrRenderGraphPass(const CrRenderGraphString& name, const float4& color, CrRenderGraphPassType::T type, const CrRenderGraphExecutionFunction& executionFunction)
		: name(name), color(color), type(type), executionFunction(executionFunction) {}

	CrRenderGraphString name;

	float4 color;

	CrRenderGraphPassType::T type;

	CrRenderGraphExecutionFunction executionFunction;

	// Texture usage for this pass
	CrVector<CrRenderGraphTextureUsage> textureUsages;

	CrHashMap<uint32_t, CrRenderGraphTextureTransition> textureTransitions;
	
	// Buffer usage for this pass
	CrVector<CrRenderGraphBufferUsage> bufferUsages;

	CrHashMap<uint32_t, CrRenderGraphBufferTransition> bufferTransitions;
};

// Objectives
// 
// There are certain features we are looking for in a render graph. Here is a non-exhaustive list
// of what the render graph wants to achieve
//
// 1) Deduce source and destination transitions for each resource in a pass automatically
// 2) Dispatch portions of the render graph to different queues and command buffers, in different threads
// 3) Reserve render target or buffer memory automatically if needed
// 4) Express split barriers
// 5) Express memory dependencies
// 6) Express temporal or history buffers
// 7) Inject passes between existing passes, and remove passes
// 8) Handle async compute
// 9) Reuse code (e.g. blur) without having name clashes
// 10) Multiple render passes inside a rendering (e.g. renderdoc) scope

// API and Usage
//
// 1) Add a render pass by doing renderGraph.AddPass
// 1.1) A pass has a unique name, and two functions: setup and execution. Adding a pass with the same name is an error
// 1.2) A pass gets assigned a unique id, and has a type that determines its purpose and behavior
// 
// 2) Resources are declared outside the setup of a render pass. That allows passes to be independent in terms of resources.
// e.g. if a depth texture is incrementally constructed through the prepass, the gbuffer, decals, etc but the prepass and decals are optional.
// 2.1) Resources are described when adding to the graph. Resources can be transient or fixed (like a history buffer)
// We have the option to attach a physical resource, but failing that, they get allocated from a pool in a platform-specific way. As resources
// are allocated based on their usage, a declared resource that's never used doesn't use any memory. This is not an error
// 2.2) Each resource has a unique id and name. Adding a resource with the same name is an error
// 
// 3) Setup takes in the rendergraph, and is a place to register resource usage. Note that resources can also be declared inside the setup pass
// but one cannot use the result of the setup pass inside the execution pass if capturing by value
// 3.1) Resources can be used by name or directly by resource id. Using the id directly is more efficient
// and less error-prone but only makes sense inside the engine or with a certain amount of coupling. Using the name
// makes sense when extending it from a "game" perspective, where we might have a database of names but not the handle itself.
// 
// 4) Execution takes in the render graph and a command buffer, where actual commands are executed
// 4.1) All passes are executed sequentially once their order has been determined. One has to think of the render graph as an
// independent execution timeline from the one where the lambda was created

struct CrRenderGraphFrameParams
{
	ICrCommandBuffer* commandBuffer;
	CrGPUTimingQueryTracker* timingQueryTracker;
};

class CrRenderGraph
{
public:

	CrRenderGraph();

	void AddRenderPass(const CrRenderGraphString& name, const float4& color, CrRenderGraphPassType::T type, const CrRenderGraphSetupFunction& setupFunction, const CrRenderGraphExecutionFunction& executionFunction);

	CrRenderGraphTextureId CreateTexture(const CrRenderGraphString& name, const CrRenderGraphTextureDescriptor& descriptor);

	CrRenderGraphBufferId CreateBuffer(const CrRenderGraphString& name, const CrRenderGraphBufferDescriptor& descriptor);

	void AddTexture(CrRenderGraphTextureId textureId, cr3d::ShaderStageFlags::T shaderStages);

	void AddRWTexture(CrRenderGraphTextureId textureId, cr3d::ShaderStageFlags::T shaderStages, uint32_t mipmapStart = 0, uint32_t mipmapCount = 1, uint32_t sliceStart = 0, uint32_t sliceCount = 1);

	void AddRenderTarget
	(
		CrRenderGraphTextureId textureId,
		CrRenderTargetLoadOp loadOp = CrRenderTargetLoadOp::Load,
		CrRenderTargetStoreOp storeOp = CrRenderTargetStoreOp::Store,
		float4 clearColor = float4(),
		uint32_t mipmap = 0, uint32_t slice = 0
	);

	void AddDepthStencilTarget
	(
		CrRenderGraphTextureId textureId,
		CrRenderTargetLoadOp loadOp = CrRenderTargetLoadOp::Load,
		CrRenderTargetStoreOp storeOp = CrRenderTargetStoreOp::Store,
		float depthClearValue = 0.0f,
		CrRenderTargetLoadOp stencilLoadOp = CrRenderTargetLoadOp::DontCare, 
		CrRenderTargetStoreOp stencilStoreOp = CrRenderTargetStoreOp::DontCare, 
		uint32_t stencilClearValue = 0,
		uint32_t mipmap = 0, uint32_t slice = 0
	);

	void AddSwapchain(CrRenderGraphTextureId textureId);

	void AddBuffer(CrRenderGraphBufferId bufferId, cr3d::ShaderStageFlags::T shaderStages);

	void AddRWBuffer(CrRenderGraphBufferId bufferId, cr3d::ShaderStageFlags::T shaderStages);

	ICrTexture* GetPhysicalTexture(CrRenderGraphTextureId textureId) const
	{
		return m_textureResources[textureId.id].descriptor.texture;
	}

	CrGPUBuffer* GetPhysicalBuffer(CrRenderGraphBufferId bufferId) const
	{
		return m_bufferResources[bufferId.id].descriptor.buffer;
	}

	void Execute();

	void Begin(const CrRenderGraphFrameParams& setupParams);

	void End();

	template<typename FunctionT>
	void ForEachPass(const FunctionT& function)
	{
		for (CrRenderPassId logicalPassId(0); logicalPassId < CrRenderPassId((uint32_t)m_logicalPasses.size()); ++logicalPassId)
		{
			function(m_logicalPasses[logicalPassId.id]);
		}
	}

private:

	CrRenderPassId m_uniquePassId;
	CrRenderPassId m_workingPassId; // This id is set during the setup lambda and invalid outside of it

	CrRenderGraphTextureId m_uniqueTextureId;
	CrRenderGraphBufferId m_uniqueBufferId;

	// Vector of passes in submission order
	CrVector<CrRenderGraphPass> m_logicalPasses;

	// From name to pass id
	CrHashMap<CrRenderGraphString, CrRenderPassId> m_nameToLogicalPassId;

	// Textures declared for this graph
	CrVector<CrRenderGraphTextureResource> m_textureResources;

	// From name to texture id
	CrHashMap<CrRenderGraphString, CrRenderGraphTextureId> m_nameToTextureId;

	// Buffers declared for this graph
	CrVector<CrRenderGraphBufferResource> m_bufferResources;

	// From name to buffer id
	CrHashMap<CrRenderGraphString, CrRenderGraphBufferId> m_nameToBufferId;

	// Pass these resources were last seen in. As we traverse the graph we take note of which resources
	// we've been encountering and update passes as necessary
	CrVector<CrRenderPassId> m_textureLastUsedPass;

	CrVector<CrRenderPassId> m_bufferLastUsedPass;

	CrRenderGraphFrameParams m_frameParams;
};