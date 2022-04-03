#include "CrRendering_pch.h"

#include "CrCommandBuffer_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrPipeline_d3d12.h"
#include "CrTexture_d3d12.h"
#include "CrSampler_d3d12.h"
#include "CrShader_d3d12.h"

#include "Core/CrAlignment.h"

#define USE_PIX
#include "WinPixEventRuntime/pix3.h"

#include "Core/Logging/ICrDebug.h"

CrCommandBufferD3D12::CrCommandBufferD3D12(ICrRenderDevice* renderDevice, CrCommandQueueType::T queueType)
	: ICrCommandBuffer(renderDevice, queueType)
	, m_CBV_SRV_UAV_DescriptorHeap(nullptr)
	, m_samplerDescriptorHeap(nullptr)
{
	CrRenderDeviceD3D12* d3d12RenderDevice = static_cast<CrRenderDeviceD3D12*>(renderDevice);

	D3D12_COMMAND_LIST_TYPE d3dc12CommandListType = crd3d::GetD3D12CommandQueueType(queueType);

	d3d12RenderDevice->GetD3D12Device()->CreateCommandAllocator(d3dc12CommandListType, IID_PPV_ARGS(&m_d3d12CommandAllocator));

	d3d12RenderDevice->GetD3D12Device()->CreateCommandList(0, d3dc12CommandListType, m_d3d12CommandAllocator, nullptr, IID_PPV_ARGS(&m_d3d12GraphicsCommandList));

	m_d3d12GraphicsCommandList->Close();

	{
		CrDescriptorHeapDescriptor cbv_SRV_UAV_DescriptorHeapDescriptor;
		cbv_SRV_UAV_DescriptorHeapDescriptor.name = "CBV SRV UAV Streaming Heap";
		cbv_SRV_UAV_DescriptorHeapDescriptor.numDescriptors = 32 * 1024;
		cbv_SRV_UAV_DescriptorHeapDescriptor.type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		m_CBV_SRV_UAV_DescriptorStream.Initialize(d3d12RenderDevice, cbv_SRV_UAV_DescriptorHeapDescriptor);
	}

	{
		CrDescriptorHeapDescriptor cbv_SRV_UAV_ShaderVisibleDescriptorHeapDescriptor;
		cbv_SRV_UAV_ShaderVisibleDescriptorHeapDescriptor.name = "CBV SRV UAV Shader Visible Streaming Heap";
		cbv_SRV_UAV_ShaderVisibleDescriptorHeapDescriptor.numDescriptors = 32 * 1024;
		cbv_SRV_UAV_ShaderVisibleDescriptorHeapDescriptor.type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbv_SRV_UAV_ShaderVisibleDescriptorHeapDescriptor.flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		m_CBV_SRV_UAV_ShaderVisibleDescriptorStream.Initialize(d3d12RenderDevice, cbv_SRV_UAV_ShaderVisibleDescriptorHeapDescriptor);
	}

	{
		CrDescriptorHeapDescriptor samplerDescriptorHeapDescriptor;
		samplerDescriptorHeapDescriptor.name = "Sampler Streaming Heap";
		samplerDescriptorHeapDescriptor.numDescriptors = 2048;
		samplerDescriptorHeapDescriptor.type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		m_samplerDescriptorStream.Initialize(d3d12RenderDevice, samplerDescriptorHeapDescriptor);
	}

	{
		CrDescriptorHeapDescriptor samplerShaderVisibleDescriptorHeapDescriptor;
		samplerShaderVisibleDescriptorHeapDescriptor.name = "Sampler Shader Visible Streaming Heap";
		samplerShaderVisibleDescriptorHeapDescriptor.numDescriptors = 2048;
		samplerShaderVisibleDescriptorHeapDescriptor.type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		samplerShaderVisibleDescriptorHeapDescriptor.flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		m_samplerShaderVisibleDescriptorStream.Initialize(d3d12RenderDevice, samplerShaderVisibleDescriptorHeapDescriptor);
	}
}

D3D12_RESOURCE_STATES GetTextureState(cr3d::TextureState::T textureState, cr3d::ShaderStageFlags::T shaderStages)
{
	switch (textureState)
	{
		case cr3d::TextureState::Undefined:         return D3D12_RESOURCE_STATE_COMMON;
		case cr3d::TextureState::ShaderInput:
		{
			D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_COMMON;

			// If the pixel shader is using the resource
			if (shaderStages & cr3d::ShaderStageFlags::Pixel)
			{
				resourceState |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			}

			// If any other stages are using the resource
			if ((shaderStages & ~cr3d::ShaderStageFlags::Pixel) != 0)
			{
				resourceState |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			}

			return resourceState;
		}
		case cr3d::TextureState::RenderTarget:      return D3D12_RESOURCE_STATE_RENDER_TARGET;
		case cr3d::TextureState::RWTexture:         return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		case cr3d::TextureState::Present:           return D3D12_RESOURCE_STATE_PRESENT;
		case cr3d::TextureState::DepthStencilRead:  return D3D12_RESOURCE_STATE_DEPTH_READ;
		case cr3d::TextureState::DepthStencilWrite: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
		case cr3d::TextureState::CopySource:        return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case cr3d::TextureState::CopyDestination:   return D3D12_RESOURCE_STATE_COPY_DEST;
		case cr3d::TextureState::PreInitialized:    return D3D12_RESOURCE_STATE_COMMON; // TODO Delete
		default: return D3D12_RESOURCE_STATE_COMMON;
	}
}

void CrCommandBufferD3D12::ProcessTextureAndBufferBarriers
(
	const CrRenderPassDescriptor::BufferTransitionVector& buffers, 
	const CrRenderPassDescriptor::TextureTransitionVector& textures, 
	CrBarrierVectorD3D12& resourceBarriers
)
{
	for (const CrRenderPassTextureDescriptor& descriptor : textures)
	{
		const CrTextureD3D12* d3d12Texture = static_cast<const CrTextureD3D12*>(descriptor.texture);

		D3D12_RESOURCE_BARRIER textureBarrier;
		textureBarrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		textureBarrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		textureBarrier.Transition.pResource   = d3d12Texture->GetD3D12Resource();
		textureBarrier.Transition.StateBefore = GetTextureState(descriptor.sourceState, descriptor.sourceShaderStages);
		textureBarrier.Transition.StateAfter  = GetTextureState(descriptor.destinationState, descriptor.destinationShaderStages);
		textureBarrier.Transition.Subresource = crd3d::CalculateSubresource
		(
			descriptor.mipmapStart, descriptor.sliceStart, 0,
			d3d12Texture->GetMipmapCount(), d3d12Texture->GetArraySize()
		);

		resourceBarriers.push_back(textureBarrier);
	}

	// TODO Handle buffer transitions
	(buffers);
}

void CrCommandBufferD3D12::ProcessRenderTargetBarrier(
	const CrRenderTargetDescriptor& renderTargetDescriptor, 
	cr3d::TextureState::T initialState,
	cr3d::TextureState::T finalState,
	CrBarrierVectorD3D12& resourceBarriers)
{
	const CrTextureD3D12* d3d12Texture = static_cast<const CrTextureD3D12*>(renderTargetDescriptor.texture);

	// Don't use the texture states from the render target descriptor. The reason is that the render target descriptor
	// contains information about the initial, usage and final states. Non-render target transitions already come split
	D3D12_RESOURCE_BARRIER textureBarrier;
	textureBarrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	textureBarrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	textureBarrier.Transition.pResource   = d3d12Texture->GetD3D12Resource();
	textureBarrier.Transition.StateBefore = GetTextureState(initialState, cr3d::ShaderStageFlags::None);
	textureBarrier.Transition.StateAfter  = GetTextureState(finalState, cr3d::ShaderStageFlags::None);
	textureBarrier.Transition.Subresource = crd3d::CalculateSubresource
	(
		renderTargetDescriptor.mipmap, renderTargetDescriptor.slice, 0,
		d3d12Texture->GetMipmapCount(), d3d12Texture->GetArraySize()
	);

	resourceBarriers.push_back(textureBarrier);
}

void CrCommandBufferD3D12::BeginRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor)
{
	CrBarrierVectorD3D12 resourceBarriers;

	ProcessTextureAndBufferBarriers(renderPassDescriptor.beginBuffers, renderPassDescriptor.beginTextures, resourceBarriers);

	for (const CrRenderTargetDescriptor& renderTargetDescriptor : renderPassDescriptor.color)
	{
		if (renderTargetDescriptor.initialState != renderTargetDescriptor.usageState)
		{
			ProcessRenderTargetBarrier(renderTargetDescriptor, renderTargetDescriptor.initialState, renderTargetDescriptor.usageState, resourceBarriers);
		}
	}

	const CrRenderTargetDescriptor& depthDescriptor = renderPassDescriptor.depth;

	if (depthDescriptor.texture && (depthDescriptor.initialState != depthDescriptor.usageState))
	{
		ProcessRenderTargetBarrier(depthDescriptor, depthDescriptor.initialState, depthDescriptor.usageState, resourceBarriers);
	}

	if (resourceBarriers.size() > 0)
	{
		m_d3d12GraphicsCommandList->ResourceBarrier((UINT)resourceBarriers.size(), resourceBarriers.data());
	}

	if (renderPassDescriptor.type == cr3d::RenderPassType::Graphics)
	{
		D3D12_RENDER_PASS_RENDER_TARGET_DESC d3d12RenderTargets[cr3d::MaxRenderTargets] = {};
		D3D12_RENDER_PASS_DEPTH_STENCIL_DESC d3d12DepthStencil = {};

		for (uint32_t i = 0, renderTargetCount = (uint32_t)renderPassDescriptor.color.size(); i < renderTargetCount; ++i)
		{
			const CrRenderTargetDescriptor& renderTargetDescriptor = renderPassDescriptor.color[i];
			D3D12_RENDER_PASS_RENDER_TARGET_DESC& renderTargetDesc = d3d12RenderTargets[i];
			const CrTextureD3D12* d3d12Texture = static_cast<const CrTextureD3D12*>(renderTargetDescriptor.texture);
			renderTargetDesc.cpuDescriptor = d3d12Texture->GetD3D12RenderTargetView(renderTargetDescriptor.mipmap, renderTargetDescriptor.slice).cpuHandle;
			
			// Render Target Load Operation
			renderTargetDesc.BeginningAccess.Type = crd3d::GetD3D12BeginningAccessType(renderTargetDescriptor.loadOp);
			renderTargetDesc.BeginningAccess.Clear.ClearValue.Format = crd3d::GetDXGIFormat(d3d12Texture->GetFormat());
			store(renderTargetDescriptor.clearColor, renderTargetDesc.BeginningAccess.Clear.ClearValue.Color);

			// Render Target Store Operation
			renderTargetDesc.EndingAccess.Type = crd3d::GetD3D12EndingAccessType(renderTargetDescriptor.storeOp);
		}

		if (depthDescriptor.texture)
		{
			const CrTextureD3D12* d3d12Texture = static_cast<const CrTextureD3D12*>(depthDescriptor.texture);
			d3d12DepthStencil.cpuDescriptor = d3d12Texture->GetD3D12DepthStencilView().cpuHandle;
			
			// Depth Load Operation
			d3d12DepthStencil.DepthBeginningAccess.Type = crd3d::GetD3D12BeginningAccessType(depthDescriptor.loadOp);
			d3d12DepthStencil.DepthBeginningAccess.Clear.ClearValue.Format = crd3d::GetDXGIFormat(d3d12Texture->GetFormat());
			d3d12DepthStencil.StencilBeginningAccess.Clear.ClearValue.DepthStencil.Depth = depthDescriptor.depthClearValue;
			d3d12DepthStencil.StencilBeginningAccess.Clear.ClearValue.DepthStencil.Stencil = depthDescriptor.stencilClearValue;

			// Depth Store Operation
			d3d12DepthStencil.DepthEndingAccess.Type = crd3d::GetD3D12EndingAccessType(depthDescriptor.storeOp);

			// Stencil Load Operation
			d3d12DepthStencil.StencilBeginningAccess.Type = crd3d::GetD3D12BeginningAccessType(depthDescriptor.loadOp);
			d3d12DepthStencil.StencilBeginningAccess.Clear.ClearValue.Format = crd3d::GetDXGIFormat(d3d12Texture->GetFormat());
			d3d12DepthStencil.StencilBeginningAccess.Clear.ClearValue.DepthStencil.Depth = depthDescriptor.depthClearValue;
			d3d12DepthStencil.StencilBeginningAccess.Clear.ClearValue.DepthStencil.Stencil = depthDescriptor.stencilClearValue;

			// Stencil Store Operation
			d3d12DepthStencil.StencilEndingAccess.Type = crd3d::GetD3D12EndingAccessType(depthDescriptor.storeOp);
		}

		m_d3d12GraphicsCommandList->BeginRenderPass
		(
			(UINT)renderPassDescriptor.color.size(),
			d3d12RenderTargets,
			renderPassDescriptor.depth.texture ? &d3d12DepthStencil : nullptr,
			D3D12_RENDER_PASS_FLAG_NONE // TODO Handle flags
		);
	}
}

void CrCommandBufferD3D12::EndRenderPassPS()
{
	CrBarrierVectorD3D12 resourceBarriers;
	const CrRenderPassDescriptor& renderPassDescriptor = m_currentState.m_currentRenderPass;

	if (renderPassDescriptor.type == cr3d::RenderPassType::Graphics)
	{
		m_d3d12GraphicsCommandList->EndRenderPass();

		for (const CrRenderTargetDescriptor& renderTargetDescriptor : renderPassDescriptor.color)
		{
			if (renderTargetDescriptor.usageState != renderTargetDescriptor.finalState)
			{
				ProcessRenderTargetBarrier(renderTargetDescriptor, renderTargetDescriptor.usageState, renderTargetDescriptor.finalState, resourceBarriers);
			}
		}

		const CrRenderTargetDescriptor& depthDescriptor = renderPassDescriptor.depth;

		if (depthDescriptor.texture && (depthDescriptor.usageState != depthDescriptor.finalState))
		{
			ProcessRenderTargetBarrier(depthDescriptor, depthDescriptor.usageState, depthDescriptor.finalState, resourceBarriers);
		}
	}

	ProcessTextureAndBufferBarriers(renderPassDescriptor.endBuffers, renderPassDescriptor.endTextures, resourceBarriers);

	if (resourceBarriers.size() > 0)
	{
		m_d3d12GraphicsCommandList->ResourceBarrier((UINT)resourceBarriers.size(), resourceBarriers.data());
	}
}

void CrCommandBufferD3D12::ResetGPUQueriesPS(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count)
{
	unused_parameter(queryPool);
	unused_parameter(start);
	unused_parameter(count);
}

void CrCommandBufferD3D12::ResolveGPUQueriesPS(const ICrGPUQueryPool* queryPool, uint32_t start, uint32_t count)
{
	const CrGPUQueryPoolD3D12* d3d12QueryPool = static_cast<const CrGPUQueryPoolD3D12*>(queryPool);
	const CrHardwareGPUBufferD3D12* d3d12GPUBuffer = static_cast<const CrHardwareGPUBufferD3D12*>(d3d12QueryPool->GetResultsBuffer());

	m_d3d12GraphicsCommandList->ResolveQueryData(d3d12QueryPool->GetD3D12QueryHeap(), D3D12_QUERY_TYPE_TIMESTAMP, start, count, d3d12GPUBuffer->GetD3D12Buffer(), start * sizeof(uint64_t));
}

void CrCommandBufferD3D12::FlushGraphicsRenderStatePS()
{
	CrRenderDeviceD3D12* d3d12RenderDevice = static_cast<CrRenderDeviceD3D12*>(m_renderDevice);

	const CrGraphicsPipelineD3D12* d3d12GraphicsPipeline = static_cast<const CrGraphicsPipelineD3D12*>(m_currentState.m_graphicsPipeline);
	const CrGraphicsShaderHandle& currentGraphicsShader = d3d12GraphicsPipeline->GetShader();

	if (m_currentState.m_indexBufferDirty)
	{
		const CrHardwareGPUBufferD3D12* d3d12GPUBuffer = static_cast<const CrHardwareGPUBufferD3D12*>(m_currentState.m_indexBuffer);
		ID3D12Resource* d3d12Resource = d3d12GPUBuffer->GetD3D12Buffer();

		D3D12_INDEX_BUFFER_VIEW d3d12IndexBufferView;
		d3d12IndexBufferView.BufferLocation = d3d12Resource->GetGPUVirtualAddress() + m_currentState.m_indexBufferOffset;
		d3d12IndexBufferView.SizeInBytes = d3d12GPUBuffer->sizeBytes;
		d3d12IndexBufferView.Format = crd3d::GetDXGIFormat(m_currentState.m_indexBufferFormat);

		m_d3d12GraphicsCommandList->IASetIndexBuffer(&d3d12IndexBufferView);
		m_currentState.m_indexBufferDirty = false;
	}

	if (m_currentState.m_vertexBufferDirty)
	{
		if (d3d12GraphicsPipeline->GetVertexStreamCount() > 0)
		{
			D3D12_VERTEX_BUFFER_VIEW d3d12Views[cr3d::MaxVertexStreams];

			uint32_t usedVertexStreamCount = d3d12GraphicsPipeline->GetVertexStreamCount();

			for (uint32_t streamId = 0; streamId < usedVertexStreamCount; ++streamId)
			{
				const VertexBufferBinding& binding = m_currentState.m_vertexBuffers[streamId];
				const CrHardwareGPUBufferD3D12* d3d12GPUBuffer = static_cast<const CrHardwareGPUBufferD3D12*>(binding.vertexBuffer);
				ID3D12Resource* d3d12Resource = d3d12GPUBuffer->GetD3D12Buffer();
				d3d12Views[streamId].BufferLocation = d3d12Resource->GetGPUVirtualAddress() + binding.offset;
				d3d12Views[streamId].SizeInBytes = binding.size;
				d3d12Views[streamId].StrideInBytes = binding.stride;
			}

			m_d3d12GraphicsCommandList->IASetVertexBuffers(0, usedVertexStreamCount, d3d12Views);
		}

		m_currentState.m_vertexBufferDirty = false;
	}

	if (m_currentState.m_scissorDirty)
	{
		const CrScissor& scissor = m_currentState.m_scissor;
		D3D12_RECT d3d12Rect = { (LONG)scissor.x, (LONG)scissor.y, (LONG)(scissor.x + scissor.width), (LONG)(scissor.y + scissor.height) };
		m_d3d12GraphicsCommandList->RSSetScissorRects(1, &d3d12Rect);
		m_currentState.m_scissorDirty = false;
	}

	if (m_currentState.m_viewportDirty)
	{
		const CrViewport& viewport = m_currentState.m_viewport;

		D3D12_VIEWPORT d3d12Viewport =
		{
			viewport.x,
			viewport.y,
			viewport.width,
			viewport.height,
			viewport.minDepth,
			viewport.maxDepth
		};

		m_d3d12GraphicsCommandList->RSSetViewports(1, &d3d12Viewport);

		m_currentState.m_viewportDirty = false;
	}

	if (m_currentState.m_graphicsPipelineDirty)
	{
		const CrGraphicsPipelineD3D12* d3dGraphicsPipeline = static_cast<const CrGraphicsPipelineD3D12*>(m_currentState.m_graphicsPipeline);
		m_d3d12GraphicsCommandList->SetPipelineState(d3dGraphicsPipeline->GetD3D12PipelineState());
		m_d3d12GraphicsCommandList->SetGraphicsRootSignature(d3dGraphicsPipeline->GetD3D12RootSignature()); // TODO Cache

		D3D12_PRIMITIVE_TOPOLOGY primitiveTopology = d3dGraphicsPipeline->GetD3D12PrimitiveTopologyType();
		if (m_primitiveTopology != primitiveTopology)
		{
			m_d3d12GraphicsCommandList->IASetPrimitiveTopology(primitiveTopology);
			m_primitiveTopology = primitiveTopology;
		}

		m_currentState.m_graphicsPipelineDirty = false;
	}

	if (m_currentState.m_stencilRefDirty)
	{
		m_d3d12GraphicsCommandList->OMSetStencilRef(m_currentState.m_stencilRef);
		m_currentState.m_stencilRefDirty = false;
	}

	const ICrShaderBindingLayout& bindingLayout = currentGraphicsShader->GetBindingLayout();

	// Samplers go in a different descriptor heap
	uint32_t numSRV_CBV_UAV = bindingLayout.GetTotalResourceCount() - bindingLayout.GetSamplerCount();
	uint32_t numSamplers = bindingLayout.GetSamplerCount();

	// Allocate the necessary number of descriptors and take note of the start of the tables to start offsetting based on the binding number
	crd3d::DescriptorD3D12 cbv_SRV_UAV_TableStart = m_CBV_SRV_UAV_DescriptorStream.Allocate(numSRV_CBV_UAV);
	crd3d::DescriptorD3D12 cbv_SRV_UAV_ShaderVisibleTableStart = m_CBV_SRV_UAV_ShaderVisibleDescriptorStream.Allocate(numSRV_CBV_UAV);

	crd3d::DescriptorD3D12 samplerTableStart = m_samplerDescriptorStream.Allocate(numSamplers);
	crd3d::DescriptorD3D12 samplerShaderVisibleTableStart = m_samplerShaderVisibleDescriptorStream.Allocate(numSamplers);

	uint32_t cbv_SRV_UAV_DescriptorSize = m_CBV_SRV_UAV_DescriptorStream.GetDescriptorHeap().GetDescriptorStride();
	uint32_t samplerDescriptorSize      = m_samplerDescriptorStream.GetDescriptorHeap().GetDescriptorStride();

	// Handle non-shader-visible tables
	crd3d::DescriptorD3D12 cbvTables[cr3d::ShaderStage::GraphicsStageCount] = {};
	crd3d::DescriptorD3D12 srvTables[cr3d::ShaderStage::GraphicsStageCount] = {};
	crd3d::DescriptorD3D12 uavTables[cr3d::ShaderStage::GraphicsStageCount] = {};

	cbvTables[cr3d::ShaderStage::Vertex] = cbv_SRV_UAV_TableStart;
	srvTables[cr3d::ShaderStage::Vertex] = cbvTables[cr3d::ShaderStage::Vertex] + bindingLayout.GetConstantBufferCount(cr3d::ShaderStage::Vertex) * cbv_SRV_UAV_DescriptorSize;
	uavTables[cr3d::ShaderStage::Vertex] = srvTables[cr3d::ShaderStage::Vertex] + (bindingLayout.GetTextureCount(cr3d::ShaderStage::Vertex) + bindingLayout.GetStorageBufferCount(cr3d::ShaderStage::Vertex)) * cbv_SRV_UAV_DescriptorSize;

	cbvTables[cr3d::ShaderStage::Pixel] = uavTables[cr3d::ShaderStage::Vertex];
	srvTables[cr3d::ShaderStage::Pixel] = cbvTables[cr3d::ShaderStage::Pixel] + bindingLayout.GetConstantBufferCount(cr3d::ShaderStage::Pixel) * cbv_SRV_UAV_DescriptorSize;
	uavTables[cr3d::ShaderStage::Pixel] = srvTables[cr3d::ShaderStage::Pixel] + (bindingLayout.GetTextureCount(cr3d::ShaderStage::Pixel) + bindingLayout.GetStorageBufferCount(cr3d::ShaderStage::Pixel)) * cbv_SRV_UAV_DescriptorSize;
	
	crd3d::DescriptorD3D12 samplerTables[cr3d::ShaderStage::GraphicsStageCount] = {};
	samplerTables[cr3d::ShaderStage::Vertex] = samplerTableStart;
	samplerTables[cr3d::ShaderStage::Pixel] = samplerTables[cr3d::ShaderStage::Vertex] + bindingLayout.GetSamplerCount(cr3d::ShaderStage::Vertex);

	bindingLayout.ForEachConstantBuffer([=](cr3d::ShaderStage::T stage, ConstantBuffers::T id, bindpoint_t bindPoint)
	{
		const ConstantBufferBinding& binding = m_currentState.GetConstantBufferBinding(stage, id);
		const CrHardwareGPUBufferD3D12* d3d12GPUBuffer = static_cast<const CrHardwareGPUBufferD3D12*>(binding.buffer);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDescriptor;
		cbvDescriptor.BufferLocation = d3d12GPUBuffer->GetD3D12Buffer()->GetGPUVirtualAddress() + binding.offsetBytes;
		cbvDescriptor.SizeInBytes = CrAlignUp256(binding.sizeBytes); // Align to 256 bytes as required by the spec

		crd3d::DescriptorD3D12 cbvHandle = cbvTables[stage] + bindPoint * cbv_SRV_UAV_DescriptorSize;
		d3d12RenderDevice->GetD3D12Device()->CreateConstantBufferView(&cbvDescriptor, cbvHandle.cpuHandle);
	});

	bindingLayout.ForEachTexture([=](cr3d::ShaderStage::T stage, Textures::T id, bindpoint_t bindPoint)
	{
		const ICrTexture* texture = m_currentState.m_textures[stage][id];
		const CrTextureD3D12* d3d12Texture = static_cast<const CrTextureD3D12*>(texture);	
		const D3D12_SHADER_RESOURCE_VIEW_DESC& srvDescriptor = d3d12Texture->GetD3D12ShaderResourceView();
		crd3d::DescriptorD3D12 srvHandle = srvTables[stage] + bindPoint * cbv_SRV_UAV_DescriptorSize;
		d3d12RenderDevice->GetD3D12Device()->CreateShaderResourceView(d3d12Texture->GetD3D12Resource(), &srvDescriptor, srvHandle.cpuHandle);
	});

	bindingLayout.ForEachRWTexture([=](cr3d::ShaderStage::T stage, RWTextures::T id, bindpoint_t bindPoint)
	{
		const RWTextureBinding& rwTextureBinding = m_currentState.m_rwTextures[stage][id];
		const CrTextureD3D12* d3d12Texture = static_cast<const CrTextureD3D12*>(rwTextureBinding.texture);
		const D3D12_UNORDERED_ACCESS_VIEW_DESC& uavDescriptor = d3d12Texture->GetD3D12UnorderedAccessView(rwTextureBinding.mip);
		crd3d::DescriptorD3D12 uavHandle = uavTables[stage] + bindPoint * cbv_SRV_UAV_DescriptorSize;
		d3d12RenderDevice->GetD3D12Device()->CreateUnorderedAccessView(d3d12Texture->GetD3D12Resource(), nullptr, &uavDescriptor, uavHandle.cpuHandle);
	});

	bindingLayout.ForEachSampler([=](cr3d::ShaderStage::T stage, Samplers::T id, bindpoint_t bindPoint)
	{
		const ICrSampler* sampler = m_currentState.m_samplers[stage][id];
		const CrSamplerD3D12* d3d12Sampler = static_cast<const CrSamplerD3D12*>(sampler);
		const D3D12_SAMPLER_DESC& samplerDescriptor = d3d12Sampler->GetD3D12Sampler();
		crd3d::DescriptorD3D12 samplerHandle = samplerTables[stage] + bindPoint * samplerDescriptorSize;
		d3d12RenderDevice->GetD3D12Device()->CreateSampler(&samplerDescriptor, samplerHandle.cpuHandle);
	});

	// Bind shader visible heaps to the command buffer
	ID3D12DescriptorHeap* cbv_SRV_UAV_ShaderVisibleDescriptorHeap = m_CBV_SRV_UAV_ShaderVisibleDescriptorStream.GetDescriptorHeap().GetD3D12DescriptorHeap();
	ID3D12DescriptorHeap* samplerShaderVisibleDescriptorHeap = m_samplerShaderVisibleDescriptorStream.GetDescriptorHeap().GetD3D12DescriptorHeap();

	if (m_CBV_SRV_UAV_DescriptorHeap != cbv_SRV_UAV_ShaderVisibleDescriptorHeap || m_samplerDescriptorHeap != samplerShaderVisibleDescriptorHeap)
	{
		ID3D12DescriptorHeap* descriptorHeaps[] =
		{
			cbv_SRV_UAV_ShaderVisibleDescriptorHeap, // One of CBV_SRV_UAV
			samplerShaderVisibleDescriptorHeap // One of samplers
		};

		m_d3d12GraphicsCommandList->SetDescriptorHeaps(2, descriptorHeaps);

		m_CBV_SRV_UAV_DescriptorHeap = cbv_SRV_UAV_ShaderVisibleDescriptorHeap;
		m_samplerDescriptorHeap = samplerShaderVisibleDescriptorHeap;
	}

	// Handle shader-visible tables
	crd3d::DescriptorD3D12 cbvShaderVisibleTables[cr3d::ShaderStage::GraphicsStageCount] = {};
	crd3d::DescriptorD3D12 srvShaderVisibleTables[cr3d::ShaderStage::GraphicsStageCount] = {};
	crd3d::DescriptorD3D12 uavShaderVisibleTables[cr3d::ShaderStage::GraphicsStageCount] = {};

	cbvShaderVisibleTables[cr3d::ShaderStage::Vertex] = cbv_SRV_UAV_ShaderVisibleTableStart;
	srvShaderVisibleTables[cr3d::ShaderStage::Vertex] = cbvShaderVisibleTables[cr3d::ShaderStage::Vertex] + bindingLayout.GetConstantBufferCount(cr3d::ShaderStage::Vertex) * cbv_SRV_UAV_DescriptorSize;
	uavShaderVisibleTables[cr3d::ShaderStage::Vertex] = srvShaderVisibleTables[cr3d::ShaderStage::Vertex] + (bindingLayout.GetTextureCount(cr3d::ShaderStage::Vertex) + bindingLayout.GetStorageBufferCount(cr3d::ShaderStage::Vertex)) * cbv_SRV_UAV_DescriptorSize;

	cbvShaderVisibleTables[cr3d::ShaderStage::Pixel] = uavShaderVisibleTables[cr3d::ShaderStage::Vertex];
	srvShaderVisibleTables[cr3d::ShaderStage::Pixel] = cbvShaderVisibleTables[cr3d::ShaderStage::Pixel] + bindingLayout.GetConstantBufferCount(cr3d::ShaderStage::Pixel) * cbv_SRV_UAV_DescriptorSize;
	uavShaderVisibleTables[cr3d::ShaderStage::Pixel] = srvShaderVisibleTables[cr3d::ShaderStage::Pixel] + (bindingLayout.GetTextureCount(cr3d::ShaderStage::Pixel) + bindingLayout.GetStorageBufferCount(cr3d::ShaderStage::Pixel)) * cbv_SRV_UAV_DescriptorSize;

	crd3d::DescriptorD3D12 samplerShaderVisibleTables[cr3d::ShaderStage::GraphicsStageCount] = {};
	samplerShaderVisibleTables[cr3d::ShaderStage::Vertex] = samplerShaderVisibleTableStart;
	samplerShaderVisibleTables[cr3d::ShaderStage::Pixel] = samplerShaderVisibleTables[cr3d::ShaderStage::Vertex] + bindingLayout.GetSamplerCount(cr3d::ShaderStage::Vertex);

	// Vertex shader resources
	m_d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(0, cbvShaderVisibleTables[cr3d::ShaderStage::Vertex].gpuHandle);
	m_d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(1, srvShaderVisibleTables[cr3d::ShaderStage::Vertex].gpuHandle);
	m_d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(2, uavShaderVisibleTables[cr3d::ShaderStage::Vertex].gpuHandle);
	m_d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(3, samplerShaderVisibleTables[cr3d::ShaderStage::Vertex].gpuHandle);

	// Pixel shader resources
	m_d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(4, cbvShaderVisibleTables[cr3d::ShaderStage::Pixel].gpuHandle);
	m_d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(5, srvShaderVisibleTables[cr3d::ShaderStage::Pixel].gpuHandle);
	m_d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(6, uavShaderVisibleTables[cr3d::ShaderStage::Pixel].gpuHandle);
	m_d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(7, samplerShaderVisibleTables[cr3d::ShaderStage::Pixel].gpuHandle);
}

void CrCommandBufferD3D12::FlushComputeRenderStatePS()
{
	const CrGraphicsPipelineD3D12* d3dComputePipeline = static_cast<const CrGraphicsPipelineD3D12*>(m_currentState.m_computePipeline);
	// const CrGraphicsShaderHandle& currentComputeShader = d3dComputePipeline->GetShader();

	if (m_currentState.m_computePipelineDirty)
	{
		m_d3d12GraphicsCommandList->SetPipelineState(d3dComputePipeline->GetD3D12PipelineState());
		m_d3d12GraphicsCommandList->SetComputeRootSignature(d3dComputePipeline->GetD3D12RootSignature()); // TODO Cache
		m_currentState.m_computePipelineDirty = false;
	}
}

void CrCommandBufferD3D12::BeginPS()
{
	m_d3d12CommandAllocator->Reset();
	m_d3d12GraphicsCommandList->Reset(m_d3d12CommandAllocator, nullptr);

	m_CBV_SRV_UAV_DescriptorStream.Reset();
	m_CBV_SRV_UAV_ShaderVisibleDescriptorStream.Reset();

	m_samplerDescriptorStream.Reset();
	m_samplerShaderVisibleDescriptorStream.Reset();

	m_CBV_SRV_UAV_DescriptorHeap = nullptr;
	m_samplerDescriptorHeap = nullptr;

	m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
}

void CrCommandBufferD3D12::ClearRenderTargetPS(const ICrTexture* renderTarget, const float4& color, uint32_t level, uint32_t slice, uint32_t levelCount, uint32_t sliceCount)
{
	unused_parameter(renderTarget);
	unused_parameter(color);
	unused_parameter(level);
	unused_parameter(slice);
	unused_parameter(levelCount);
	unused_parameter(sliceCount);
}

void CrCommandBufferD3D12::BeginDebugEventPS(const char* eventName, const float4& color)
{
	float4 color255 = color * 255.0f;
	BYTE r = (BYTE)(color255.r);
	BYTE g = (BYTE)(color255.g);
	BYTE b = (BYTE)(color255.b);

	PIXBeginEvent(m_d3d12GraphicsCommandList, PIX_COLOR(r, g, b), eventName);
}

void CrCommandBufferD3D12::EndDebugEventPS()
{
	PIXEndEvent(m_d3d12GraphicsCommandList);
}

void CrCommandBufferD3D12::InsertDebugMarkerPS(const char* markerName, const float4& color)
{
	float4 color255 = color * 255.0f;
	BYTE r = (BYTE)(color255.r);
	BYTE g = (BYTE)(color255.g);
	BYTE b = (BYTE)(color255.b);

	PIXSetMarker(m_d3d12GraphicsCommandList, PIX_COLOR(r, g, b), markerName);
}

void CrCommandBufferD3D12::EndPS()
{
	m_d3d12GraphicsCommandList->Close();

	CrRenderDeviceD3D12* d3d12RenderDevice = static_cast<CrRenderDeviceD3D12*>(m_renderDevice);

	// Copy CBV, SRV and UAV descriptors
	{
		crd3d::DescriptorD3D12 cbv_SRV_UAV_Heap = m_CBV_SRV_UAV_DescriptorStream.GetDescriptorHeap().GetHeapStart();
		crd3d::DescriptorD3D12 cbv_SRV_UAV_ShaderVisibleHeap = m_CBV_SRV_UAV_ShaderVisibleDescriptorStream.GetDescriptorHeap().GetHeapStart();
		CrAssertMsg(m_CBV_SRV_UAV_DescriptorStream.GetNumDescriptors() == m_CBV_SRV_UAV_ShaderVisibleDescriptorStream.GetNumDescriptors(), "Didn't allocate same descriptors");
		d3d12RenderDevice->GetD3D12Device()->CopyDescriptorsSimple(m_CBV_SRV_UAV_DescriptorStream.GetNumDescriptors(),
			cbv_SRV_UAV_ShaderVisibleHeap.cpuHandle, cbv_SRV_UAV_Heap.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	
	// Copy sampler descriptors
	{
		crd3d::DescriptorD3D12 samplerHeap = m_samplerDescriptorStream.GetDescriptorHeap().GetHeapStart();
		crd3d::DescriptorD3D12 samplerShaderVisibleHeap = m_samplerShaderVisibleDescriptorStream.GetDescriptorHeap().GetHeapStart();
		CrAssertMsg(m_samplerDescriptorStream.GetNumDescriptors() == m_samplerShaderVisibleDescriptorStream.GetNumDescriptors(), "Didn't allocate same descriptors");
		d3d12RenderDevice->GetD3D12Device()->CopyDescriptorsSimple(m_samplerDescriptorStream.GetNumDescriptors(),
			samplerShaderVisibleHeap.cpuHandle, samplerHeap.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	}
}
