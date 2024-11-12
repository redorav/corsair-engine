#include "Rendering/CrRendering_pch.h"

#include "Rendering/CrRenderGraph.h"
#include "Rendering/ICrTexture.h"
#include "Rendering/ICrCommandBuffer.h"
#include "Rendering/CrGPUTimingQueryTracker.h"

#include "Core/Logging/ICrDebug.h"

//#define RENDER_GRAPH_LOGS

#if defined(RENDER_GRAPH_LOGS)
#define CrRenderGraphLog2(format, ...) CrLog(format, __VA_ARGS__)
#else
#define CrRenderGraphLog2(format, ...)
#endif

void CrRenderGraph::AddRenderPass
(
	const CrRenderGraphString& name, const float4& color, CrRenderGraphPassType::T type, 
	const CrRenderGraphSetupFunction& setupFunction, const CrRenderGraphExecutionFunction& executionFunction
)
{
	CrRenderGraphPass2& workingPass = m_workingPasses.push_back();
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

void CrRenderGraph::BindTexture
(
	Textures::T textureIndex, ICrTexture* texture, cr3d::ShaderStageFlags::T shaderStages)
{
	CrRenderGraphPass2& workingPass = GetWorkingRenderPass();

	CrHash subresourceHash;
	subresourceHash << (uintptr_t)texture;

	CrRenderGraphTextureUsage2 textureUsage;
	textureUsage.texture = texture;
	textureUsage.mipmapStart = 0;
	textureUsage.mipmapCount = texture->GetMipmapCount();
	textureUsage.sliceStart = 0;
	textureUsage.sliceCount = texture->GetSliceCount();
	textureUsage.textureIndex = textureIndex;
	textureUsage.state = cr3d::TextureState(cr3d::TextureLayout::ShaderInput, shaderStages);
	textureUsage.subresourceId = GetSubresourceId(subresourceHash);
	workingPass.textureUsages.push_back(textureUsage);

	CrRenderGraphLog2("Added Texture %s", texture->GetDebugName());
}

void CrRenderGraph::BindRWTexture(RWTextures::T rwTextureIndex, ICrTexture* texture, cr3d::ShaderStageFlags::T shaderStages, uint32_t mipmap, uint32_t sliceStart, uint32_t sliceCount)
{
	CrRenderGraphPass2& workingPass = GetWorkingRenderPass();

	CrHash subresourceHash;
	subresourceHash << (uintptr_t)texture;

	CrRenderGraphTextureUsage2 textureUsage;
	textureUsage.texture = texture;
	textureUsage.mipmapStart = mipmap;
	textureUsage.mipmapCount = 1;
	unused_parameter(sliceStart);
	unused_parameter(sliceCount);
	textureUsage.sliceStart = 0;
	textureUsage.sliceCount = texture->GetSliceCount();
	textureUsage.rwTextureIndex = rwTextureIndex;
	textureUsage.state = cr3d::TextureState(cr3d::TextureLayout::RWTexture, shaderStages);
	textureUsage.subresourceId = GetSubresourceId(subresourceHash);
	workingPass.textureUsages.push_back(textureUsage);

	CrRenderGraphLog2("Added RWTexture %s", texture->GetDebugName());
}

void CrRenderGraph::BindRenderTarget
(
	ICrTexture* texture,
	CrRenderTargetLoadOp loadOp,
	CrRenderTargetStoreOp storeOp,
	float4 clearColor,
	uint32_t mipmap, uint32_t slice
)
{
	CrRenderGraphPass2& workingPass = GetWorkingRenderPass();

	CrHash subresourceHash;
	subresourceHash << (uintptr_t)texture;

	CrRenderGraphTextureUsage2 textureUsage;
	textureUsage.texture = texture;
	textureUsage.mipmapStart = mipmap;
	textureUsage.mipmapCount = 1;
	textureUsage.sliceStart = slice;
	textureUsage.sliceCount = 1;
	textureUsage.clearColor = clearColor;
	textureUsage.storeOp = storeOp;
	textureUsage.loadOp = loadOp;
	textureUsage.state = cr3d::TextureState(cr3d::TextureLayout::RenderTarget, cr3d::ShaderStageFlags::Pixel);
	textureUsage.subresourceId = GetSubresourceId(subresourceHash);
	workingPass.textureUsages.push_back(textureUsage);

	CrRenderGraphLog2("Added Render Target %s", texture->GetDebugName());
}

void CrRenderGraph::BindDepthStencilTarget
(
	ICrTexture* texture,
	CrRenderTargetLoadOp loadOp,
	CrRenderTargetStoreOp storeOp,
	float depthClearValue,
	CrRenderTargetLoadOp stencilLoadOp,
	CrRenderTargetStoreOp stencilStoreOp,
	uint8_t stencilClearValue,
	uint32_t mipmap, uint32_t slice,
	bool readOnlyDepth, bool readOnlyStencil
)
{
	CrRenderGraphPass2& workingPass = GetWorkingRenderPass();

	CrAssertMsg(workingPass.type == CrRenderGraphPassType::Graphics, "Render pass must be graphics");
	CrAssertMsg(workingPass.depthTexture == nullptr, "Cannot bind multiple depth targets");

	CrHash subresourceHash;
	subresourceHash << (uintptr_t)texture;

	CrRenderGraphTextureUsage2 textureUsage;
	textureUsage.texture = texture;
	textureUsage.mipmapStart = mipmap;
	textureUsage.mipmapCount = 1;
	textureUsage.sliceStart = slice;
	textureUsage.sliceCount = 1;
	textureUsage.depthClearValue = depthClearValue;
	textureUsage.stencilClearValue = stencilClearValue;
	textureUsage.stencilLoadOp = stencilLoadOp;
	textureUsage.stencilStoreOp = stencilStoreOp;
	textureUsage.storeOp = storeOp;
	textureUsage.loadOp = loadOp;
	textureUsage.subresourceId = GetSubresourceId(subresourceHash);

	// If we don't care about either the inputs or the outputs, we can conclude that nothing meaningful is going to be written to it
	bool writeDepth = loadOp == CrRenderTargetLoadOp::Clear || storeOp != CrRenderTargetStoreOp::DontCare;
	bool writeStencil = stencilLoadOp == CrRenderTargetLoadOp::Clear || stencilStoreOp != CrRenderTargetStoreOp::DontCare;

	CrAssertMsg(!(writeDepth && readOnlyDepth), "Cannot read and write to depth simultaneously");
	CrAssertMsg(!(writeStencil && readOnlyStencil), "Cannot read and write to stencil simultaneously");

	// Read-only depth specifies that we want to prevent writing to the selected aspect of the render target
	// If read-only is set to false on both aspects, it just means that we don't want to prevent writing
	// We can still prevent reading if we detect that 

	// If we're not loading or clearing the depth or stencil, we know we won't test it
	bool readDepth = loadOp != CrRenderTargetLoadOp::DontCare;
	bool readStencil = stencilLoadOp != CrRenderTargetLoadOp::DontCare;

	cr3d::TextureLayout::T layout = cr3d::TextureLayout::Count;

	if (readOnlyDepth && writeStencil)
	{
		layout = cr3d::TextureLayout::StencilWriteDepthReadOnly;
	}
	else if (readOnlyStencil && writeDepth)
	{
		layout = cr3d::TextureLayout::DepthWriteStencilReadOnly;
	}
	else if (readOnlyDepth && readOnlyStencil)
	{
		layout = cr3d::TextureLayout::DepthStencilReadOnly;
	}
	else
	{
		if (writeDepth || writeStencil)
		{
			if (readDepth || readStencil)
			{
				layout = cr3d::TextureLayout::DepthStencilReadWrite;
			}
			else
			{
				layout = cr3d::TextureLayout::DepthStencilWrite;
			}
		}
		else
		{
			layout = cr3d::TextureLayout::DepthStencilReadOnly;
		}
	}

	CrAssertMsg(layout != cr3d::TextureLayout::Count, "Invalid layout selected");

	textureUsage.state = { layout, cr3d::ShaderStageFlags::Pixel };

	workingPass.depthTexture = texture;

	workingPass.textureUsages.push_back(textureUsage);

	CrRenderGraphLog2("Added Depth Stencil Target %s", texture->GetDebugName());
}

void CrRenderGraph::BindSwapchain(ICrTexture* texture, uint32_t mipmap, uint32_t slice)
{
	CrRenderGraphPass2& workingPass = GetWorkingRenderPass();

	CrHash subresourceHash;
	subresourceHash << (uintptr_t)texture;

	CrRenderGraphTextureUsage2 textureUsage;
	textureUsage.texture = texture;
	textureUsage.mipmapStart = mipmap;
	textureUsage.sliceStart = slice;
	textureUsage.state = { cr3d::TextureLayout::Present, cr3d::ShaderStageFlags::Present };
	textureUsage.subresourceId = GetSubresourceId(subresourceHash);
	workingPass.textureUsages.push_back(textureUsage);

	CrRenderGraphLog2("Added Swapchain %s", texture->GetDebugName());
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

void CrRenderGraph::BindStorageBuffer(StorageBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, cr3d::ShaderStageFlags::T shaderStages, uint32_t numElements, uint32_t stride, uint32_t offset)
{
	CrRenderGraphPass2& workingPass = GetWorkingRenderPass();

	CrHash bufferHash;
	bufferHash << (uintptr_t)buffer;

	CrRenderGraphBufferUsage2 bufferUsage;
	bufferUsage.buffer = buffer;
	bufferUsage.usageState = cr3d::BufferState::ShaderInput;
	bufferUsage.shaderStages = shaderStages;
	bufferUsage.storageBufferIndex = bufferIndex;
	bufferUsage.resourceType = cr3d::ShaderResourceType::StorageBuffer;
	bufferUsage.bufferId = GetUniqueBufferId(bufferHash);
	workingPass.bufferUsages.push_back(bufferUsage);

	unused_parameter(numElements);
	unused_parameter(stride);
	unused_parameter(offset);
}

void CrRenderGraph::BindStorageBuffer(StorageBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, cr3d::ShaderStageFlags::T shaderStages)
{
	BindStorageBuffer(bufferIndex, buffer, shaderStages, buffer->GetNumElements(), buffer->GetStrideBytes(), 0);
}

void CrRenderGraph::BindRWStorageBuffer(RWStorageBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, cr3d::ShaderStageFlags::T shaderStages, uint32_t numElements, uint32_t stride, uint32_t offset)
{
	CrRenderGraphPass2& workingPass = GetWorkingRenderPass();

	CrHash bufferHash;
	bufferHash << (uintptr_t)buffer;

	CrRenderGraphBufferUsage2 bufferUsage;
	bufferUsage.buffer = buffer;
	bufferUsage.usageState = cr3d::BufferState::ReadWrite;
	bufferUsage.shaderStages = shaderStages;
	bufferUsage.rwStorageBufferIndex = bufferIndex;
	bufferUsage.resourceType = cr3d::ShaderResourceType::RWStorageBuffer;
	bufferUsage.bufferId = GetUniqueBufferId(bufferHash);
	workingPass.bufferUsages.push_back(bufferUsage);

	unused_parameter(numElements);
	unused_parameter(stride);
	unused_parameter(offset);
}

void CrRenderGraph::BindRWStorageBuffer(RWStorageBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, cr3d::ShaderStageFlags::T shaderStages)
{
	BindRWStorageBuffer(bufferIndex, buffer, shaderStages, buffer->GetNumElements(), buffer->GetStrideBytes(), 0);
}

void CrRenderGraph::BindTypedBuffer(TypedBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, cr3d::ShaderStageFlags::T shaderStages, uint32_t numElements, uint32_t stride, uint32_t offset)
{
	CrRenderGraphPass2& workingPass = GetWorkingRenderPass();

	CrHash bufferHash;
	bufferHash << (uintptr_t)buffer;

	CrRenderGraphBufferUsage2 bufferUsage;
	bufferUsage.buffer = buffer;
	bufferUsage.usageState = cr3d::BufferState::ShaderInput;
	bufferUsage.shaderStages = shaderStages;
	bufferUsage.typedBufferIndex = bufferIndex;
	bufferUsage.resourceType = cr3d::ShaderResourceType::TypedBuffer;
	bufferUsage.bufferId = GetUniqueBufferId(bufferHash);
	workingPass.bufferUsages.push_back(bufferUsage);

	unused_parameter(numElements);
	unused_parameter(stride);
	unused_parameter(offset);
}

void CrRenderGraph::BindTypedBuffer(TypedBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, cr3d::ShaderStageFlags::T shaderStages)
{
	BindTypedBuffer(bufferIndex, buffer, shaderStages, buffer->GetNumElements(), buffer->GetStrideBytes(), 0);
}

void CrRenderGraph::BindRWTypedBuffer(RWTypedBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, cr3d::ShaderStageFlags::T shaderStages, uint32_t numElements, uint32_t stride, uint32_t offset)
{
	CrRenderGraphPass2& workingPass = GetWorkingRenderPass();

	CrHash bufferHash;
	bufferHash << (uintptr_t)buffer;

	CrRenderGraphBufferUsage2 bufferUsage;
	bufferUsage.buffer = buffer;
	bufferUsage.usageState = cr3d::BufferState::ReadWrite;
	bufferUsage.shaderStages = shaderStages;
	bufferUsage.rwTypedBufferIndex = bufferIndex;
	bufferUsage.resourceType = cr3d::ShaderResourceType::TypedBuffer;
	bufferUsage.bufferId = GetUniqueBufferId(bufferHash);
	workingPass.bufferUsages.push_back(bufferUsage);

	unused_parameter(numElements);
	unused_parameter(stride);
	unused_parameter(offset);
}

void CrRenderGraph::BindRWTypedBuffer(RWTypedBuffers::T bufferIndex, const ICrHardwareGPUBuffer* buffer, cr3d::ShaderStageFlags::T shaderStages)
{
	BindRWTypedBuffer(bufferIndex, buffer, shaderStages, buffer->GetNumElements(), buffer->GetStrideBytes(), 0);
}

void CrRenderGraph::Begin(const CrRenderGraphFrameParams& frameParams)
{
	m_frameParams = frameParams;

	m_workingPassIndex = 0;
	m_subresourceIdCounter = 0;
	m_bufferIdCounter = 0;

	CrRenderGraphLog2("Beginning render pass for frame %ld", m_frameParams.frameIndex);
}

void CrRenderGraph::Execute()
{
	m_textureLastUsedPass.clear();
	m_textureLastUsedPass.resize(m_textureSubresourceIds.size(), nullptr);
	m_bufferLastUsedPass.clear();
	m_bufferLastUsedPass.resize(m_textureSubresourceIds.size(), nullptr);

	for (size_t renderGraphPassIndex = 0; renderGraphPassIndex < m_workingPasses.size(); ++renderGraphPassIndex)
	{
		CrRenderGraphPass2* renderGraphPass = &m_workingPasses[renderGraphPassIndex];

		// Process textures within a pass
		for (uint32_t textureIndex = 0; textureIndex < renderGraphPass->textureUsages.size(); ++textureIndex)
		{
			const CrRenderGraphTextureUsage2& textureUsage = renderGraphPass->textureUsages[textureIndex];

			CrRenderGraphTextureTransitionInfo2 transitionInfo;
			transitionInfo.usageState = textureUsage.state;

			// Initialize final state to default state until we have more information
			// TODO Track default state in the render device so that we don't always have to transition to and from the same state
			// Rename to initial state as it will probably only be used then to initialize it for the first time
			transitionInfo.finalState = textureUsage.texture->GetDefaultState();

			// Figure out what pass this subresource was last used in
			// TODO Iterate over all subresources
			CrRenderGraphPass2* lastUsedRenderPass = m_textureLastUsedPass[textureUsage.subresourceId];

			if (lastUsedRenderPass)
			{
				// By definition the pass that references it has a transition info set up
				CrRenderGraphTextureTransitionInfo2& lastUsedTransitionInfo = lastUsedRenderPass->textureTransitionInfos.find(textureUsage.subresourceId)->second;
				lastUsedTransitionInfo.finalState = textureUsage.state;
				transitionInfo.initialState = textureUsage.state;
			}
			else
			{
				// If no previous pass referenced this subresource, set the initial state
				// TODO As mentioned before track default state in the render device so that we don't always have to transition to and from the same state
				transitionInfo.initialState = textureUsage.texture->GetDefaultState();
			}

			renderGraphPass->textureTransitionInfos.insert(textureUsage.subresourceId, transitionInfo);

			m_textureLastUsedPass[textureUsage.subresourceId] = renderGraphPass;
		}

		// Process buffers within a pass
		for (uint32_t bufferIndex = 0; bufferIndex < renderGraphPass->bufferUsages.size(); ++bufferIndex)
		{
			const CrRenderGraphBufferUsage2& bufferUsage = renderGraphPass->bufferUsages[bufferIndex];

			CrRenderGraphBufferTransitionInfo2 transitionInfo;
			transitionInfo.usageState = bufferUsage.usageState;
			transitionInfo.finalState = bufferUsage.usageState; // Initialize final state to current state until we have more information
			transitionInfo.usageShaderStages = bufferUsage.shaderStages;
			transitionInfo.finalShaderStages = bufferUsage.shaderStages;

			CrRenderGraphPass2* lastUsedRenderPass = m_bufferLastUsedPass[bufferUsage.bufferId];

			if (lastUsedRenderPass)
			{
				// Inject the final and initial states with usage states
				CrRenderGraphBufferTransitionInfo2& lastUsedTransitionInfo = lastUsedRenderPass->bufferTransitionInfos.find(bufferUsage.bufferId)->second;
				lastUsedTransitionInfo.finalState = bufferUsage.usageState;
				lastUsedTransitionInfo.finalShaderStages = bufferUsage.shaderStages;

				transitionInfo.initialState = lastUsedTransitionInfo.finalState;
				transitionInfo.initialShaderStages = lastUsedTransitionInfo.finalShaderStages;
			}
			// If we didn't find the resource it means we're the first to access it.
			// In normal circumstances this could be an error (i.e. we access a resource nobody has populated)
			else
			{
				transitionInfo.initialState = cr3d::BufferState::Undefined;
				transitionInfo.initialShaderStages = renderGraphPass->type == CrRenderGraphPassType::Compute ? cr3d::ShaderStageFlags::Compute : cr3d::ShaderStageFlags::Graphics;
			}

			renderGraphPass->bufferTransitionInfos.insert(bufferUsage.bufferId, transitionInfo);

			m_bufferLastUsedPass[bufferUsage.bufferId] = renderGraphPass;
		}
	}

	for (size_t renderGraphPassIndex = 0; renderGraphPassIndex < m_workingPasses.size(); ++renderGraphPassIndex)
	{
		const CrRenderGraphPass2& renderGraphPass = m_workingPasses[renderGraphPassIndex];

		CrRenderGraphLog2("Executing Render Pass %s", renderGraphPass.name.c_str());

		if (renderGraphPass.type != CrRenderGraphPassType::Behavior)
		{
			CrRenderPassDescriptor renderPassDescriptor;
			renderPassDescriptor.debugName = renderGraphPass.name;
			renderPassDescriptor.debugColor = renderGraphPass.color;

			if (renderGraphPass.type == CrRenderGraphPassType::Graphics)
			{
				renderPassDescriptor.type = cr3d::RenderPassType::Graphics;
			}
			else if (renderGraphPass.type == CrRenderGraphPassType::Compute)
			{
				renderPassDescriptor.type = cr3d::RenderPassType::Compute;
			}

			for (uint32_t i = 0; i < renderGraphPass.textureUsages.size(); ++i)
			{
				const CrRenderGraphTextureUsage2& textureUsage = renderGraphPass.textureUsages[i];
				const CrRenderGraphTextureTransitionInfo2& transitionInfo = renderGraphPass.textureTransitionInfos.find(textureUsage.subresourceId)->second;

				switch (textureUsage.state.layout)
				{
					case cr3d::TextureLayout::RenderTarget:
					{
						CrRenderTargetDescriptor renderTargetDescriptor;
						renderTargetDescriptor.texture      = textureUsage.texture;
						renderTargetDescriptor.mipmap       = textureUsage.mipmapStart;
						renderTargetDescriptor.slice        = textureUsage.sliceStart;
						renderTargetDescriptor.clearColor   = textureUsage.clearColor;
						renderTargetDescriptor.loadOp       = textureUsage.loadOp;
						renderTargetDescriptor.storeOp      = textureUsage.storeOp;
						renderTargetDescriptor.initialState = transitionInfo.initialState;
						renderTargetDescriptor.usageState   = transitionInfo.usageState;
						renderTargetDescriptor.finalState   = transitionInfo.finalState;

						CrRenderGraphLog2("  Render Target %s [%s -> %s -> %s]",
							textureUsage.texture->GetDebugName(),
							cr3d::TextureLayout::ToString(renderTargetDescriptor.initialState.layout),
							cr3d::TextureLayout::ToString(renderTargetDescriptor.usageState.layout),
							cr3d::TextureLayout::ToString(renderTargetDescriptor.finalState.layout));

						renderPassDescriptor.color.push_back(renderTargetDescriptor);
						break;
					}
					case cr3d::TextureLayout::DepthStencilReadWrite:
					case cr3d::TextureLayout::DepthStencilWrite:
					case cr3d::TextureLayout::StencilWriteDepthReadOnly:
					case cr3d::TextureLayout::DepthWriteStencilReadOnly:
					case cr3d::TextureLayout::DepthStencilReadOnly:
					case cr3d::TextureLayout::DepthStencilReadOnlyShader:
					{
						CrRenderTargetDescriptor depthDescriptor;
						depthDescriptor.texture           = textureUsage.texture;
						depthDescriptor.mipmap            = textureUsage.mipmapStart;
						depthDescriptor.slice             = textureUsage.sliceStart;
						depthDescriptor.depthClearValue   = textureUsage.depthClearValue;
						depthDescriptor.stencilClearValue = textureUsage.stencilClearValue;
						depthDescriptor.loadOp            = textureUsage.loadOp;
						depthDescriptor.storeOp           = textureUsage.storeOp;
						depthDescriptor.stencilLoadOp     = textureUsage.stencilLoadOp;
						depthDescriptor.stencilStoreOp    = textureUsage.stencilStoreOp;
						depthDescriptor.initialState      = transitionInfo.initialState;
						depthDescriptor.usageState        = transitionInfo.usageState;
						depthDescriptor.finalState        = transitionInfo.finalState;

						CrRenderGraphLog2("  Depth Stencil %s [%s -> %s -> %s]", textureUsage.texture->GetDebugName(),
							cr3d::TextureLayout::ToString(depthDescriptor.initialState.layout),
							cr3d::TextureLayout::ToString(depthDescriptor.usageState.layout),
							cr3d::TextureLayout::ToString(depthDescriptor.finalState.layout));

						renderPassDescriptor.depth = depthDescriptor;
						break;
					}
					case cr3d::TextureLayout::RWTexture:
					case cr3d::TextureLayout::ShaderInput:
					{
						if (transitionInfo.initialState != transitionInfo.usageState)
						{
							renderPassDescriptor.beginTextures.emplace_back
							(
								textureUsage.texture, textureUsage.mipmapStart, textureUsage.mipmapCount,
								textureUsage.sliceStart, textureUsage.sliceCount, textureUsage.texturePlane,
								transitionInfo.initialState, transitionInfo.usageState
							);

							CrRenderGraphLog2("  Texture %s [%s -> %s]", textureUsage.texture->GetDebugName(),
								cr3d::TextureLayout::ToString(transitionInfo.initialState.layout),
								cr3d::TextureLayout::ToString(transitionInfo.usageState.layout));
						}

						if (transitionInfo.usageState != transitionInfo.finalState)
						{
							renderPassDescriptor.endTextures.emplace_back
							(
								textureUsage.texture, textureUsage.mipmapStart, textureUsage.mipmapCount,
								textureUsage.sliceStart, textureUsage.sliceCount, textureUsage.texturePlane,
								transitionInfo.usageState, transitionInfo.finalState
							);

							CrRenderGraphLog2("  Texture %s [%s -> %s]", textureUsage.texture->GetDebugName(),
								cr3d::TextureLayout::ToString(transitionInfo.usageState.layout),
								cr3d::TextureLayout::ToString(transitionInfo.finalState.layout));
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
					case cr3d::TextureLayout::RWTexture:
						m_frameParams.commandBuffer->BindRWTexture(textureUsage.rwTextureIndex, textureUsage.texture, textureUsage.mipmapStart);
						break;
					case cr3d::TextureLayout::ShaderInput:
						m_frameParams.commandBuffer->BindTexture(textureUsage.textureIndex, textureUsage.texture, textureUsage.texturePlane);
						break;
				}
			}

			for (uint32_t i = 0; i < renderGraphPass.bufferUsages.size(); ++i)
			{
				const CrRenderGraphBufferUsage2& bufferUsage = renderGraphPass.bufferUsages[i];
				const CrRenderGraphBufferTransitionInfo2& transitionInfo = renderGraphPass.bufferTransitionInfos.find(bufferUsage.bufferId)->second;
					
				if (transitionInfo.initialState != transitionInfo.usageState)
				{
					renderPassDescriptor.beginBuffers.emplace_back(bufferUsage.buffer, bufferUsage.size, bufferUsage.offset,
						transitionInfo.initialState, transitionInfo.initialShaderStages,
						transitionInfo.usageState, transitionInfo.usageShaderStages);

					CrRenderGraphLog2("  Buffer %s [%s -> %s]", bufferUsage.buffer->GetDebugName(),
						cr3d::BufferState::ToString(transitionInfo.initialState),
						cr3d::BufferState::ToString(transitionInfo.usageState));
				}

				if (transitionInfo.usageState != transitionInfo.finalState)
				{
					renderPassDescriptor.endBuffers.emplace_back(bufferUsage.buffer, bufferUsage.size, bufferUsage.offset,
						transitionInfo.usageState, transitionInfo.usageShaderStages,
						transitionInfo.finalState, transitionInfo.finalShaderStages);

					CrRenderGraphLog2("  Buffer %s [%s -> %s]", bufferUsage.buffer->GetDebugName(),
						cr3d::BufferState::ToString(transitionInfo.usageState),
						cr3d::BufferState::ToString(transitionInfo.finalState));
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
}
