#include "CrRendering_pch.h"

#include "CrRenderSystem_d3d12.h"
#include "CrRenderDevice_d3d12.h"

#include "CrCommandBuffer_d3d12.h"
#include "CrTexture_d3d12.h"
#include "CrSampler_d3d12.h"
#include "CrSwapchain_d3d12.h"
#include "CrGPUBuffer_d3d12.h"
#include "CrGPUSynchronization_d3d12.h"
#include "CrShader_d3d12.h"
#include "CrPipeline_d3d12.h"

#include "CrD3D12.h"

#include "Core/CrMacros.h"

#include "GeneratedShaders/BuiltinShaders.h"

CrRenderDeviceD3D12::CrRenderDeviceD3D12(const ICrRenderSystem* renderSystem) : ICrRenderDevice(renderSystem)
{
	const CrRenderSystemD3D12* d3d12RenderSystem = static_cast<const CrRenderSystemD3D12*>(renderSystem);
	IDXGIFactory4* dxgiFactory = d3d12RenderSystem->GetDXGIFactory();

	SIZE_T maxVideoMemory = 0;

	IDXGIAdapter1* dxgiAdapter = nullptr;
	DXGI_ADAPTER_DESC1 selectedAdapterDescriptor = {};

	for(UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		DXGI_ADAPTER_DESC1 adapterDescriptor;
		dxgiAdapter->GetDesc1(&adapterDescriptor);

		if (adapterDescriptor.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		if (adapterDescriptor.DedicatedVideoMemory > maxVideoMemory)
		{
			m_dxgiAdapter = dxgiAdapter;
			selectedAdapterDescriptor = adapterDescriptor;
			maxVideoMemory = adapterDescriptor.DedicatedVideoMemory;
		}
	}

	m_renderDeviceProperties.vendor = GetVendorFromVendorID(selectedAdapterDescriptor.VendorId);
	m_renderDeviceProperties.description.append_convert<wchar_t>(selectedAdapterDescriptor.Description);

	HRESULT hResult = S_OK;

	hResult = D3D12CreateDevice(m_dxgiAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_d3d12Device));
	CrAssertMsg(SUCCEEDED(hResult), "Error creating D3D12 device");

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = m_d3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_d3d12GraphicsCommandQueue));
	CrAssertMsg(SUCCEEDED(hResult), "Error creating command queue");

	if (ICrRenderSystem::GetIsValidationEnabled())
	{
		ID3D12InfoQueue* d3d12InfoQueue = NULL;
		if (SUCCEEDED(m_d3d12Device->QueryInterface(__uuidof(ID3D12InfoQueue), (void**)&d3d12InfoQueue)))
		{
			d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			//d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

			D3D12_MESSAGE_ID blockedIds[] =
			{
				D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE, // https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
			};
			D3D12_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.pIDList = blockedIds;
			filter.DenyList.NumIDs = sizeof_array(blockedIds);
			d3d12InfoQueue->AddRetrievalFilterEntries(&filter);
			d3d12InfoQueue->AddStorageFilterEntries(&filter);
			d3d12InfoQueue->Release();
		}
	}

	// Descriptor pool for render target views
	{
		CrDescriptorHeapDescriptor rtvDescriptorHeapDescriptor;
		rtvDescriptorHeapDescriptor.name = "RTV Descriptor Heap";
		rtvDescriptorHeapDescriptor.numDescriptors = 1024;
		rtvDescriptorHeapDescriptor.type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		m_rtvPool.Initialize(this, rtvDescriptorHeapDescriptor);
	}

	// Descriptor pool for depth stencil views
	{
		CrDescriptorHeapDescriptor dsvDescriptorHeapDescriptor;
		dsvDescriptorHeapDescriptor.name = "DSV Descriptor Heap";
		dsvDescriptorHeapDescriptor.numDescriptors = 512;
		dsvDescriptorHeapDescriptor.type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		m_dsvPool.Initialize(this, dsvDescriptorHeapDescriptor);
	}

	// Descriptor pool for samplers
	{
		CrDescriptorHeapDescriptor samplerDescriptorHeapDescriptor;
		samplerDescriptorHeapDescriptor.name = "Sampler Descriptor Heap";
		samplerDescriptorHeapDescriptor.numDescriptors = 2048;
		samplerDescriptorHeapDescriptor.type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		m_samplerPool.Initialize(this, samplerDescriptorHeapDescriptor);
	}

 	const CrShaderBytecodeSharedHandle& graphicsRootSignatureBytecode = ICrRenderSystem::GetBuiltinShaderBytecode(CrBuiltinShaders::RootSignatureGraphics);
	hResult = m_d3d12Device->CreateRootSignature(0, graphicsRootSignatureBytecode->GetBytecode().data(), graphicsRootSignatureBytecode->GetBytecode().size(),
		__uuidof(ID3D12RootSignature), (void**)&m_d3d12GraphicsRootSignature);
	CrAssertMsg(SUCCEEDED(hResult), "Error creating graphics root signature");

	const CrShaderBytecodeSharedHandle& computeRootSignatureBytecode = ICrRenderSystem::GetBuiltinShaderBytecode(CrBuiltinShaders::RootSignatureCompute);
	hResult = m_d3d12Device->CreateRootSignature(0, computeRootSignatureBytecode->GetBytecode().data(), computeRootSignatureBytecode->GetBytecode().size(),
		__uuidof(ID3D12RootSignature), (void**)&m_d3d12ComputeRootSignature);
	CrAssertMsg(SUCCEEDED(hResult), "Error creating compute root signature");

	m_waitIdleFence = CreateGPUFence();
}

CrRenderDeviceD3D12::~CrRenderDeviceD3D12()
{

}

void CrRenderDeviceD3D12::SetD3D12ObjectName(ID3D12Object* object, const char* name)
{
	CrFixedWString128 wName;
	wName.append_convert<char>(name);
	object->SetName(wName.c_str());
}

ICrCommandBuffer* CrRenderDeviceD3D12::CreateCommandBufferPS(CrCommandQueueType::T type)
{
	return new CrCommandBufferD3D12(this, type);
}

ICrGPUFence* CrRenderDeviceD3D12::CreateGPUFencePS()
{
	return new CrGPUFenceD3D12(this);
}

ICrGPUSemaphore* CrRenderDeviceD3D12::CreateGPUSemaphorePS()
{
	CrAssertMsg(false, "Not implemented");
	return nullptr;
}

ICrGraphicsShader* CrRenderDeviceD3D12::CreateGraphicsShaderPS(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor)
{
	return new CrGraphicsShaderD3D12(this, graphicsShaderDescriptor);
}

ICrComputeShader* CrRenderDeviceD3D12::CreateComputeShaderPS(const CrComputeShaderDescriptor& computeShaderDescriptor)
{
	return new CrComputeShaderD3D12(this, computeShaderDescriptor);
}

ICrHardwareGPUBuffer* CrRenderDeviceD3D12::CreateHardwareGPUBufferPS(const CrHardwareGPUBufferDescriptor& descriptor)
{
	return new CrHardwareGPUBufferD3D12(this, descriptor);
}

ICrSampler* CrRenderDeviceD3D12::CreateSamplerPS(const CrSamplerDescriptor& descriptor)
{
	return new CrSamplerD3D12(this, descriptor);
}

ICrSwapchain* CrRenderDeviceD3D12::CreateSwapchainPS(const CrSwapchainDescriptor& swapchainDescriptor)
{
	return new CrSwapchainD3D12(this, swapchainDescriptor);
}

ICrTexture* CrRenderDeviceD3D12::CreateTexturePS(const CrTextureDescriptor& params)
{
	return new CrTextureD3D12(this, params);
}

ICrGraphicsPipeline* CrRenderDeviceD3D12::CreateGraphicsPipelinePS
(
	const CrGraphicsPipelineDescriptor& pipelineDescriptor, const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor
)
{
	return new CrGraphicsPipelineD3D12(this, pipelineDescriptor, graphicsShader, vertexDescriptor);
}

ICrComputePipeline* CrRenderDeviceD3D12::CreateComputePipelinePS(const CrComputePipelineDescriptor& /*pipelineDescriptor*/, const CrComputeShaderHandle& computeShader)
{
	return new CrComputePipelineD3D12(this, computeShader);
}

ICrGPUQueryPool* CrRenderDeviceD3D12::CreateGPUQueryPoolPS(const CrGPUQueryPoolDescriptor& queryPoolDescriptor)
{
	return new CrGPUQueryPoolD3D12(this, queryPoolDescriptor);
}

cr3d::GPUFenceResult CrRenderDeviceD3D12::WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds)
{
	const CrGPUFenceD3D12* d3dFence = static_cast<const CrGPUFenceD3D12*>(fence);
	ID3D12Fence* d3d12Fence = d3dFence->GetD3D12Fence();

	DWORD waitResult = 0;
	UINT64 currentValue = d3d12Fence->GetCompletedValue();

	if (currentValue < 1)
	{
		d3d12Fence->SetEventOnCompletion(1, d3dFence->GetFenceEvent());
		waitResult = WaitForSingleObjectEx(d3dFence->GetFenceEvent(), (DWORD)timeoutNanoseconds, FALSE);
	}

	if (waitResult == 0)
	{
		return cr3d::GPUFenceResult::Success;
	}
	else
	{
		return cr3d::GPUFenceResult::TimeoutOrNotReady;
	}
}

cr3d::GPUFenceResult CrRenderDeviceD3D12::GetFenceStatusPS(const ICrGPUFence* fence) const
{
	const CrGPUFenceD3D12* d3dFence = static_cast<const CrGPUFenceD3D12*>(fence);

	if (d3dFence->GetD3D12Fence()->GetCompletedValue() == 1)
	{
		return cr3d::GPUFenceResult::Success;
	}
	else
	{
		return cr3d::GPUFenceResult::TimeoutOrNotReady;
	}
}

void CrRenderDeviceD3D12::SignalFencePS(CrCommandQueueType::T queueType, const ICrGPUFence* fence)
{
	const CrGPUFenceD3D12* d3dFence = static_cast<const CrGPUFenceD3D12*>(fence);

	// To signal a fence, we set its value to 1. Note that we're using fences like Vulkan
	// uses fences, there are no values
	if (queueType == CrCommandQueueType::Graphics)
	{
		m_d3d12GraphicsCommandQueue->Signal(d3dFence->GetD3D12Fence(), 1);
	}
	else
	{
		CrAssertMsg(false, "Not implemented");
	}
}

void CrRenderDeviceD3D12::ResetFencePS(const ICrGPUFence* fence)
{
	// To reset the fence we signal it on the CPU side back to 0, and also
	// manually reset the event. This is manual so we can wait for the same
	// fence in multiple sites
	const CrGPUFenceD3D12* d3dFence = static_cast<const CrGPUFenceD3D12*>(fence);
	d3dFence->GetD3D12Fence()->Signal(0);
	ResetEvent(d3dFence->GetFenceEvent());
}

void CrRenderDeviceD3D12::WaitIdlePS()
{
	// Signal a fence and immediately wait for it
	SignalFence(CrCommandQueueType::Graphics, m_waitIdleFence.get());

	WaitForFence(m_waitIdleFence.get(), UINT64_MAX);
}

void CrRenderDeviceD3D12::SubmitCommandBufferPS(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence)
{
	unused_parameter(waitSemaphore);
	unused_parameter(signalSemaphore);

	// Signal fence so we can wait for it on next use
	SignalFencePS(CrCommandQueueType::Graphics, signalFence);

	const CrCommandBufferD3D12* d3d12CommandBuffer = static_cast<const CrCommandBufferD3D12*>(commandBuffer);

	ID3D12CommandList* d3d12CommandList = { d3d12CommandBuffer->GetD3D12CommandList() };
	m_d3d12GraphicsCommandQueue->ExecuteCommandLists(1, &d3d12CommandList);
}
