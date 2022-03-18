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
		rtvDescriptorHeapDescriptor.name = "RTV Descriptor";
		rtvDescriptorHeapDescriptor.numDescriptors = 1024;
		rtvDescriptorHeapDescriptor.type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		m_d3d12RTVHeap.Initialize(this, rtvDescriptorHeapDescriptor);
	}

	// Descriptor pool for depth stencil views
	{
		CrDescriptorHeapDescriptor dsvDescriptorHeapDescriptor;
		dsvDescriptorHeapDescriptor.name = "DSV Descriptor";
		dsvDescriptorHeapDescriptor.numDescriptors = 512;
		dsvDescriptorHeapDescriptor.type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		m_d3d12DSVHeap.Initialize(this, dsvDescriptorHeapDescriptor);
	}

	// Descriptor pool for samplers
	{
		CrDescriptorHeapDescriptor samplerDescriptorHeapDescriptor;
		samplerDescriptorHeapDescriptor.name = "Sampler Descriptor";
		samplerDescriptorHeapDescriptor.numDescriptors = 2048;
		samplerDescriptorHeapDescriptor.type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		m_d3d12SamplerHeap.Initialize(this, samplerDescriptorHeapDescriptor);
	}
}

CrRenderDeviceD3D12::~CrRenderDeviceD3D12()
{

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
	CrAssertMsg(true, "Not implemented");
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
	const CrGraphicsPipelineDescriptor& pipelineDescriptor, const ICrGraphicsShader* graphicsShader, const CrVertexDescriptor& vertexDescriptor
)
{
	return new CrGraphicsPipelineD3D12(this, pipelineDescriptor, graphicsShader, vertexDescriptor);
}

ICrComputePipeline* CrRenderDeviceD3D12::CreateComputePipelinePS(const CrComputePipelineDescriptor& /*pipelineDescriptor*/, const ICrComputeShader* computeShader)
{
	CrAssertMsg(true, "Not implemented");
	return new CrComputePipelineD3D12(this, computeShader);
}

ICrGPUQueryPool* CrRenderDeviceD3D12::CreateGPUQueryPoolPS(const CrGPUQueryPoolDescriptor& queryPoolDescriptor)
{
	CrAssertMsg(true, "Not implemented");
	unused_parameter(queryPoolDescriptor);
	return nullptr;
}

cr3d::GPUFenceResult CrRenderDeviceD3D12::WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds)
{
	CrAssertMsg(true, "Not implemented");
	unused_parameter(fence);
	unused_parameter(timeoutNanoseconds);
	return cr3d::GPUFenceResult::Success;
}

cr3d::GPUFenceResult CrRenderDeviceD3D12::GetFenceStatusPS(const ICrGPUFence* fence) const
{
	CrAssertMsg(true, "Not implemented");
	unused_parameter(fence);
	return cr3d::GPUFenceResult::Success;
}

void CrRenderDeviceD3D12::SignalFencePS(CrCommandQueueType::T queueType, const ICrGPUFence* signalFence)
{
	CrAssertMsg(true, "Not implemented");
	unused_parameter(queueType);
	unused_parameter(signalFence);
}

void CrRenderDeviceD3D12::ResetFencePS(const ICrGPUFence* fence)
{
	CrAssertMsg(true, "Not implemented");
	unused_parameter(fence);
}

void CrRenderDeviceD3D12::WaitIdlePS()
{
	CrAssertMsg(true, "Not implemented");
}

void CrRenderDeviceD3D12::SubmitCommandBufferPS(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence)
{
	CrAssertMsg(true, "Not implemented");
	unused_parameter(commandBuffer);
	unused_parameter(waitSemaphore);
	unused_parameter(signalSemaphore);
	unused_parameter(signalFence);
}
