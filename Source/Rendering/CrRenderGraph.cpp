#include "CrRendering_pch.h"

#include "CrRenderGraph.h"
#include "Rendering/ICrCommandBuffer.h"
#include "Rendering/ICrGPUQueryPool.h"
#include "Rendering/CrGPUTimingQueryTracker.h"

#include "Core/Logging/ICrDebug.h"

CrRenderGraph::CrRenderGraph()
{
	End();
}

void CrRenderGraph::AddRenderPass
(
	const CrRenderGraphString& name, const float4& color, CrRenderGraphPassType::T type, 
	const CrRenderGraphSetupFunction& setupFunction, const CrRenderGraphExecutionFunction& executionFunction
)
{
	const auto& passIter = m_nameToLogicalPassId.find(name);

	if (passIter == m_nameToLogicalPassId.end())
	{
		m_logicalPasses.emplace_back(name, color, type, executionFunction);
		m_workingPassId = m_uniquePassId;
		setupFunction(*this);
		m_workingPassId = CrRenderPassId(0xffffffff);
		m_nameToLogicalPassId.insert({ name, m_uniquePassId });
		m_uniquePassId.id++;
	}
	else
	{
		CrLog("Duplicated pass detected");
	}
}

CrRenderGraphTextureId CrRenderGraph::CreateTexture(const CrRenderGraphString& name, const CrRenderGraphTextureDescriptor& descriptor)
{
	const auto& iter = m_nameToTextureId.find(name);

	if (iter == m_nameToTextureId.end())
	{
		CrRenderGraphTextureId textureId(m_uniqueTextureId);
		m_textureResources.emplace_back(name, m_uniqueTextureId, descriptor);
		m_nameToTextureId.insert({ name, textureId });
		m_textureLastUsedPass.emplace_back();
		m_uniqueTextureId++;
		return textureId;
	}
	else
	{
		return iter->second;
	}
}

CrRenderGraphBufferId CrRenderGraph::CreateBuffer(const CrRenderGraphString& name, const CrRenderGraphBufferDescriptor& descriptor)
{
	const auto& iter = m_nameToBufferId.find(name);

	if (iter == m_nameToBufferId.end())
	{
		CrRenderGraphBufferId bufferId(m_uniqueBufferId);
		m_bufferResources.emplace_back(name, m_uniqueBufferId, descriptor);
		m_nameToBufferId.insert({ name, bufferId });
		m_bufferLastUsedPass.emplace_back();
		m_uniqueBufferId++;
		return bufferId;
	}
	else
	{
		return iter->second;
	}
}

void CrRenderGraph::AddTexture(CrRenderGraphTextureId textureId, cr3d::ShaderStageFlags::T shaderStages)
{
	if (textureId != CrRenderGraphTextureId())
	{
		CrRenderGraphTextureResource* textureResource = &m_textureResources[textureId.id];

		CrRenderGraphTextureUsage textureUsage;
		textureUsage.textureId = textureId;
		textureUsage.mipmapStart = 0;
		textureUsage.mipmapCount = textureResource->descriptor.mipmapCount;
		textureUsage.sliceStart = 0;
		textureUsage.sliceCount = textureResource->descriptor.sliceCount;
		textureUsage.state = cr3d::TextureState::ShaderInput;
		textureUsage.shaderStages = shaderStages;
		m_logicalPasses[m_workingPassId.id].textureUsages.push_back(textureUsage);
	}
}

void CrRenderGraph::AddRWTexture(CrRenderGraphTextureId textureId, cr3d::ShaderStageFlags::T shaderStages, uint32_t mipmapStart, uint32_t mipmapCount, uint32_t sliceStart, uint32_t sliceCount)
{
	if (textureId != CrRenderGraphTextureId())
	{
		CrRenderGraphTextureResource* textureResource = &m_textureResources[textureId.id];

		CrRenderGraphTextureUsage textureUsage;
		textureUsage.textureId = textureId;
		textureUsage.mipmapStart = CrMin(mipmapStart, textureResource->descriptor.mipmapCount);
		textureUsage.mipmapCount = CrMin(mipmapCount, textureResource->descriptor.mipmapCount);
		textureUsage.sliceStart = CrMin(sliceStart, textureResource->descriptor.sliceCount);
		textureUsage.sliceCount = CrMin(sliceCount, textureResource->descriptor.sliceCount);
		textureUsage.state = cr3d::TextureState::RWTexture;
		textureUsage.shaderStages = shaderStages;
		m_logicalPasses[m_workingPassId.id].textureUsages.push_back(textureUsage);
	}
}

void CrRenderGraph::AddRenderTarget(CrRenderGraphTextureId textureId, CrRenderTargetLoadOp loadOp, CrRenderTargetStoreOp storeOp, float4 clearColor, uint32_t mipmap, uint32_t slice)
{
	if (textureId != CrRenderGraphTextureId())
	{
		CrRenderGraphTextureUsage textureUsage;
		textureUsage.textureId = textureId;
		textureUsage.mipmapStart = mipmap;
		textureUsage.sliceStart = slice;
		textureUsage.clearColor = clearColor;
		textureUsage.loadOp = loadOp;
		textureUsage.storeOp = storeOp;
		textureUsage.state = cr3d::TextureState::RenderTarget;
		m_logicalPasses[m_workingPassId.id].textureUsages.push_back(textureUsage);
	}
}

void CrRenderGraph::AddDepthStencilTarget
(
	CrRenderGraphTextureId textureId, 
	CrRenderTargetLoadOp loadOp, CrRenderTargetStoreOp storeOp, float depthClearValue, 
	CrRenderTargetLoadOp stencilLoadOp, CrRenderTargetStoreOp stencilStoreOp, uint8_t stencilClearValue,
	uint32_t mipmap, uint32_t slice
)
{
	if (textureId != CrRenderGraphTextureId())
	{
		CrRenderGraphTextureUsage textureUsage;
		textureUsage.textureId = textureId;
		textureUsage.mipmapStart = mipmap;
		textureUsage.sliceStart = slice;
		textureUsage.depthClearValue = depthClearValue;
		textureUsage.stencilClearValue = stencilClearValue;
		textureUsage.loadOp = loadOp;
		textureUsage.storeOp = storeOp;
		textureUsage.stencilLoadOp = stencilLoadOp;
		textureUsage.stencilStoreOp = stencilStoreOp;
		textureUsage.state = cr3d::TextureState::DepthStencilWrite;
		m_logicalPasses[m_workingPassId.id].textureUsages.push_back(textureUsage);
	}
}

void CrRenderGraph::AddSwapchain(CrRenderGraphTextureId textureId)
{
	if (textureId != CrRenderGraphTextureId())
	{
		CrRenderGraphTextureUsage textureUsage;
		textureUsage.textureId = textureId;
		textureUsage.state = cr3d::TextureState::Present;
		m_logicalPasses[m_workingPassId.id].textureUsages.push_back(textureUsage);
	}
}

void CrRenderGraph::AddBuffer(CrRenderGraphBufferId bufferId, cr3d::ShaderStageFlags::T shaderStages)
{
	if (bufferId != CrRenderGraphBufferId())
	{
		CrRenderGraphBufferUsage bufferUsage;
		bufferUsage.bufferId = bufferId;
		bufferUsage.usageState = cr3d::BufferState::ShaderInput;
		bufferUsage.shaderStages = shaderStages;
		m_logicalPasses[m_workingPassId.id].bufferUsages.push_back(bufferUsage);
	}
}

void CrRenderGraph::AddRWBuffer(CrRenderGraphBufferId bufferId, cr3d::ShaderStageFlags::T shaderStages)
{
	if (bufferId != CrRenderGraphBufferId())
	{
		CrRenderGraphBufferUsage bufferUsage;
		bufferUsage.bufferId = bufferId;
		bufferUsage.usageState = cr3d::BufferState::ReadWrite;
		bufferUsage.shaderStages = shaderStages;
		m_logicalPasses[m_workingPassId.id].bufferUsages.push_back(bufferUsage);
	}
}

void CrRenderGraph::Execute()
{
	// Process all passes
	for (CrRenderPassId logicalPassId(0); logicalPassId < CrRenderPassId((uint32_t)m_logicalPasses.size()); ++logicalPassId)
	{
		CrRenderGraphPass* renderGraphPass = &m_logicalPasses[logicalPassId.id];

		// Process textures within a pass
		for (uint32_t textureIndex = 0; textureIndex < renderGraphPass->textureUsages.size(); ++textureIndex)
		{
			const CrRenderGraphTextureUsage& textureUsage = renderGraphPass->textureUsages[textureIndex];
			const CrRenderGraphTextureResource* texture = &m_textureResources[textureUsage.textureId.id];

			CrRenderGraphTextureTransition transitionInfo;
			transitionInfo.name = texture->name;
			transitionInfo.usageState = textureUsage.state;
			transitionInfo.finalState = textureUsage.state; // Initialize final state to current state until we have more information
			transitionInfo.usageShaderStages = textureUsage.shaderStages;
			transitionInfo.finalShaderStages = textureUsage.shaderStages;

			CrRenderPassId lastUsedPassId = m_textureLastUsedPass[texture->id.id];

			// If we found the resource, someone must have accessed it before us
			// We therefore populate the previous resource state of this physical
			// pass and the final resource state of the previous pass
			if (lastUsedPassId != CrRenderPassId())
			{
				CrRenderGraphPass* lastUsedPass = &m_logicalPasses[lastUsedPassId.id];

				// Inject the final and initial states with usage states
				CrRenderGraphTextureTransition& lastUsedTransitionInfo = lastUsedPass->textureTransitions.find(texture->id.id)->second;
				lastUsedTransitionInfo.finalState = textureUsage.state;
				lastUsedTransitionInfo.finalShaderStages = textureUsage.shaderStages;

				transitionInfo.initialState = lastUsedTransitionInfo.usageState;
				transitionInfo.initialShaderStages = lastUsedTransitionInfo.usageShaderStages;
			}
			// If we didn't find the resource it means we're the first to access it
			// In normal circumstances this could be an error (i.e. we access a
			// resource nobody has populated)
			else
			{
				transitionInfo.initialState = cr3d::TextureState::Undefined;
				transitionInfo.initialShaderStages = cr3d::ShaderStageFlags::None;
			}

			renderGraphPass->textureTransitions.insert({ texture->id.id, transitionInfo });

			m_textureLastUsedPass[texture->id.id] = logicalPassId;
		}

		// Process buffers within a pass
		for (uint32_t bufferIndex = 0; bufferIndex < renderGraphPass->bufferUsages.size(); ++bufferIndex)
		{
			const CrRenderGraphBufferUsage& bufferUsage = renderGraphPass->bufferUsages[bufferIndex];
			const CrRenderGraphBufferResource* buffer = &m_bufferResources[bufferUsage.bufferId.id];

			CrRenderGraphBufferTransition transitionInfo;
			transitionInfo.name = buffer->name;
			transitionInfo.usageState = bufferUsage.usageState;
			transitionInfo.finalState = bufferUsage.usageState; // Initialize final state to current state until we have more information

			CrRenderPassId lastUsedPassId = m_bufferLastUsedPass[buffer->id.id];

			// If we found the resource, someone must have accessed it before us
			// We therefore populate the previous resource state of this physical
			// pass and the final resource state of the previous pass
			if (lastUsedPassId != CrRenderPassId())
			{
				CrRenderGraphPass* lastUsedPass = &m_logicalPasses[lastUsedPassId.id];

				// Inject the final and initial states with usage states
				CrRenderGraphBufferTransition& lastUsedTransitionInfo = lastUsedPass->bufferTransitions.find(buffer->id.id)->second;
				lastUsedTransitionInfo.finalState = bufferUsage.usageState;
				transitionInfo.initialState = lastUsedTransitionInfo.usageState;
			}
			// If we didn't find the resource it means we're the first to access it
			// In normal circumstances this could be an error (i.e. we access a
			// resource nobody has populated)
			else
			{
				transitionInfo.initialState = cr3d::BufferState::Undefined;
			}

			renderGraphPass->bufferTransitions.insert({ buffer->id.id, transitionInfo });

			m_bufferLastUsedPass[buffer->id.id] = logicalPassId;
		}
	}

	// TODO Cull and reorder passes

	for (CrRenderPassId logicalPassId(0); logicalPassId < CrRenderPassId((uint32_t)m_logicalPasses.size()); ++logicalPassId)
	{
		const CrRenderGraphPass& renderGraphPass = m_logicalPasses[logicalPassId.id];

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
				const CrRenderGraphTextureUsage& textureUsage = renderGraphPass.textureUsages[i];
				const CrRenderGraphTextureId textureId = textureUsage.textureId;
				const CrRenderGraphTextureTransition& transitionInfo = renderGraphPass.textureTransitions.find(textureId.id)->second;

				switch (textureUsage.state)
				{
					case cr3d::TextureState::RenderTarget:
					{
						CrRenderTargetDescriptor renderTargetDescriptor;
						renderTargetDescriptor.texture      = m_textureResources[textureId.id].descriptor.texture;
						renderTargetDescriptor.mipmap       = textureUsage.mipmapStart;
						renderTargetDescriptor.slice        = textureUsage.sliceStart;
						renderTargetDescriptor.clearColor   = textureUsage.clearColor;
						renderTargetDescriptor.loadOp       = textureUsage.loadOp;
						renderTargetDescriptor.storeOp      = textureUsage.storeOp;
						renderTargetDescriptor.initialState = transitionInfo.initialState;
						renderTargetDescriptor.usageState   = transitionInfo.usageState;
						renderTargetDescriptor.finalState   = transitionInfo.finalState;

						renderPassDescriptor.color.push_back(renderTargetDescriptor);
						break;
					}
					case cr3d::TextureState::DepthStencilWrite:
					{
						CrRenderTargetDescriptor depthDescriptor;
						depthDescriptor.texture           = m_textureResources[textureId.id].descriptor.texture;
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

						renderPassDescriptor.depth        = depthDescriptor;
						break;
					}
					case cr3d::TextureState::RWTexture:
					{
						if (transitionInfo.initialState != transitionInfo.usageState)
						{
							renderPassDescriptor.beginTextures.emplace_back
							(
								m_textureResources[textureId.id].descriptor.texture,
								textureUsage.mipmapStart, textureUsage.mipmapCount,
								textureUsage.sliceStart, textureUsage.sliceCount,
								transitionInfo.initialState, transitionInfo.initialShaderStages,
								transitionInfo.usageState, transitionInfo.usageShaderStages
							);
						}

						if (transitionInfo.usageState != transitionInfo.finalState)
						{
							renderPassDescriptor.endTextures.emplace_back
							(
								m_textureResources[textureId.id].descriptor.texture,
								textureUsage.mipmapStart, textureUsage.mipmapCount,
								textureUsage.sliceStart, textureUsage.sliceCount,
								transitionInfo.usageState, transitionInfo.usageShaderStages,
								transitionInfo.finalState, transitionInfo.finalShaderStages
							);
						}
						break;
					}
				}
			}

			for (uint32_t i = 0; i < renderGraphPass.bufferUsages.size(); ++i)
			{
				const CrRenderGraphBufferUsage& bufferUsage = renderGraphPass.bufferUsages[i];
				CrRenderGraphBufferId bufferId = bufferUsage.bufferId;
				const CrRenderGraphBufferTransition& transitionInfo = renderGraphPass.bufferTransitions.find(bufferId.id)->second;

				if (transitionInfo.initialState != transitionInfo.usageState)
				{
					renderPassDescriptor.beginBuffers.emplace_back(m_bufferResources[bufferId.id].descriptor.buffer, 
						transitionInfo.initialState, transitionInfo.initialShaderStages,
						transitionInfo.usageState, transitionInfo.usageShaderStages);
				}

				if (transitionInfo.usageState != transitionInfo.finalState)
				{
					renderPassDescriptor.endBuffers.emplace_back(m_bufferResources[bufferId.id].descriptor.buffer, 
						transitionInfo.usageState, transitionInfo.usageShaderStages,
						transitionInfo.finalState, transitionInfo.finalShaderStages);
				}
			}

			// TODO Compute hash statically
			CrGPUTimingRequest passRequest = m_frameParams.timingQueryTracker->AllocateTimingRequest(CrHash(renderGraphPass.name.c_str()));

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

void CrRenderGraph::Begin(const CrRenderGraphFrameParams& frameParams)
{
	m_frameParams = frameParams;
}

void CrRenderGraph::End()
{
	m_uniquePassId = CrRenderPassId(0);
	m_uniqueTextureId = CrRenderGraphTextureId(0);
	m_uniqueBufferId = CrRenderGraphBufferId(0);

	m_logicalPasses.reserve(256);
	m_textureResources.reserve(256);

	m_logicalPasses.clear();
	m_nameToLogicalPassId.clear();

	m_textureResources.clear();
	m_nameToTextureId.clear();

	m_bufferResources.clear();
	m_nameToBufferId.clear();

	m_textureLastUsedPass.clear();
	m_bufferLastUsedPass.clear();
}
