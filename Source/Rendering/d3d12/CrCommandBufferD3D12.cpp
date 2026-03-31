#include "Rendering/CrRendering_pch.h"

#include "CrCommandBufferD3D12.h"
#include "CrRenderDeviceD3D12.h"
#include "CrPipelineD3D12.h"
#include "CrTextureD3D12.h"
#include "CrSamplerD3D12.h"
#include "CrShaderD3D12.h"

#include "Rendering/Extensions/CrPIXHeader.h"

#include "Core/CrAlignment.h"
#include "Core/Logging/ICrDebug.h"

CrCommandBufferD3D12::CrCommandBufferD3D12(ICrRenderDevice* renderDevice, const CrCommandBufferDescriptor& descriptor)
	: ICrCommandBuffer(renderDevice, descriptor)
	, m_primitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
	, m_shaderResourceDescriptorHeap(nullptr)
	, m_samplerDescriptorHeap(nullptr)
{
	CrRenderDeviceD3D12* d3d12RenderDevice = static_cast<CrRenderDeviceD3D12*>(renderDevice);

	D3D12_COMMAND_LIST_TYPE d3d12CommandListType = crd3d::GetD3D12CommandQueueType(descriptor.queueType);

	ID3D12Device* d3dDevice = d3d12RenderDevice->GetD3D12Device();

	d3dDevice->CreateCommandAllocator(d3d12CommandListType, IID_PPV_ARGS(&m_d3d12CommandAllocator));

	d3dDevice->CreateCommandList(0, d3d12CommandListType, m_d3d12CommandAllocator, nullptr, IID_PPV_ARGS(&m_d3d12GraphicsCommandList));

	d3d12RenderDevice->SetD3D12ObjectName(m_d3d12GraphicsCommandList, descriptor.name.c_str());

	m_d3d12GraphicsCommandList->QueryInterface(IID_PPV_ARGS(&m_d3d12GraphicsCommandList7));

	m_d3d12GraphicsCommandList->Close();

	{
		CrDescriptorHeapDescriptor dynamicDescriptorHeapDescriptor;
		dynamicDescriptorHeapDescriptor.name = "Dynamic Descriptor Heap";
		dynamicDescriptorHeapDescriptor.numDescriptors = 16 * 1024;
		dynamicDescriptorHeapDescriptor.type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		m_dynamicDescriptorStream.Initialize(d3d12RenderDevice, dynamicDescriptorHeapDescriptor);
	}

	{
		CrDescriptorHeapDescriptor shaderResourceShaderVisibleDescriptorHeapDescriptor;
		shaderResourceShaderVisibleDescriptorHeapDescriptor.name = "Shader Resource Shader Visible Streaming Heap";
		shaderResourceShaderVisibleDescriptorHeapDescriptor.numDescriptors = 32 * 1024;
		shaderResourceShaderVisibleDescriptorHeapDescriptor.type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		shaderResourceShaderVisibleDescriptorHeapDescriptor.flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		m_shaderResourceShaderVisibleDescriptorStream.Initialize(d3d12RenderDevice, shaderResourceShaderVisibleDescriptorHeapDescriptor);
	}

	{
		CrDescriptorHeapDescriptor samplerShaderVisibleDescriptorHeapDescriptor;
		samplerShaderVisibleDescriptorHeapDescriptor.name = "Sampler Shader Visible Streaming Heap";
		samplerShaderVisibleDescriptorHeapDescriptor.numDescriptors = 2048;
		samplerShaderVisibleDescriptorHeapDescriptor.type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		samplerShaderVisibleDescriptorHeapDescriptor.flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		m_samplerShaderVisibleDescriptorStream.Initialize(d3d12RenderDevice, samplerShaderVisibleDescriptorHeapDescriptor);
	}

	m_shaderResourceCPUDescriptors.Initialize(32 * 1024);

	m_samplerCPUDescriptors.Initialize(2048);
}

void CrCommandBufferD3D12::ProcessLegacyTextureAndBufferBarriers
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
		textureBarrier.Transition.StateBefore = crd3d::GetD3D12LegacyResourceState(descriptor.sourceState);
		textureBarrier.Transition.StateAfter  = crd3d::GetD3D12LegacyResourceState(descriptor.destinationState);
		CrAssertMsg(textureBarrier.Transition.StateBefore != textureBarrier.Transition.StateAfter, "States cannot be the same");

		// TODO Handle subresource transitions
		unsigned int planeSlice = descriptor.texturePlane == cr3d::TexturePlane::Stencil ? 1 : 0;

		textureBarrier.Transition.Subresource = crd3d::CalculateSubresource
		(
			descriptor.mipmapStart, descriptor.sliceStart, planeSlice,
			d3d12Texture->GetMipmapCount(), d3d12Texture->GetSliceCount()
		);

		resourceBarriers.push_back(textureBarrier);
	}

	// TODO Handle buffer transitions
	unused_parameter(buffers);
}

void CrCommandBufferD3D12::ProcessTextureBarriers
(
	const CrRenderPassDescriptor::TextureTransitionVector& textures,
	CrTextureBarrierVectorD3D12& d3d12TextureBarriers
)
{
	for (const CrRenderPassTextureDescriptor& descriptor : textures)
	{
		const CrTextureD3D12* d3d12Texture = static_cast<const CrTextureD3D12*>(descriptor.texture);

		crd3d::TextureBarrierInfoD3D12 sourceTextureBarrierInfo = crd3d::GetD3D12TextureBarrierInfo(descriptor.sourceState);
		crd3d::TextureBarrierInfoD3D12 destinationTextureBarrierInfo = crd3d::GetD3D12TextureBarrierInfo(descriptor.destinationState);

		D3D12_TEXTURE_BARRIER& d3d12TextureBarrier = d3d12TextureBarriers.push_back_uninitialized();
		d3d12TextureBarrier.SyncBefore                        = sourceTextureBarrierInfo.sync;
		d3d12TextureBarrier.SyncAfter                         = destinationTextureBarrierInfo.sync;
		d3d12TextureBarrier.AccessBefore                      = sourceTextureBarrierInfo.access;
		d3d12TextureBarrier.AccessAfter                       = destinationTextureBarrierInfo.access;
		d3d12TextureBarrier.LayoutBefore                      = crd3d::GetD3D12BarrierTextureLayout(descriptor.sourceState.layout);
		d3d12TextureBarrier.LayoutAfter                       = crd3d::GetD3D12BarrierTextureLayout(descriptor.destinationState.layout);
		d3d12TextureBarrier.pResource                         = d3d12Texture->GetD3D12Resource();
		d3d12TextureBarrier.Subresources.IndexOrFirstMipLevel = descriptor.mipmapStart;
		d3d12TextureBarrier.Subresources.NumMipLevels         = min(descriptor.mipmapCount, descriptor.texture->GetMipmapCount());
		d3d12TextureBarrier.Subresources.FirstArraySlice      = descriptor.sliceStart;
		d3d12TextureBarrier.Subresources.NumArraySlices       = min(descriptor.sliceCount, descriptor.texture->GetSliceCount());
		d3d12TextureBarrier.Subresources.FirstPlane           = descriptor.texturePlane;
		d3d12TextureBarrier.Subresources.NumPlanes            = 1;
		d3d12TextureBarrier.Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE; // TODO Handle discard when we have transient resources
		CrAssertMsg(d3d12TextureBarrier.LayoutBefore != d3d12TextureBarrier.LayoutAfter, "Layouts cannot be the same");
	}
}

void CrCommandBufferD3D12::ProcessBufferBarriers(const CrRenderPassDescriptor::BufferTransitionVector& buffers, CrBufferBarrierVectorD3D12& d3d12BufferBarriers)
{
	for (const CrRenderPassBufferDescriptor& descriptor : buffers)
	{
		const CrHardwareGPUBufferD3D12* d3d12Buffer = static_cast<const CrHardwareGPUBufferD3D12*>(descriptor.hardwareBuffer);

		crd3d::BufferBarrierInfoD3D12 sourceBufferBarrierInfo = crd3d::GetD3D12BufferBarrierInfo(descriptor.sourceState, descriptor.sourceShaderStages);
		crd3d::BufferBarrierInfoD3D12 destinationBufferBarrierInfo = crd3d::GetD3D12BufferBarrierInfo(descriptor.destinationState, descriptor.destinationShaderStages);

		D3D12_BUFFER_BARRIER& d3d12BufferBarrier = d3d12BufferBarriers.push_back_uninitialized();
		d3d12BufferBarrier.SyncBefore   = sourceBufferBarrierInfo.sync;
		d3d12BufferBarrier.SyncAfter    = destinationBufferBarrierInfo.sync;
		d3d12BufferBarrier.AccessBefore = sourceBufferBarrierInfo.access;
		d3d12BufferBarrier.AccessAfter  = destinationBufferBarrierInfo.access;
		d3d12BufferBarrier.pResource    = d3d12Buffer->GetD3D12Resource();
		d3d12BufferBarrier.Offset       = 0;
		d3d12BufferBarrier.Size         = d3d12Buffer->GetSizeBytes();
	}
}

void CrCommandBufferD3D12::ProcessLegacyRenderTargetBarrier(
	const CrRenderTargetDescriptor& renderTargetDescriptor, 
	const cr3d::TextureState& initialState,
	const cr3d::TextureState& finalState,
	CrBarrierVectorD3D12& resourceBarriers)
{
	const CrTextureD3D12* d3d12Texture = static_cast<const CrTextureD3D12*>(renderTargetDescriptor.texture);

	// Don't use the texture states from the render target descriptor. The reason is that the render target descriptor
	// contains information about the initial, usage and final states. Non-render target transitions already come split
	D3D12_RESOURCE_BARRIER textureBarrier;
	textureBarrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	textureBarrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	textureBarrier.Transition.pResource   = d3d12Texture->GetD3D12Resource();
	textureBarrier.Transition.StateBefore = crd3d::GetD3D12LegacyResourceState(initialState);
	textureBarrier.Transition.StateAfter  = crd3d::GetD3D12LegacyResourceState(finalState);

#if defined(COMMAND_BUFFER_VALIDATION)
	if (finalState.layout != cr3d::TextureLayout::Present)
	{
		CrAssertMsg(textureBarrier.Transition.StateAfter != D3D12_RESOURCE_STATE_COMMON, "Invalid transition state");
	}
#endif

	CrAssertMsg(textureBarrier.Transition.StateBefore != textureBarrier.Transition.StateAfter, "States cannot be the same");

	textureBarrier.Transition.Subresource = crd3d::CalculateSubresource
	(
		renderTargetDescriptor.mipmap, renderTargetDescriptor.slice, 0,
		d3d12Texture->GetMipmapCount(), d3d12Texture->GetSliceCount()
	);

	resourceBarriers.push_back(textureBarrier);
}

void CrCommandBufferD3D12::ProcessRenderTargetBarrier
(
	const CrRenderTargetDescriptor& renderTargetDescriptor,
	const cr3d::TextureState& initialState,
	const cr3d::TextureState& finalState,
	CrTextureBarrierVectorD3D12& d3d12TextureBarriers
)
{
	const CrTextureD3D12* d3d12Texture = static_cast<const CrTextureD3D12*>(renderTargetDescriptor.texture);

	crd3d::TextureBarrierInfoD3D12 sourceTextureBarrierInfo = crd3d::GetD3D12TextureBarrierInfo(initialState);
	crd3d::TextureBarrierInfoD3D12 destinationTextureBarrierInfo = crd3d::GetD3D12TextureBarrierInfo(finalState);

	D3D12_TEXTURE_BARRIER d3d12TextureBarrier = {};
	d3d12TextureBarrier.SyncBefore                        = sourceTextureBarrierInfo.sync;
	d3d12TextureBarrier.SyncAfter                         = destinationTextureBarrierInfo.sync;
	d3d12TextureBarrier.AccessBefore                      = sourceTextureBarrierInfo.access;
	d3d12TextureBarrier.AccessAfter                       = destinationTextureBarrierInfo.access;
	d3d12TextureBarrier.LayoutBefore                      = crd3d::GetD3D12BarrierTextureLayout(initialState.layout);
	d3d12TextureBarrier.LayoutAfter                       = crd3d::GetD3D12BarrierTextureLayout(finalState.layout);
	d3d12TextureBarrier.pResource                         = d3d12Texture->GetD3D12Resource();
	d3d12TextureBarrier.Subresources.IndexOrFirstMipLevel = renderTargetDescriptor.mipmap;
	d3d12TextureBarrier.Subresources.NumMipLevels         = 1;
	d3d12TextureBarrier.Subresources.FirstArraySlice      = renderTargetDescriptor.slice;
	d3d12TextureBarrier.Subresources.NumArraySlices       = 1;
	d3d12TextureBarrier.Subresources.FirstPlane           = 0;
	d3d12TextureBarrier.Subresources.NumPlanes            = 1;
	d3d12TextureBarrier.Flags                             = D3D12_TEXTURE_BARRIER_FLAG_NONE; // TODO Handle discard when we have transient resources
	CrAssertMsg(d3d12TextureBarrier.LayoutBefore != d3d12TextureBarrier.LayoutAfter, "Layouts cannot be the same");

	d3d12TextureBarriers.push_back(d3d12TextureBarrier);
}

void CrCommandBufferD3D12::BeginRenderPassPS(const CrRenderPassDescriptor& renderPassDescriptor)
{
	CrBarrierVectorD3D12 resourceBarriers;
	CrTextureBarrierVectorD3D12 textureBarriers;
	CrBufferBarrierVectorD3D12 bufferBarriers;

	bool enhancedBarriersSupported = static_cast<CrRenderDeviceD3D12*>(m_renderDevice)->GetIsEnhancedBarriersSupported();

	const CrRenderTargetDescriptor& depthDescriptor = renderPassDescriptor.depth;

	if (enhancedBarriersSupported)
	{
		ProcessTextureBarriers(renderPassDescriptor.beginTextures, textureBarriers);

		ProcessBufferBarriers(renderPassDescriptor.beginBuffers, bufferBarriers);

		for (const CrRenderTargetDescriptor& renderTargetDescriptor : renderPassDescriptor.color)
		{
			if (renderTargetDescriptor.initialState != renderTargetDescriptor.usageState)
			{
				ProcessRenderTargetBarrier(renderTargetDescriptor, renderTargetDescriptor.initialState, renderTargetDescriptor.usageState, textureBarriers);
			}
		}

		if (depthDescriptor.texture && (depthDescriptor.initialState != depthDescriptor.usageState))
		{
			ProcessRenderTargetBarrier(depthDescriptor, depthDescriptor.initialState, depthDescriptor.usageState, textureBarriers);
		}

		crstl::array<D3D12_BARRIER_GROUP, 3> barrierGroups = {};
		size_t barrierGroupCount = 0;

		if (textureBarriers.size() > 0)
		{
			barrierGroups[barrierGroupCount].Type = D3D12_BARRIER_TYPE_TEXTURE;
			barrierGroups[barrierGroupCount].NumBarriers = (UINT32)textureBarriers.size();
			barrierGroups[barrierGroupCount].pTextureBarriers = textureBarriers.data();
			barrierGroupCount++;
		}

		if (bufferBarriers.size() > 0)
		{
			barrierGroups[barrierGroupCount].Type = D3D12_BARRIER_TYPE_BUFFER;
			barrierGroups[barrierGroupCount].NumBarriers = (UINT32)bufferBarriers.size();
			barrierGroups[barrierGroupCount].pBufferBarriers = bufferBarriers.data();
			barrierGroupCount++;
		}

		if (barrierGroupCount)
		{
			m_d3d12GraphicsCommandList7->Barrier((UINT)barrierGroupCount, barrierGroups.data());
		}
	}
	else
	{
		ProcessLegacyTextureAndBufferBarriers(renderPassDescriptor.beginBuffers, renderPassDescriptor.beginTextures, resourceBarriers);

		for (const CrRenderTargetDescriptor& renderTargetDescriptor : renderPassDescriptor.color)
		{
			if (renderTargetDescriptor.initialState != renderTargetDescriptor.usageState)
			{
				ProcessLegacyRenderTargetBarrier(renderTargetDescriptor, renderTargetDescriptor.initialState, renderTargetDescriptor.usageState, resourceBarriers);
			}
		}

		if (depthDescriptor.texture && (depthDescriptor.initialState != depthDescriptor.usageState))
		{
			ProcessLegacyRenderTargetBarrier(depthDescriptor, depthDescriptor.initialState, depthDescriptor.usageState, resourceBarriers);
		}

		if (resourceBarriers.size() > 0)
		{
			m_d3d12GraphicsCommandList->ResourceBarrier((UINT)resourceBarriers.size(), resourceBarriers.data());
		}
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
			renderTargetDesc.cpuDescriptor = d3d12Texture->GetD3D12RenderTargetView(renderTargetDescriptor.mipmap, renderTargetDescriptor.slice);
			
			// Render Target Load Operation
			renderTargetDesc.BeginningAccess.Type = crd3d::GetD3D12BeginningAccessType(renderTargetDescriptor.loadOp);
			renderTargetDesc.BeginningAccess.Clear.ClearValue.Format = crd3d::GetDXGIFormat(d3d12Texture->GetFormat());
			store(renderTargetDesc.BeginningAccess.Clear.ClearValue.Color, renderTargetDescriptor.clearColor);

			// Render Target Store Operation
			renderTargetDesc.EndingAccess.Type = crd3d::GetD3D12EndingAccessType(renderTargetDescriptor.storeOp);
		}

		if (depthDescriptor.texture)
		{
			const CrTextureD3D12* d3d12Texture = static_cast<const CrTextureD3D12*>(depthDescriptor.texture);
			d3d12DepthStencil.cpuDescriptor = d3d12Texture->GetD3D12DepthStencilView();
			
			// Depth Load Operation
			d3d12DepthStencil.DepthBeginningAccess.Type = crd3d::GetD3D12BeginningAccessType(depthDescriptor.loadOp);
			d3d12DepthStencil.DepthBeginningAccess.Clear.ClearValue.Format = crd3d::GetDXGIFormat(d3d12Texture->GetFormat());
			d3d12DepthStencil.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth = depthDescriptor.depthClearValue;
			d3d12DepthStencil.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Stencil = depthDescriptor.stencilClearValue;

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
	CrTextureBarrierVectorD3D12 textureBarriers;
	CrBufferBarrierVectorD3D12 bufferBarriers;
	const CrRenderPassDescriptor& renderPassDescriptor = m_currentState.m_currentRenderPass;

	bool enhancedBarriersEnabled = static_cast<CrRenderDeviceD3D12*>(m_renderDevice)->GetIsEnhancedBarriersSupported();

	if (renderPassDescriptor.type == cr3d::RenderPassType::Graphics)
	{
		m_d3d12GraphicsCommandList->EndRenderPass();

		const CrRenderTargetDescriptor& depthDescriptor = renderPassDescriptor.depth;

		if (enhancedBarriersEnabled)
		{
			for (const CrRenderTargetDescriptor& renderTargetDescriptor : renderPassDescriptor.color)
			{
				if (renderTargetDescriptor.usageState != renderTargetDescriptor.finalState)
				{
					ProcessRenderTargetBarrier(renderTargetDescriptor, renderTargetDescriptor.usageState, renderTargetDescriptor.finalState, textureBarriers);
				}
			}

			if (depthDescriptor.texture && (depthDescriptor.usageState != depthDescriptor.finalState))
			{
				ProcessRenderTargetBarrier(depthDescriptor, depthDescriptor.usageState, depthDescriptor.finalState, textureBarriers);
			}
		}
		else
		{
			for (const CrRenderTargetDescriptor& renderTargetDescriptor : renderPassDescriptor.color)
			{
				if (renderTargetDescriptor.usageState != renderTargetDescriptor.finalState)
				{
					ProcessLegacyRenderTargetBarrier(renderTargetDescriptor, renderTargetDescriptor.usageState, renderTargetDescriptor.finalState, resourceBarriers);
				}
			}

			if (depthDescriptor.texture && (depthDescriptor.usageState != depthDescriptor.finalState))
			{
				ProcessLegacyRenderTargetBarrier(depthDescriptor, depthDescriptor.usageState, depthDescriptor.finalState, resourceBarriers);
			}
		}
	}

	if (enhancedBarriersEnabled)
	{
		ProcessTextureBarriers(renderPassDescriptor.endTextures, textureBarriers);

		ProcessBufferBarriers(renderPassDescriptor.endBuffers, bufferBarriers);

		crstl::array<D3D12_BARRIER_GROUP, 3> barrierGroups = {};
		size_t barrierGroupCount = 0;

		if (textureBarriers.size() > 0)
		{
			barrierGroups[barrierGroupCount].Type = D3D12_BARRIER_TYPE_TEXTURE;
			barrierGroups[barrierGroupCount].NumBarriers = (UINT32)textureBarriers.size();
			barrierGroups[barrierGroupCount].pTextureBarriers = textureBarriers.data();
			barrierGroupCount++;
		}

		if (bufferBarriers.size() > 0)
		{
			barrierGroups[barrierGroupCount].Type = D3D12_BARRIER_TYPE_BUFFER;
			barrierGroups[barrierGroupCount].NumBarriers = (UINT32)bufferBarriers.size();
			barrierGroups[barrierGroupCount].pBufferBarriers = bufferBarriers.data();
			barrierGroupCount++;
		}

		if (barrierGroupCount)
		{
			m_d3d12GraphicsCommandList7->Barrier((UINT)barrierGroupCount, barrierGroups.data());
		}
	}
	else
	{
		ProcessLegacyTextureAndBufferBarriers(renderPassDescriptor.endBuffers, renderPassDescriptor.endTextures, resourceBarriers);

		if (resourceBarriers.size() > 0)
		{
			m_d3d12GraphicsCommandList->ResourceBarrier((UINT)resourceBarriers.size(), resourceBarriers.data());
		}
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

	m_d3d12GraphicsCommandList->ResolveQueryData(d3d12QueryPool->GetD3D12QueryHeap(), D3D12_QUERY_TYPE_TIMESTAMP, start, count, d3d12GPUBuffer->GetD3D12Resource(), start * sizeof(uint64_t));
}

void CrCommandBufferD3D12::WriteCBV(const CrConstantBufferBinding& binding, D3D12_CPU_DESCRIPTOR_HANDLE& cbvHandle)
{
	CrRenderDeviceD3D12* d3d12RenderDevice = static_cast<CrRenderDeviceD3D12*>(m_renderDevice);
	const CrHardwareGPUBufferD3D12* d3d12GPUBuffer = static_cast<const CrHardwareGPUBufferD3D12*>(binding.buffer);

	// Allocate a single temporary descriptor from the stream. We don't store persistent constant buffer views due to the offset
	crd3d::DescriptorD3D12 cbvDescriptorD3D12 = m_dynamicDescriptorStream.Allocate(1);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDescriptor;
	cbvDescriptor.BufferLocation = d3d12GPUBuffer->GetD3D12Resource()->GetGPUVirtualAddress() + binding.offsetBytes;
	cbvDescriptor.SizeInBytes = CrAlignUp256(binding.sizeBytes); // Align to 256 bytes as required by the spec
	d3d12RenderDevice->GetD3D12Device()->CreateConstantBufferView(&cbvDescriptor, cbvDescriptorD3D12.cpuHandle);

	cbvHandle = cbvDescriptorD3D12.cpuHandle;
}

void CrCommandBufferD3D12::WriteTextureSRV(const CrTextureBinding& textureBinding, D3D12_CPU_DESCRIPTOR_HANDLE& srvHandle)
{
	const CrTextureD3D12* d3d12Texture = static_cast<const CrTextureD3D12*>(textureBinding.texture);
	D3D12_CPU_DESCRIPTOR_HANDLE srvDescriptor {};

	if (textureBinding.view == CrTextureView())
	{
		srvDescriptor = d3d12Texture->GetD3D12SRVDescriptor();
	}
	else if (textureBinding.view == CrTextureView(cr3d::TexturePlane::Stencil))
	{
		srvDescriptor = d3d12Texture->GetD3D12StencilSRVDescriptor();
	}
	else
	{
		CrAssertMsg(false, "Invalid image view");
	}

	srvHandle = srvDescriptor;
}

void CrCommandBufferD3D12::WriteSamplerView(const ICrSampler* sampler, D3D12_CPU_DESCRIPTOR_HANDLE& samplerHandle)
{
	const CrSamplerD3D12* d3d12Sampler = static_cast<const CrSamplerD3D12*>(sampler);
	samplerHandle = d3d12Sampler->GetD3D12Descriptor();
}

void CrCommandBufferD3D12::WriteRWTextureUAV(const CrRWTextureBinding& rwTextureBinding, D3D12_CPU_DESCRIPTOR_HANDLE& uavHandle)
{
	const CrTextureD3D12* d3d12Texture = static_cast<const CrTextureD3D12*>(rwTextureBinding.texture);
	uavHandle = d3d12Texture->GetD3D12UAVDescriptor(rwTextureBinding.mip);
}

void CrCommandBufferD3D12::WriteStorageBufferSRV(const CrStorageBufferBinding& binding, D3D12_CPU_DESCRIPTOR_HANDLE& srvHandle)
{
	CrRenderDeviceD3D12* d3d12RenderDevice = static_cast<CrRenderDeviceD3D12*>(m_renderDevice);
	const CrHardwareGPUBufferD3D12* d3d12GPUBuffer = static_cast<const CrHardwareGPUBufferD3D12*>(binding.buffer);

	crd3d::DescriptorD3D12 srvDescriptorD3D12 = m_dynamicDescriptorStream.Allocate(1);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescriptor = {};
	srvDescriptor.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDescriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescriptor.Buffer.NumElements = binding.numElements;
	srvDescriptor.Buffer.FirstElement = binding.offsetBytes / binding.strideBytes;

	if (d3d12GPUBuffer->HasUsage(cr3d::BufferUsage::Byte))
	{
		srvDescriptor.Format = DXGI_FORMAT_R32_TYPELESS;
		srvDescriptor.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
	}
	else
	{
		srvDescriptor.Format = DXGI_FORMAT_UNKNOWN;
		srvDescriptor.Buffer.StructureByteStride = binding.strideBytes;
	}

	d3d12RenderDevice->GetD3D12Device()->CreateShaderResourceView(d3d12GPUBuffer->GetD3D12Resource(), &srvDescriptor, srvDescriptorD3D12.cpuHandle);
	srvHandle = srvDescriptorD3D12.cpuHandle;
}

void CrCommandBufferD3D12::WriteRWStorageBufferUAV(const CrStorageBufferBinding& binding, D3D12_CPU_DESCRIPTOR_HANDLE& uavHandle)
{
	CrRenderDeviceD3D12* d3d12RenderDevice = static_cast<CrRenderDeviceD3D12*>(m_renderDevice);
	const CrHardwareGPUBufferD3D12* d3d12GPUBuffer = static_cast<const CrHardwareGPUBufferD3D12*>(binding.buffer);

	crd3d::DescriptorD3D12 uavDescriptorD3D12 = m_dynamicDescriptorStream.Allocate(1);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDescriptor = {};
	uavDescriptor.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

	if (d3d12GPUBuffer->HasUsage(cr3d::BufferUsage::Byte))
	{
		uavDescriptor.Format = DXGI_FORMAT_R32_TYPELESS;
		uavDescriptor.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
		uavDescriptor.Buffer.FirstElement = 0;
	}
	else
	{
		uavDescriptor.Format = DXGI_FORMAT_UNKNOWN;
		uavDescriptor.Buffer.FirstElement = binding.offsetBytes / binding.strideBytes;
		uavDescriptor.Buffer.StructureByteStride = binding.strideBytes;
	}

	uavDescriptor.Buffer.NumElements = binding.numElements;

	d3d12RenderDevice->GetD3D12Device()->CreateUnorderedAccessView(d3d12GPUBuffer->GetD3D12Resource(), nullptr, &uavDescriptor, uavDescriptorD3D12.cpuHandle);
	uavHandle = uavDescriptorD3D12.cpuHandle;
}

void CrCommandBufferD3D12::FlushGraphicsRenderStatePS()
{
	const CrGraphicsPipelineD3D12* d3d12GraphicsPipeline = static_cast<const CrGraphicsPipelineD3D12*>(m_currentState.m_graphicsPipeline);
	const CrGraphicsShaderHandle& currentGraphicsShader = d3d12GraphicsPipeline->GetShader();

	if (m_currentState.m_indexBufferDirty)
	{
		const CrHardwareGPUBufferD3D12* d3d12GPUBuffer = static_cast<const CrHardwareGPUBufferD3D12*>(m_currentState.m_indexBuffer);
		ID3D12Resource* d3d12Resource = d3d12GPUBuffer->GetD3D12Resource();

		D3D12_INDEX_BUFFER_VIEW d3d12IndexBufferView;
		d3d12IndexBufferView.BufferLocation = d3d12Resource->GetGPUVirtualAddress() + m_currentState.m_indexBufferOffset;
		d3d12IndexBufferView.SizeInBytes = m_currentState.m_indexBufferSize;
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
				const CrVertexBufferBinding& binding = m_currentState.m_vertexBuffers[streamId];
				const CrHardwareGPUBufferD3D12* d3d12GPUBuffer = static_cast<const CrHardwareGPUBufferD3D12*>(binding.vertexBuffer);
				ID3D12Resource* d3d12Resource = d3d12GPUBuffer->GetD3D12Resource();
				d3d12Views[streamId].BufferLocation = d3d12Resource->GetGPUVirtualAddress() + binding.offset;
				d3d12Views[streamId].SizeInBytes = binding.vertexCount * binding.stride;
				d3d12Views[streamId].StrideInBytes = binding.stride;
			}

			m_d3d12GraphicsCommandList->IASetVertexBuffers(0, usedVertexStreamCount, d3d12Views);
		}

		m_currentState.m_vertexBufferDirty = false;
	}

	if (m_currentState.m_scissorDirty)
	{
		const CrRectangle& scissor = m_currentState.m_scissor;
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
	uint32_t totalShaderResourceCount = bindingLayout.GetTotalResourceCount() - bindingLayout.GetSamplerCount();
	uint32_t totalSamplerCount = bindingLayout.GetSamplerCount();

	// Allocate the necessary number of descriptors and take note of the start of the tables to start offsetting based on the binding number
	crd3d::DescriptorD3D12 shaderResourceShaderVisibleTableStart = m_shaderResourceShaderVisibleDescriptorStream.Allocate(totalShaderResourceCount);
	crd3d::DescriptorD3D12 samplerShaderVisibleTableStart = m_samplerShaderVisibleDescriptorStream.Allocate(totalSamplerCount);

	uint32_t shaderResourceDescriptorSize = m_shaderResourceShaderVisibleDescriptorStream.GetDescriptorHeap().GetDescriptorStride();
	uint32_t samplerDescriptorSize = m_samplerShaderVisibleDescriptorStream.GetDescriptorHeap().GetDescriptorStride();

	uint32_t cbvCount[cr3d::ShaderStage::GraphicsStageCount] = {};
	uint32_t srvCount[cr3d::ShaderStage::GraphicsStageCount] = {};
	uint32_t uavCount[cr3d::ShaderStage::GraphicsStageCount] = {};
	uint32_t samplerCount[cr3d::ShaderStage::GraphicsStageCount] = {};

	// TODO Precompute in some way
	for (cr3d::ShaderStage::T shaderStage = cr3d::ShaderStage::Vertex; shaderStage < cr3d::ShaderStage::GraphicsStageCount; ++shaderStage)
	{
		cbvCount[shaderStage] = bindingLayout.GetConstantBufferCount(shaderStage);
		srvCount[shaderStage] = bindingLayout.GetTextureCount(shaderStage) + bindingLayout.GetStorageBufferCount(shaderStage);
		uavCount[shaderStage] = bindingLayout.GetRWTextureCount(shaderStage) + bindingLayout.GetRWStorageBufferCount(shaderStage);
		samplerCount[shaderStage] = bindingLayout.GetSamplerCount(shaderStage);
	}

	// Handle non-shader-visible tables
	size_t cbvOffsets[cr3d::ShaderStage::GraphicsStageCount] = {};
	size_t srvOffsets[cr3d::ShaderStage::GraphicsStageCount] = {};
	size_t uavOffsets[cr3d::ShaderStage::GraphicsStageCount] = {};
	size_t samplerOffsets[cr3d::ShaderStage::GraphicsStageCount] = {};

	size_t shaderResourceOffset = m_shaderResourceCPUDescriptors.Allocate(totalShaderResourceCount);
	size_t samplerOffset = m_samplerCPUDescriptors.Allocate(totalSamplerCount);

	cbvOffsets[cr3d::ShaderStage::Vertex] = shaderResourceOffset; shaderResourceOffset += cbvCount[cr3d::ShaderStage::Vertex];
	srvOffsets[cr3d::ShaderStage::Vertex] = shaderResourceOffset; shaderResourceOffset += srvCount[cr3d::ShaderStage::Vertex];
	uavOffsets[cr3d::ShaderStage::Vertex] = shaderResourceOffset; shaderResourceOffset += uavCount[cr3d::ShaderStage::Vertex];

	cbvOffsets[cr3d::ShaderStage::Pixel] = shaderResourceOffset; shaderResourceOffset += cbvCount[cr3d::ShaderStage::Pixel];
	srvOffsets[cr3d::ShaderStage::Pixel] = shaderResourceOffset; shaderResourceOffset += srvCount[cr3d::ShaderStage::Pixel];
	uavOffsets[cr3d::ShaderStage::Pixel] = shaderResourceOffset; shaderResourceOffset += uavCount[cr3d::ShaderStage::Pixel];

	samplerOffsets[cr3d::ShaderStage::Vertex] = samplerOffset; samplerOffset += samplerCount[cr3d::ShaderStage::Vertex];
	samplerOffsets[cr3d::ShaderStage::Pixel] = samplerOffset;

	bindingLayout.ForEachConstantBuffer([=](cr3d::ShaderStage::T stage, ConstantBuffers::T id, bindpoint_t bindPoint)
	{
		WriteCBV(m_currentState.m_constantBuffers[id], m_shaderResourceCPUDescriptors[cbvOffsets[stage] + bindPoint]);
	});

	bindingLayout.ForEachSampler([=](cr3d::ShaderStage::T stage, Samplers::T id, bindpoint_t bindPoint)
	{
		WriteSamplerView(m_currentState.m_samplers[id], m_samplerCPUDescriptors[samplerOffsets[stage] + bindPoint]);
	});

	bindingLayout.ForEachTexture([=](cr3d::ShaderStage::T stage, Textures::T id, bindpoint_t bindPoint)
	{
		WriteTextureSRV(m_currentState.m_textures[id], m_shaderResourceCPUDescriptors[srvOffsets[stage] + bindPoint]);
	});

	bindingLayout.ForEachRWTexture([=](cr3d::ShaderStage::T stage, RWTextures::T id, bindpoint_t bindPoint)
	{
		WriteRWTextureUAV(m_currentState.m_rwTextures[id], m_shaderResourceCPUDescriptors[uavOffsets[stage] + bindPoint]);
	});

	bindingLayout.ForEachStorageBuffer([=](cr3d::ShaderStage::T stage, StorageBuffers::T id, bindpoint_t bindPoint)
	{
		WriteStorageBufferSRV(m_currentState.m_storageBuffers[id], m_shaderResourceCPUDescriptors[srvOffsets[stage] + bindPoint]);
	});

	bindingLayout.ForEachRWStorageBuffer([=](cr3d::ShaderStage::T stage, RWStorageBuffers::T id, bindpoint_t bindPoint)
	{
		WriteRWStorageBufferUAV(m_currentState.m_rwStorageBuffers[id], m_shaderResourceCPUDescriptors[uavOffsets[stage] + bindPoint]);
	});

	// Bind shader visible heaps to the command buffer
	ID3D12DescriptorHeap* shaderResourceShaderVisibleDescriptorHeap = m_shaderResourceShaderVisibleDescriptorStream.GetDescriptorHeap().GetD3D12DescriptorHeap();
	ID3D12DescriptorHeap* samplerShaderVisibleDescriptorHeap = m_samplerShaderVisibleDescriptorStream.GetDescriptorHeap().GetD3D12DescriptorHeap();

	if (m_shaderResourceDescriptorHeap != shaderResourceShaderVisibleDescriptorHeap || m_samplerDescriptorHeap != samplerShaderVisibleDescriptorHeap)
	{
		// One of CBV_SRV_UAV and one of samplers
		ID3D12DescriptorHeap* descriptorHeaps[] = { shaderResourceShaderVisibleDescriptorHeap, samplerShaderVisibleDescriptorHeap };
		m_d3d12GraphicsCommandList->SetDescriptorHeaps(2, descriptorHeaps);

		m_shaderResourceDescriptorHeap = shaderResourceShaderVisibleDescriptorHeap;
		m_samplerDescriptorHeap = samplerShaderVisibleDescriptorHeap;
	}

	// Handle shader-visible tables
	crd3d::DescriptorD3D12 cbvShaderVisibleTables[cr3d::ShaderStage::GraphicsStageCount] = {};
	crd3d::DescriptorD3D12 srvShaderVisibleTables[cr3d::ShaderStage::GraphicsStageCount] = {};
	crd3d::DescriptorD3D12 uavShaderVisibleTables[cr3d::ShaderStage::GraphicsStageCount] = {};
	crd3d::DescriptorD3D12 samplerShaderVisibleTables[cr3d::ShaderStage::GraphicsStageCount] = {};

	crd3d::DescriptorD3D12 shaderResourceShaderVisibleOffset = shaderResourceShaderVisibleTableStart;
	cbvShaderVisibleTables[cr3d::ShaderStage::Vertex] = shaderResourceShaderVisibleOffset; shaderResourceShaderVisibleOffset += cbvCount[cr3d::ShaderStage::Vertex] * shaderResourceDescriptorSize;
	srvShaderVisibleTables[cr3d::ShaderStage::Vertex] = shaderResourceShaderVisibleOffset; shaderResourceShaderVisibleOffset += srvCount[cr3d::ShaderStage::Vertex] * shaderResourceDescriptorSize;
	uavShaderVisibleTables[cr3d::ShaderStage::Vertex] = shaderResourceShaderVisibleOffset; shaderResourceShaderVisibleOffset += uavCount[cr3d::ShaderStage::Vertex] * shaderResourceDescriptorSize;

	cbvShaderVisibleTables[cr3d::ShaderStage::Pixel] = shaderResourceShaderVisibleOffset; shaderResourceShaderVisibleOffset += cbvCount[cr3d::ShaderStage::Pixel] * shaderResourceDescriptorSize;
	srvShaderVisibleTables[cr3d::ShaderStage::Pixel] = shaderResourceShaderVisibleOffset; shaderResourceShaderVisibleOffset += srvCount[cr3d::ShaderStage::Pixel] * shaderResourceDescriptorSize;
	uavShaderVisibleTables[cr3d::ShaderStage::Pixel] = shaderResourceShaderVisibleOffset; shaderResourceShaderVisibleOffset += uavCount[cr3d::ShaderStage::Pixel] * shaderResourceDescriptorSize;

	crd3d::DescriptorD3D12 samplerShaderVisibleOffset = samplerShaderVisibleTableStart;
	samplerShaderVisibleTables[cr3d::ShaderStage::Vertex] = samplerShaderVisibleOffset; samplerShaderVisibleOffset += samplerCount[cr3d::ShaderStage::Vertex] * samplerDescriptorSize;
	samplerShaderVisibleTables[cr3d::ShaderStage::Pixel] = samplerShaderVisibleOffset;

	// Vertex shader resources
	if (cbvCount[cr3d::ShaderStage::Vertex])
	{
		m_d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(0, cbvShaderVisibleTables[cr3d::ShaderStage::Vertex].gpuHandle);
	}

	if (srvCount[cr3d::ShaderStage::Vertex])
	{
		m_d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(1, srvShaderVisibleTables[cr3d::ShaderStage::Vertex].gpuHandle);
	}

	if (uavCount[cr3d::ShaderStage::Vertex])
	{
		m_d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(2, uavShaderVisibleTables[cr3d::ShaderStage::Vertex].gpuHandle);
	}

	if (samplerCount[cr3d::ShaderStage::Vertex])
	{
		m_d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(3, samplerShaderVisibleTables[cr3d::ShaderStage::Vertex].gpuHandle);
	}

	// Pixel shader resources

	if (cbvCount[cr3d::ShaderStage::Pixel])
	{
		m_d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(4, cbvShaderVisibleTables[cr3d::ShaderStage::Pixel].gpuHandle);
	}

	if (srvCount[cr3d::ShaderStage::Pixel])
	{
		m_d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(5, srvShaderVisibleTables[cr3d::ShaderStage::Pixel].gpuHandle);
	}

	if (uavCount[cr3d::ShaderStage::Pixel])
	{
		m_d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(6, uavShaderVisibleTables[cr3d::ShaderStage::Pixel].gpuHandle);
	}

	if (samplerCount[cr3d::ShaderStage::Pixel])
	{
		m_d3d12GraphicsCommandList->SetGraphicsRootDescriptorTable(7, samplerShaderVisibleTables[cr3d::ShaderStage::Pixel].gpuHandle);
	}
}

void CrCommandBufferD3D12::FlushComputeRenderStatePS()
{
	const CrComputePipelineD3D12* d3dComputePipeline = static_cast<const CrComputePipelineD3D12*>(m_currentState.m_computePipeline);
	const CrComputeShaderHandle& currentComputeShader = d3dComputePipeline->GetShader();

	if (m_currentState.m_computePipelineDirty)
	{
		m_d3d12GraphicsCommandList->SetPipelineState(d3dComputePipeline->GetD3D12PipelineState());
		m_d3d12GraphicsCommandList->SetComputeRootSignature(d3dComputePipeline->GetD3D12RootSignature()); // TODO Cache
		m_currentState.m_computePipelineDirty = false;
	}

	const ICrShaderBindingLayout& bindingLayout = currentComputeShader->GetBindingLayout();

	// Samplers go in a different descriptor heap
	uint32_t shaderResourceCount = bindingLayout.GetTotalResourceCount() - bindingLayout.GetSamplerCount();
	uint32_t samplerCount        = bindingLayout.GetSamplerCount();

	uint32_t constantBufferCount = bindingLayout.GetConstantBufferCount();
	uint32_t storageBufferCount  = bindingLayout.GetStorageBufferCount();
	uint32_t textureCount        = bindingLayout.GetTextureCount();

	// Allocate the necessary number of descriptors and take note of the start of the tables to start offsetting based on the binding number
	crd3d::DescriptorD3D12 shaderResourceShaderVisibleTableStart = m_shaderResourceShaderVisibleDescriptorStream.Allocate(shaderResourceCount);
	crd3d::DescriptorD3D12 samplerShaderVisibleTableStart = m_samplerShaderVisibleDescriptorStream.Allocate(samplerCount);

	uint32_t shaderResourceDescriptorSize = m_shaderResourceShaderVisibleDescriptorStream.GetDescriptorHeap().GetDescriptorStride();

	size_t shaderResourceOffset = m_shaderResourceCPUDescriptors.Allocate(shaderResourceCount);
	size_t samplerOffset = m_samplerCPUDescriptors.Allocate(samplerCount);

	size_t cbvOffset = shaderResourceOffset; shaderResourceOffset += constantBufferCount;
	size_t srvOffset = shaderResourceOffset; shaderResourceOffset += (textureCount + storageBufferCount);
	size_t uavOffset = shaderResourceOffset;

	bindingLayout.ForEachConstantBuffer([&](cr3d::ShaderStage::T, ConstantBuffers::T id, bindpoint_t bindPoint)
	{
		WriteCBV(m_currentState.m_constantBuffers[id], m_shaderResourceCPUDescriptors[cbvOffset + bindPoint]);
	});

	bindingLayout.ForEachSampler([&](cr3d::ShaderStage::T, Samplers::T id, bindpoint_t bindPoint)
	{
		WriteSamplerView(m_currentState.m_samplers[id], m_samplerCPUDescriptors[samplerOffset + bindPoint]);
	});

	bindingLayout.ForEachTexture([&](cr3d::ShaderStage::T, Textures::T id, bindpoint_t bindPoint)
	{
		WriteTextureSRV(m_currentState.m_textures[id], m_shaderResourceCPUDescriptors[srvOffset + bindPoint]);
	});

	bindingLayout.ForEachRWTexture([&](cr3d::ShaderStage::T, RWTextures::T id, bindpoint_t bindPoint)
	{
		WriteRWTextureUAV(m_currentState.m_rwTextures[id], m_shaderResourceCPUDescriptors[uavOffset + bindPoint]);
	});

	bindingLayout.ForEachStorageBuffer([&](cr3d::ShaderStage::T, StorageBuffers::T id, bindpoint_t bindPoint)
	{
		WriteStorageBufferSRV(m_currentState.m_storageBuffers[id], m_shaderResourceCPUDescriptors[srvOffset + bindPoint]);
	});

	bindingLayout.ForEachRWStorageBuffer([&](cr3d::ShaderStage::T, RWStorageBuffers::T id, bindpoint_t bindPoint)
	{
		WriteRWStorageBufferUAV(m_currentState.m_rwStorageBuffers[id], m_shaderResourceCPUDescriptors[uavOffset + bindPoint]);
	});

	// Bind shader visible heaps to the command buffer
	ID3D12DescriptorHeap* shaderResourceShaderVisibleDescriptorHeap = m_shaderResourceShaderVisibleDescriptorStream.GetDescriptorHeap().GetD3D12DescriptorHeap();
	ID3D12DescriptorHeap* samplerShaderVisibleDescriptorHeap = m_samplerShaderVisibleDescriptorStream.GetDescriptorHeap().GetD3D12DescriptorHeap();

	if (m_shaderResourceDescriptorHeap != shaderResourceShaderVisibleDescriptorHeap || m_samplerDescriptorHeap != samplerShaderVisibleDescriptorHeap)
	{
		// One of CBV_SRV_UAV and one of samplers
		ID3D12DescriptorHeap* descriptorHeaps[] = { shaderResourceShaderVisibleDescriptorHeap, samplerShaderVisibleDescriptorHeap };
		m_d3d12GraphicsCommandList->SetDescriptorHeaps(2, descriptorHeaps);

		m_shaderResourceDescriptorHeap = shaderResourceShaderVisibleDescriptorHeap;
		m_samplerDescriptorHeap = samplerShaderVisibleDescriptorHeap;
	}

	// Handle shader-visible tables
	crd3d::DescriptorD3D12 shaderResourceShaderVisibleOffset = shaderResourceShaderVisibleTableStart;
	crd3d::DescriptorD3D12 cbvShaderVisibleTable = shaderResourceShaderVisibleOffset; shaderResourceShaderVisibleOffset += constantBufferCount * shaderResourceDescriptorSize;
	crd3d::DescriptorD3D12 srvShaderVisibleTable = shaderResourceShaderVisibleOffset; shaderResourceShaderVisibleOffset += (textureCount + storageBufferCount) * shaderResourceDescriptorSize;
	crd3d::DescriptorD3D12 uavShaderVisibleTable = shaderResourceShaderVisibleOffset;
	crd3d::DescriptorD3D12 samplerShaderVisibleTable = samplerShaderVisibleTableStart;

	// Compute shader resources
	m_d3d12GraphicsCommandList->SetComputeRootDescriptorTable(0, cbvShaderVisibleTable.gpuHandle);
	m_d3d12GraphicsCommandList->SetComputeRootDescriptorTable(1, srvShaderVisibleTable.gpuHandle);
	m_d3d12GraphicsCommandList->SetComputeRootDescriptorTable(2, uavShaderVisibleTable.gpuHandle);
	m_d3d12GraphicsCommandList->SetComputeRootDescriptorTable(3, samplerShaderVisibleTable.gpuHandle);
}

void CrCommandBufferD3D12::BeginPS()
{
	m_d3d12CommandAllocator->Reset();
	m_d3d12GraphicsCommandList->Reset(m_d3d12CommandAllocator, nullptr);

	m_shaderResourceShaderVisibleDescriptorStream.Reset();
	m_samplerShaderVisibleDescriptorStream.Reset();

	m_dynamicDescriptorStream.Reset();

	m_shaderResourceCPUDescriptors.Reset();
	m_samplerCPUDescriptors.Reset();

	m_shaderResourceDescriptorHeap = nullptr;
	m_samplerDescriptorHeap = nullptr;

	m_primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
}

void CrCommandBufferD3D12::ClearRenderTargetPS(const ICrTexture* renderTarget, const float4& color, uint32_t mip, uint32_t slice, uint32_t mipCount, uint32_t sliceCount)
{
	unused_parameter(renderTarget);
	unused_parameter(color);
	unused_parameter(mip);
	unused_parameter(slice);
	unused_parameter(mipCount);
	unused_parameter(sliceCount);
}

void CrCommandBufferD3D12::DrawIndirectPS(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count)
{
	ID3D12CommandSignature* commandSignature = static_cast<const CrRenderDeviceD3D12*>(m_renderDevice)->GetD3D12DrawIndirectCommandSignature();
	ID3D12Resource* resource = static_cast<const CrHardwareGPUBufferD3D12*>(indirectBuffer)->GetD3D12Resource();
	m_d3d12GraphicsCommandList->ExecuteIndirect(commandSignature, count, resource, offset, nullptr, 0);
}

void CrCommandBufferD3D12::DrawIndexedIndirectPS(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset, uint32_t count)
{
	ID3D12CommandSignature* commandSignature = static_cast<const CrRenderDeviceD3D12*>(m_renderDevice)->GetD3D12DrawIndexedIndirectCommandSignature();
	ID3D12Resource* resource = static_cast<const CrHardwareGPUBufferD3D12*>(indirectBuffer)->GetD3D12Resource();
	m_d3d12GraphicsCommandList->ExecuteIndirect(commandSignature, count, resource, offset, nullptr, 0);
}

void CrCommandBufferD3D12::DispatchIndirectPS(const ICrHardwareGPUBuffer* indirectBuffer, uint32_t offset)
{
	ID3D12CommandSignature* commandSignature = static_cast<const CrRenderDeviceD3D12*>(m_renderDevice)->GetD3D12DispatchIndirectCommandSignature();
	ID3D12Resource* resource = static_cast<const CrHardwareGPUBufferD3D12*>(indirectBuffer)->GetD3D12Resource();
	m_d3d12GraphicsCommandList->ExecuteIndirect(commandSignature, 1, resource, offset, nullptr, 0);
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

	// Copy shader resource descriptors
	{
		CrCommandBufferAssertMsg(m_shaderResourceCPUDescriptors.Size() == m_shaderResourceShaderVisibleDescriptorStream.GetNumDescriptors(), "Didn't allocate same descriptors");

		D3D12_CPU_DESCRIPTOR_HANDLE shaderResourceShaderVisibleHeap = m_shaderResourceShaderVisibleDescriptorStream.GetDescriptorHeap().GetHeapStartCPU();
		uint32_t shaderResourceDescriptorStride = m_shaderResourceShaderVisibleDescriptorStream.GetDescriptorHeap().GetDescriptorStride();

		for (uint32_t i = 0; i < m_shaderResourceCPUDescriptors.Size(); ++i)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE shaderVisibleDescriptor = shaderResourceShaderVisibleHeap;
			shaderVisibleDescriptor.ptr += i * shaderResourceDescriptorStride;

			d3d12RenderDevice->GetD3D12Device()->CopyDescriptorsSimple(1, shaderVisibleDescriptor, m_shaderResourceCPUDescriptors[i], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
	}
	
	{
		CrCommandBufferAssertMsg(m_samplerCPUDescriptors.Size() == m_samplerShaderVisibleDescriptorStream.GetNumDescriptors(), "Didn't allocate same descriptors");

		D3D12_CPU_DESCRIPTOR_HANDLE samplerShaderVisibleHeap = m_samplerShaderVisibleDescriptorStream.GetDescriptorHeap().GetHeapStartCPU();
		uint32_t samplerDescriptorStride = m_samplerShaderVisibleDescriptorStream.GetDescriptorHeap().GetDescriptorStride();

		for (uint32_t i = 0; i < m_samplerCPUDescriptors.Size(); ++i)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE shaderVisibleDescriptor = samplerShaderVisibleHeap;
			shaderVisibleDescriptor.ptr += i * samplerDescriptorStride;

			d3d12RenderDevice->GetD3D12Device()->CopyDescriptorsSimple(1, shaderVisibleDescriptor, m_samplerCPUDescriptors[i], D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		}
	}
}
