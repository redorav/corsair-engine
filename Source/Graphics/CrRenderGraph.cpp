#include "Graphics/CrRendering_pch.h"

#include "Graphics/CrRenderGraph.h"
#include "Graphics/ITexture.h"
#include "Graphics/ICommandBuffer.h"
#include "Graphics/GPUBuffer.h"
#include "Graphics/CrGPUTimingQueryTracker.h"
#include "Graphics/RenderPassDescriptor.h"

#include "Core/Logging/ICrDebug.h"

//#define RENDER_GRAPH_LOGS

#if defined(RENDER_GRAPH_LOGS)
#define CrRenderGraphLog(format, ...) CrLog(format, __VA_ARGS__)
#else
#define CrRenderGraphLog(format, ...)
#endif

void CrRenderGraph::AddRenderPass
(
	const CrRenderGraphString& name, const float4& color, CrRenderGraphPassType::T type,
	const CrRenderGraphSetupFunction& setupFunction, const CrRenderGraphExecutionFunction& executionFunction
)
{
	CrRenderGraphPass& workingPass = m_workingPasses.push_back();
	workingPass.name = name;
	workingPass.color = color;
	workingPass.type = type;
	workingPass.executionFunction = executionFunction;

	setupFunction(*this);

	m_workingPassIndex++;
}

uint32_t CrRenderGraph::GetSubresourceId(CrHash subresourceHash)
{
	uint32_t subresourceId = 0xffffffff;

	const auto subresourceIter = m_textureSubresourceIds.find(subresourceHash);
	if (subresourceIter != m_textureSubresourceIds.end())
	{
		subresourceId = subresourceIter->second;
	}
	else
	{
		subresourceId = m_subresourceIdCounter++;
		m_textureSubresourceIds.insert(subresourceHash, subresourceId);
	}

	return subresourceId;
}

void CrRenderGraph::BindTexture(Textures::T textureIndex, crgfx::ITexture* texture, crgfx::ShaderStageFlags::T shaderStages, crgfx::TextureView view)
{
	CrRenderGraphPass& workingPass = GetWorkingRenderPass();

	CrHash subresourceHash;
	subresourceHash << (uintptr_t)texture;

	CrRenderGraphTextureUsage textureUsage;
	textureUsage.texture = texture;
	textureUsage.view = view;
	textureUsage.textureIndex = textureIndex;
	textureUsage.state = crgfx::TextureState(crgfx::TextureLayout::ShaderInput, shaderStages);
	textureUsage.subresourceId = GetSubresourceId(subresourceHash);
	workingPass.textureUsages.push_back(textureUsage);

	CrRenderGraphLog("Added Texture %s", texture->GetDebugName());
}

void CrRenderGraph::BindRWTexture(RWTextures::T rwTextureIndex, crgfx::ITexture* texture, crgfx::ShaderStageFlags::T shaderStages, uint32_t mipmap, uint32_t sliceStart, uint32_t sliceCount)
{
	CrRenderGraphPass& workingPass = GetWorkingRenderPass();

	CrHash subresourceHash;
	subresourceHash << (uintptr_t)texture;

	CrRenderGraphTextureUsage textureUsage;
	textureUsage.texture = texture;
	textureUsage.view.mipmapStart = mipmap;
	textureUsage.view.mipmapCount = 1;
	textureUsage.view.sliceStart = sliceStart;
	textureUsage.view.sliceCount = sliceCount;
	textureUsage.rwTextureIndex = rwTextureIndex;
	textureUsage.state = crgfx::TextureState(crgfx::TextureLayout::RWTexture, shaderStages);
	textureUsage.subresourceId = GetSubresourceId(subresourceHash);
	workingPass.textureUsages.push_back(textureUsage);

	CrRenderGraphLog("Added RWTexture %s", texture->GetDebugName());
}

void CrRenderGraph::BindRenderTarget
(
	crgfx::ITexture* texture,
	crgfx::RenderTargetLoadOp loadOp,
	crgfx::RenderTargetStoreOp storeOp,
	float4 clearColor,
	uint32_t mipmap, uint32_t slice
)
{
	CrRenderGraphPass& workingPass = GetWorkingRenderPass();

	CrHash subresourceHash;
	subresourceHash << (uintptr_t)texture;

	CrRenderGraphTextureUsage textureUsage;
	textureUsage.texture = texture;
	textureUsage.view.mipmapStart = mipmap;
	textureUsage.view.mipmapCount = 1;
	textureUsage.view.sliceStart = slice;
	textureUsage.view.sliceCount = 1;
	textureUsage.clearColor = clearColor;
	textureUsage.storeOp = storeOp;
	textureUsage.loadOp = loadOp;
	textureUsage.state = crgfx::TextureState(crgfx::TextureLayout::RenderTarget, crgfx::ShaderStageFlags::Unused);
	textureUsage.subresourceId = GetSubresourceId(subresourceHash);
	workingPass.textureUsages.push_back(textureUsage);

	CrRenderGraphLog("Added Render Target %s", texture->GetDebugName());
}

void CrRenderGraph::BindDepthStencilTarget
(
	crgfx::ITexture* texture,
	crgfx::RenderTargetLoadOp loadOp,
	crgfx::RenderTargetStoreOp storeOp,
	float depthClearValue,
	crgfx::RenderTargetLoadOp stencilLoadOp,
	crgfx::RenderTargetStoreOp stencilStoreOp,
	uint8_t stencilClearValue,
	uint32_t mipmap, uint32_t slice,
	bool readOnlyDepth, bool readOnlyStencil
)
{
	CrRenderGraphPass& workingPass = GetWorkingRenderPass();

	CrAssertMsg(workingPass.type == CrRenderGraphPassType::Graphics, "Render pass must be graphics");
	CrAssertMsg(workingPass.depthTexture == nullptr, "Cannot bind multiple depth targets");

	CrHash subresourceHash;
	subresourceHash << (uintptr_t)texture;

	CrRenderGraphTextureUsage textureUsage;
	textureUsage.texture = texture;
	textureUsage.view.mipmapStart = mipmap;
	textureUsage.view.mipmapCount = 1;
	textureUsage.view.sliceStart = slice;
	textureUsage.view.sliceCount = 1;
	textureUsage.depthClearValue = depthClearValue;
	textureUsage.stencilClearValue = stencilClearValue;
	textureUsage.stencilLoadOp = stencilLoadOp;
	textureUsage.stencilStoreOp = stencilStoreOp;
	textureUsage.storeOp = storeOp;
	textureUsage.loadOp = loadOp;
	textureUsage.subresourceId = GetSubresourceId(subresourceHash);

	// If we don't care about either the inputs or the outputs, we can conclude that nothing meaningful is going to be written to it
	bool writeDepth = loadOp == crgfx::RenderTargetLoadOp::Clear || storeOp != crgfx::RenderTargetStoreOp::DontCare;
	bool writeStencil = stencilLoadOp == crgfx::RenderTargetLoadOp::Clear || stencilStoreOp != crgfx::RenderTargetStoreOp::DontCare;

	CrAssertMsg(!(writeDepth && readOnlyDepth), "Cannot read and write to depth simultaneously");
	CrAssertMsg(!(writeStencil && readOnlyStencil), "Cannot read and write to stencil simultaneously");

	// Read-only depth specifies that we want to prevent writing to the selected aspect of the render target
	// If read-only is set to false on both aspects, it just means that we don't want to prevent writing
	// We can still prevent reading if we detect that 

	// If we're not loading or clearing the depth or stencil, we know we won't test it
	bool readDepth = loadOp != crgfx::RenderTargetLoadOp::DontCare;
	bool readStencil = stencilLoadOp != crgfx::RenderTargetLoadOp::DontCare;

	crgfx::TextureLayout::T layout = crgfx::TextureLayout::Count;

	if (readOnlyDepth && writeStencil)
	{
		layout = crgfx::TextureLayout::StencilWriteDepthReadOnly;
	}
	else if (readOnlyStencil && writeDepth)
	{
		layout = crgfx::TextureLayout::DepthWriteStencilReadOnly;
	}
	else if (readOnlyDepth && readOnlyStencil)
	{
		layout = crgfx::TextureLayout::DepthStencilReadOnly;
	}
	else
	{
		if (writeDepth || writeStencil)
		{
			if (readDepth || readStencil)
			{
				layout = crgfx::TextureLayout::DepthStencilReadWrite;
			}
			else
			{
				layout = crgfx::TextureLayout::DepthStencilWrite;
			}
		}
		else
		{
			layout = crgfx::TextureLayout::DepthStencilReadOnly;
		}
	}

	CrAssertMsg(layout != crgfx::TextureLayout::Count, "Invalid layout selected");

	textureUsage.state = { layout, crgfx::ShaderStageFlags::Unused };

	workingPass.depthTexture = texture;

	workingPass.textureUsages.push_back(textureUsage);

	CrRenderGraphLog("Added Depth Stencil Target %s", texture->GetDebugName());
}

void CrRenderGraph::BindSwapchain(crgfx::ITexture* texture, uint32_t mipmap, uint32_t slice)
{
	CrRenderGraphPass& workingPass = GetWorkingRenderPass();

	CrHash subresourceHash;
	subresourceHash << (uintptr_t)texture;

	CrRenderGraphTextureUsage textureUsage;
	textureUsage.texture = texture;
	textureUsage.view.mipmapStart = mipmap;
	textureUsage.view.mipmapCount = 1;
	textureUsage.view.sliceStart = slice;
	textureUsage.view.sliceCount = 1;
	textureUsage.state = { crgfx::TextureLayout::Present, crgfx::ShaderStageFlags::Unused };
	textureUsage.subresourceId = GetSubresourceId(subresourceHash);
	workingPass.textureUsages.push_back(textureUsage);

	CrRenderGraphLog("Added Swapchain %s", texture->GetDebugName());
}

uint32_t CrRenderGraph::GetUniqueBufferId(CrHash bufferHash)
{
	uint32_t bufferId = 0xffffffff;

	const auto bufferIter = m_bufferIds.find(bufferHash);
	if (bufferIter != m_bufferIds.end())
	{
		bufferId = bufferIter->second;
	}
	else
	{
		bufferId = m_bufferIdCounter++;
		m_bufferIds.insert(bufferHash, bufferId);
	}

	return bufferId;
}

void CrRenderGraph::BindStorageBuffer(StorageBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, crgfx::ShaderStageFlags::T shaderStages, uint32_t numElements, uint32_t stride, uint32_t offset)
{
	CrRenderGraphPass& workingPass = GetWorkingRenderPass();

	CrHash bufferHash;
	bufferHash << (uintptr_t)buffer;

	CrRenderGraphBufferUsage bufferUsage;
	bufferUsage.buffer = buffer;
	bufferUsage.usageState = crgfx::BufferState::ShaderInput;
	bufferUsage.shaderStages = shaderStages;
	bufferUsage.storageBufferIndex = bufferIndex;
	bufferUsage.resourceType = crgfx::ShaderResourceType::StorageBuffer;
	bufferUsage.bufferId = GetUniqueBufferId(bufferHash);
	bufferUsage.numElements = numElements;
	bufferUsage.stride = stride;
	bufferUsage.offset = offset;
	workingPass.bufferUsages.push_back(bufferUsage);
}

void CrRenderGraph::BindStorageBuffer(StorageBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, crgfx::ShaderStageFlags::T shaderStages)
{
	BindStorageBuffer(bufferIndex, buffer, shaderStages, buffer->GetNumElements(), buffer->GetStrideBytes(), 0);
}

void CrRenderGraph::BindRWStorageBuffer(RWStorageBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, crgfx::ShaderStageFlags::T shaderStages, uint32_t numElements, uint32_t stride, uint32_t offset)
{
	CrRenderGraphPass& workingPass = GetWorkingRenderPass();

	CrHash bufferHash;
	bufferHash << (uintptr_t)buffer;

	CrRenderGraphBufferUsage bufferUsage;
	bufferUsage.buffer               = buffer;
	bufferUsage.usageState           = crgfx::BufferState::ReadWrite;
	bufferUsage.shaderStages         = shaderStages;
	bufferUsage.rwStorageBufferIndex = bufferIndex;
	bufferUsage.resourceType         = crgfx::ShaderResourceType::RWStorageBuffer;
	bufferUsage.bufferId             = GetUniqueBufferId(bufferHash);
	bufferUsage.numElements          = numElements;
	bufferUsage.stride               = stride;
	bufferUsage.offset               = offset;
	workingPass.bufferUsages.push_back(bufferUsage);
}

void CrRenderGraph::BindRWStorageBuffer(RWStorageBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, crgfx::ShaderStageFlags::T shaderStages)
{
	BindRWStorageBuffer(bufferIndex, buffer, shaderStages, buffer->GetNumElements(), buffer->GetStrideBytes(), 0);
}

void CrRenderGraph::BindTypedBuffer(TypedBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, crgfx::ShaderStageFlags::T shaderStages, uint32_t numElements, uint32_t stride, uint32_t offset)
{
	CrRenderGraphPass& workingPass = GetWorkingRenderPass();

	CrHash bufferHash;
	bufferHash << (uintptr_t)buffer;

	CrRenderGraphBufferUsage bufferUsage;
	bufferUsage.buffer           = buffer;
	bufferUsage.usageState       = crgfx::BufferState::ShaderInput;
	bufferUsage.shaderStages     = shaderStages;
	bufferUsage.typedBufferIndex = bufferIndex;
	bufferUsage.resourceType     = crgfx::ShaderResourceType::TypedBuffer;
	bufferUsage.bufferId         = GetUniqueBufferId(bufferHash);
	bufferUsage.numElements      = numElements;
	bufferUsage.stride           = stride;
	bufferUsage.offset           = offset;
	workingPass.bufferUsages.push_back(bufferUsage);
}

void CrRenderGraph::BindTypedBuffer(TypedBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, crgfx::ShaderStageFlags::T shaderStages)
{
	BindTypedBuffer(bufferIndex, buffer, shaderStages, buffer->GetNumElements(), buffer->GetStrideBytes(), 0);
}

void CrRenderGraph::BindRWTypedBuffer(RWTypedBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, crgfx::ShaderStageFlags::T shaderStages, uint32_t numElements, uint32_t stride, uint32_t offset)
{
	CrRenderGraphPass& workingPass = GetWorkingRenderPass();

	CrHash bufferHash;
	bufferHash << (uintptr_t)buffer;

	CrRenderGraphBufferUsage bufferUsage;
	bufferUsage.buffer             = buffer;
	bufferUsage.usageState         = crgfx::BufferState::ReadWrite;
	bufferUsage.shaderStages       = shaderStages;
	bufferUsage.rwTypedBufferIndex = bufferIndex;
	bufferUsage.resourceType       = crgfx::ShaderResourceType::TypedBuffer;
	bufferUsage.bufferId           = GetUniqueBufferId(bufferHash);
	bufferUsage.numElements        = numElements;
	bufferUsage.stride             = stride;
	bufferUsage.offset             = offset;
	workingPass.bufferUsages.push_back(bufferUsage);
}

void CrRenderGraph::BindRWTypedBuffer(RWTypedBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, crgfx::ShaderStageFlags::T shaderStages)
{
	BindRWTypedBuffer(bufferIndex, buffer, shaderStages, buffer->GetNumElements(), buffer->GetStrideBytes(), 0);
}

void CrRenderGraph::Begin(const CrRenderGraphFrameParams& frameParams)
{
	m_frameParams = frameParams;

	m_workingPassIndex = 0;
	m_subresourceIdCounter = 0;
	m_bufferIdCounter = 0;

	CrRenderGraphLog("------------------------------------");
	CrRenderGraphLog("Beginning Render Graph For Frame %ld", m_frameParams.frameIndex);
	CrRenderGraphLog("------------------------------------");
}

void CrRenderGraph::Execute()
{
	m_textureLastUsedPass.clear();
	m_textureLastUsedPass.resize(m_textureSubresourceIds.size(), nullptr);
	m_bufferLastUsedPass.clear();
	m_bufferLastUsedPass.resize(m_textureSubresourceIds.size(), nullptr);

	for (size_t renderGraphPassIndex = 0; renderGraphPassIndex < m_workingPasses.size(); ++renderGraphPassIndex)
	{
		CrRenderGraphPass* renderGraphPass = &m_workingPasses[renderGraphPassIndex];

		// Process textures within a pass
		for (uint32_t textureIndex = 0; textureIndex < renderGraphPass->textureUsages.size(); ++textureIndex)
		{
			const CrRenderGraphTextureUsage& textureUsage = renderGraphPass->textureUsages[textureIndex];

			CrRenderGraphTextureTransitionInfo transitionInfo;
			transitionInfo.usageState = textureUsage.state;

			// Initialize final state to default state until we have more information
			// TODO Track default state in the render device so that we don't always have to transition to and from the same state
			// Rename to initial state as it will probably only be used then to initialize it for the first time
			transitionInfo.finalState = textureUsage.texture->GetDefaultState();

			// Figure out what pass this subresource was last used in
			// TODO Iterate over all subresources
			CrRenderGraphPass* lastUsedRenderPass = m_textureLastUsedPass[textureUsage.subresourceId];

			if (lastUsedRenderPass)
			{
				// By definition the pass that references it has a transition info set up because we submit passes in logical order
				CrRenderGraphTextureTransitionInfo& lastUsedTransitionInfo = lastUsedRenderPass->textureTransitionInfos.find(textureUsage.subresourceId)->second;
				lastUsedTransitionInfo.finalState = textureUsage.state;
				transitionInfo.initialState = textureUsage.state;
			}
			else
			{
				// If no previous pass referenced this subresource, set the initial state
				// TODO As mentioned before track default state in the render device so that we don't always have to transition to and from the same state
				transitionInfo.initialState = crgfx::TextureState();
			}

			renderGraphPass->textureTransitionInfos.insert(textureUsage.subresourceId, transitionInfo);

			m_textureLastUsedPass[textureUsage.subresourceId] = renderGraphPass;
		}

		// Process buffers within a pass
		for (uint32_t bufferIndex = 0; bufferIndex < renderGraphPass->bufferUsages.size(); ++bufferIndex)
		{
			const CrRenderGraphBufferUsage& bufferUsage = renderGraphPass->bufferUsages[bufferIndex];

			CrRenderGraphBufferTransitionInfo transitionInfo;
			transitionInfo.usageState = bufferUsage.usageState;
			transitionInfo.finalState = bufferUsage.usageState; // Initialize final state to current state until we have more information
			transitionInfo.usageShaderStages = bufferUsage.shaderStages;
			transitionInfo.finalShaderStages = bufferUsage.shaderStages;

			CrRenderGraphPass* lastUsedRenderPass = m_bufferLastUsedPass[bufferUsage.bufferId];

			if (lastUsedRenderPass)
			{
				// Inject the final and initial states with usage states
				CrRenderGraphBufferTransitionInfo& lastUsedTransitionInfo = lastUsedRenderPass->bufferTransitionInfos.find(bufferUsage.bufferId)->second;
				lastUsedTransitionInfo.finalState = bufferUsage.usageState;
				lastUsedTransitionInfo.finalShaderStages = bufferUsage.shaderStages;

				transitionInfo.initialState = lastUsedTransitionInfo.finalState;
				transitionInfo.initialShaderStages = lastUsedTransitionInfo.finalShaderStages;
			}
			// If we didn't find the resource it means we're the first to access it.
			// In normal circumstances this could be an error (i.e. we access a resource nobody has populated)
			else
			{
				transitionInfo.initialState = crgfx::BufferState::Undefined;
				transitionInfo.initialShaderStages = renderGraphPass->type == CrRenderGraphPassType::Compute ? crgfx::ShaderStageFlags::Compute : crgfx::ShaderStageFlags::Graphics;
			}

			renderGraphPass->bufferTransitionInfos.insert(bufferUsage.bufferId, transitionInfo);

			m_bufferLastUsedPass[bufferUsage.bufferId] = renderGraphPass;
		}
	}

	for (size_t renderGraphPassIndex = 0; renderGraphPassIndex < m_workingPasses.size(); ++renderGraphPassIndex)
	{
		const CrRenderGraphPass& renderGraphPass = m_workingPasses[renderGraphPassIndex];

		CrRenderGraphLog("Executing Render Pass %s", renderGraphPass.name.c_str());

		if (renderGraphPass.type != CrRenderGraphPassType::Behavior)
		{
			crgfx::RenderPassDescriptor renderPassDescriptor;
			renderPassDescriptor.debugName = renderGraphPass.name;
			renderPassDescriptor.debugColor = renderGraphPass.color;

			if (renderGraphPass.type == CrRenderGraphPassType::Graphics)
			{
				renderPassDescriptor.type = crgfx::RenderPassType::Graphics;
			}
			else if (renderGraphPass.type == CrRenderGraphPassType::Compute)
			{
				renderPassDescriptor.type = crgfx::RenderPassType::Compute;
			}

			for (uint32_t i = 0; i < renderGraphPass.textureUsages.size(); ++i)
			{
				const CrRenderGraphTextureUsage& textureUsage = renderGraphPass.textureUsages[i];
				const CrRenderGraphTextureTransitionInfo& transitionInfo = renderGraphPass.textureTransitionInfos.find(textureUsage.subresourceId)->second;

				switch (textureUsage.state.layout)
				{
					case crgfx::TextureLayout::RenderTarget:
					{
						crgfx::RenderTargetDescriptor renderTargetDescriptor;
						renderTargetDescriptor.texture      = textureUsage.texture;
						renderTargetDescriptor.mipmap       = textureUsage.view.mipmapStart;
						renderTargetDescriptor.slice        = textureUsage.view.sliceStart;
						renderTargetDescriptor.clearColor   = textureUsage.clearColor;
						renderTargetDescriptor.loadOp       = textureUsage.loadOp;
						renderTargetDescriptor.storeOp      = textureUsage.storeOp;
						renderTargetDescriptor.initialState = transitionInfo.initialState;
						renderTargetDescriptor.usageState   = transitionInfo.usageState;
						renderTargetDescriptor.finalState   = transitionInfo.finalState;

						CrRenderGraphLog("  Render Target %s [%s -> %s -> %s]",
							textureUsage.texture->GetDebugName(),
							crgfx::TextureLayout::ToString(renderTargetDescriptor.initialState.layout),
							crgfx::TextureLayout::ToString(renderTargetDescriptor.usageState.layout),
							crgfx::TextureLayout::ToString(renderTargetDescriptor.finalState.layout));

						renderPassDescriptor.color.push_back(renderTargetDescriptor);
						break;
					}
					case crgfx::TextureLayout::DepthStencilReadWrite:
					case crgfx::TextureLayout::DepthStencilWrite:
					case crgfx::TextureLayout::StencilWriteDepthReadOnly:
					case crgfx::TextureLayout::DepthWriteStencilReadOnly:
					case crgfx::TextureLayout::DepthStencilReadOnly:
					case crgfx::TextureLayout::DepthStencilReadOnlyShader:
					{
						crgfx::RenderTargetDescriptor depthDescriptor;
						depthDescriptor.texture           = textureUsage.texture;
						depthDescriptor.mipmap            = textureUsage.view.mipmapStart;
						depthDescriptor.slice             = textureUsage.view.sliceStart;
						depthDescriptor.depthClearValue   = textureUsage.depthClearValue;
						depthDescriptor.stencilClearValue = textureUsage.stencilClearValue;
						depthDescriptor.loadOp            = textureUsage.loadOp;
						depthDescriptor.storeOp           = textureUsage.storeOp;
						depthDescriptor.stencilLoadOp     = textureUsage.stencilLoadOp;
						depthDescriptor.stencilStoreOp    = textureUsage.stencilStoreOp;
						depthDescriptor.initialState      = transitionInfo.initialState;
						depthDescriptor.usageState        = transitionInfo.usageState;
						depthDescriptor.finalState        = transitionInfo.finalState;

						CrRenderGraphLog("  Depth Stencil %s [%s -> %s -> %s]", textureUsage.texture->GetDebugName(),
							crgfx::TextureLayout::ToString(depthDescriptor.initialState.layout),
							crgfx::TextureLayout::ToString(depthDescriptor.usageState.layout),
							crgfx::TextureLayout::ToString(depthDescriptor.finalState.layout));

						renderPassDescriptor.depth = depthDescriptor;
						break;
					}
					case crgfx::TextureLayout::RWTexture:
					case crgfx::TextureLayout::ShaderInput:
					{
						if (transitionInfo.initialState.layout != transitionInfo.usageState.layout)
						{
							renderPassDescriptor.beginTextures.emplace_back
							(
								textureUsage.texture, textureUsage.view.mipmapStart, textureUsage.view.mipmapCount,
								textureUsage.view.sliceStart, textureUsage.view.sliceCount, textureUsage.view.plane,
								transitionInfo.initialState, transitionInfo.usageState
							);

							CrRenderGraphLog("  Texture %s [%s -> %s]", textureUsage.texture->GetDebugName(),
								crgfx::TextureLayout::ToString(transitionInfo.initialState.layout),
								crgfx::TextureLayout::ToString(transitionInfo.usageState.layout));
						}

						if (transitionInfo.usageState.layout != transitionInfo.finalState.layout)
						{
							renderPassDescriptor.endTextures.emplace_back
							(
								textureUsage.texture, textureUsage.view.mipmapStart, textureUsage.view.mipmapCount,
								textureUsage.view.sliceStart, textureUsage.view.sliceCount, textureUsage.view.plane,
								transitionInfo.usageState, transitionInfo.finalState
							);

							CrRenderGraphLog("  Texture %s [%s -> %s]", textureUsage.texture->GetDebugName(),
								crgfx::TextureLayout::ToString(transitionInfo.usageState.layout),
								crgfx::TextureLayout::ToString(transitionInfo.finalState.layout));
						}
						break;
					}
					default:
						CrAssertMsg(false, "Unhandled texture layout");
						break;
				}

				// Bind the texture to the slot it was assigned to
				switch (textureUsage.state.layout)
				{
					case crgfx::TextureLayout::RWTexture:
						m_frameParams.commandBuffer->BindRWTexture(textureUsage.rwTextureIndex, textureUsage.texture, textureUsage.view.mipmapStart);
						break;
					case crgfx::TextureLayout::ShaderInput:
						m_frameParams.commandBuffer->BindTexture(textureUsage.textureIndex, textureUsage.texture, textureUsage.view);
						break;
					default:
						break;
				}
			}

			for (uint32_t i = 0; i < renderGraphPass.bufferUsages.size(); ++i)
			{
				const CrRenderGraphBufferUsage& bufferUsage = renderGraphPass.bufferUsages[i];
				const CrRenderGraphBufferTransitionInfo& transitionInfo = renderGraphPass.bufferTransitionInfos.find(bufferUsage.bufferId)->second;
					
				if (transitionInfo.initialState != transitionInfo.usageState)
				{
					renderPassDescriptor.beginBuffers.emplace_back(bufferUsage.buffer,
						bufferUsage.numElements, bufferUsage.stride, bufferUsage.offset,
						transitionInfo.initialState, transitionInfo.initialShaderStages,
						transitionInfo.usageState, transitionInfo.usageShaderStages);

					CrRenderGraphLog("  Buffer %s [%s -> %s]", bufferUsage.buffer->GetDebugName(),
						crgfx::BufferState::ToString(transitionInfo.initialState),
						crgfx::BufferState::ToString(transitionInfo.usageState));
				}

				if (transitionInfo.usageState != transitionInfo.finalState)
				{
					renderPassDescriptor.endBuffers.emplace_back(bufferUsage.buffer,
						bufferUsage.numElements, bufferUsage.stride, bufferUsage.offset,
						transitionInfo.usageState, transitionInfo.usageShaderStages,
						transitionInfo.finalState, transitionInfo.finalShaderStages);

					CrRenderGraphLog("  Buffer %s [%s -> %s]", bufferUsage.buffer->GetDebugName(),
						crgfx::BufferState::ToString(transitionInfo.usageState),
						crgfx::BufferState::ToString(transitionInfo.finalState));
				}
			}

			// TODO Compute hash statically
			CrGPUTimingRequest passRequest = m_frameParams.timingQueryTracker->AllocateTimingRequest(CrHash(renderGraphPass.name.c_str(), renderGraphPass.name.length()));

			m_frameParams.commandBuffer->BeginTimestampQuery(m_frameParams.timingQueryTracker->GetCurrentQueryPool(), passRequest.startQuery);

			m_frameParams.commandBuffer->BeginRenderPass(renderPassDescriptor);

			// Execute the render graph lambda
			renderGraphPass.executionFunction(*this, m_frameParams.commandBuffer);

			m_frameParams.commandBuffer->EndRenderPass();

			m_frameParams.commandBuffer->BeginTimestampQuery(m_frameParams.timingQueryTracker->GetCurrentQueryPool(), passRequest.endQuery);
		}
		else
		{
			renderGraphPass.executionFunction(*this, m_frameParams.commandBuffer);
		}
	}
}

void CrRenderGraph::End()
{
	m_workingPasses.clear();

	m_textureSubresourceIds.clear();

	m_bufferIds.clear();

	CrRenderGraphLog("------------------------------------");
	CrRenderGraphLog("Ending Render Graph For Frame %ld", m_frameParams.frameIndex);
	CrRenderGraphLog("------------------------------------");
}
