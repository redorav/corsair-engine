#include "Rendering/CrRendering_pch.h"

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

#include "Math/CrMath.h"

#if defined(CR_PLATFORM_WINDOWS)
#define USE_AGILITY_SDK
#endif

#if defined(USE_AGILITY_SDK)

// Let the application know we want to look for the Agility SDK
// https://devblogs.microsoft.com/directx/gettingstarted-dx12agility/#parametersa
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 614; }

extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }

#endif

// Good documentation regarding how to create the device can be found here
// https://walbourn.github.io/anatomy-of-direct3d-12-create-device/

const char* GetD3DFeatureLevelString(D3D_FEATURE_LEVEL featureLevel)
{
	switch (featureLevel)
	{
		case D3D_FEATURE_LEVEL_12_0: return "12.0";
		case D3D_FEATURE_LEVEL_12_1: return "12.1";
		case D3D_FEATURE_LEVEL_12_2: return "12.2";
		default: return "";
	}
}

CrRenderDeviceD3D12::CrRenderDeviceD3D12(ICrRenderSystem* renderSystem, const CrRenderDeviceDescriptor& descriptor) : ICrRenderDevice(renderSystem, descriptor)
{
	CrRenderSystemD3D12* d3d12RenderSystem = static_cast<CrRenderSystemD3D12*>(renderSystem);
	IDXGIFactory4* dxgiFactory4 = d3d12RenderSystem->GetDXGIFactory4();
	
	struct PriorityKey
	{
		uint64_t deviceMemory = 0;
		uint32_t priority = 0;

		bool operator < (const PriorityKey& other)
		{
			return (deviceMemory < other.deviceMemory) ? true : (priority < other.priority) ? true : false;
		}
	};

	struct SelectedDevice
	{
		PriorityKey priorityKey;
		IDXGIAdapter1* dxgiAdapter = nullptr;
		DXGI_ADAPTER_DESC3 descriptor;
	};

	SelectedDevice maxPriorityDevice;
	SelectedDevice maxPriorityPreferredDevice;

	cr3d::GraphicsVendor::T preferredVendor = descriptor.preferredVendor;

	IDXGIAdapter1* dxgiAdapter1 = nullptr;
	IDXGIAdapter2* dxgiAdapter2 = nullptr;
	IDXGIAdapter4* dxgiAdapter4 = nullptr;

	for(UINT i = 0; dxgiFactory4->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		// Provide a union here as they are incremental
		union
		{
			DXGI_ADAPTER_DESC1 adapterDescriptor1;
			DXGI_ADAPTER_DESC2 adapterDescriptor2;
			DXGI_ADAPTER_DESC3 adapterDescriptor3 = {};
		};

		if (dxgiAdapter1->QueryInterface(IID_PPV_ARGS(&dxgiAdapter4)) == S_OK)
		{
			dxgiAdapter4->GetDesc3(&adapterDescriptor3);
		}
		else if (dxgiAdapter1->QueryInterface(IID_PPV_ARGS(&dxgiAdapter2)) == S_OK)
		{
			dxgiAdapter2->GetDesc2(&adapterDescriptor2);
		}
		else
		{
			dxgiAdapter1->GetDesc1(&adapterDescriptor1);
		}

		cr3d::GraphicsVendor::T graphicsVendor = cr3d::GraphicsVendor::FromVendorID(adapterDescriptor3.VendorId);

		bool isSoftwareRenderer = adapterDescriptor3.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE;

		PriorityKey priorityKey;
		priorityKey.priority |= !isSoftwareRenderer ? 1u << 31u : 0u;
		priorityKey.deviceMemory = adapterDescriptor3.DedicatedVideoMemory;

		if (maxPriorityDevice.priorityKey < priorityKey)
		{
			maxPriorityDevice.priorityKey = priorityKey;
			maxPriorityDevice.dxgiAdapter = dxgiAdapter1;
			maxPriorityDevice.descriptor = adapterDescriptor3;
		}

		if (graphicsVendor == preferredVendor)
		{
			if (maxPriorityPreferredDevice.priorityKey < priorityKey)
			{
				maxPriorityPreferredDevice.priorityKey = priorityKey;
				maxPriorityPreferredDevice.dxgiAdapter = dxgiAdapter1;
				maxPriorityPreferredDevice.descriptor = adapterDescriptor3;
			}
		}
	}

	DXGI_ADAPTER_DESC3 adapterDescriptor = {};

	if (maxPriorityPreferredDevice.dxgiAdapter != nullptr)
	{
		m_dxgiAdapter = maxPriorityPreferredDevice.dxgiAdapter;
		adapterDescriptor = maxPriorityPreferredDevice.descriptor;
	}
	else
	{
		m_dxgiAdapter = maxPriorityDevice.dxgiAdapter;
		adapterDescriptor = maxPriorityDevice.descriptor;
	}

	HRESULT hResult = S_OK;

	if (D3D12CreateDevice(m_dxgiAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_d3d12Device)) != S_OK)
	{
		CrAssertMsg(false, "Unable to create device");
	}

	CrAssertMsg(m_d3d12Device != nullptr, "Error creating D3D12 device");

	D3D_FEATURE_LEVEL supportedFeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0
	};

	m_d3d12Device->QueryInterface(IID_PPV_ARGS(&m_d3d12Device1));
	m_d3d12Device->QueryInterface(IID_PPV_ARGS(&m_d3d12Device2));
	m_d3d12Device->QueryInterface(IID_PPV_ARGS(&m_d3d12Device3));
	m_d3d12Device->QueryInterface(IID_PPV_ARGS(&m_d3d12Device4));
	m_d3d12Device->QueryInterface(IID_PPV_ARGS(&m_d3d12Device5));
	m_d3d12Device->QueryInterface(IID_PPV_ARGS(&m_d3d12Device6));
	m_d3d12Device->QueryInterface(IID_PPV_ARGS(&m_d3d12Device7));
	m_d3d12Device->QueryInterface(IID_PPV_ARGS(&m_d3d12Device8));
	m_d3d12Device->QueryInterface(IID_PPV_ARGS(&m_d3d12Device9));
	m_d3d12Device->QueryInterface(IID_PPV_ARGS(&m_d3d12Device10));
	m_d3d12Device->QueryInterface(IID_PPV_ARGS(&m_d3d12Device11));
	m_d3d12Device->QueryInterface(IID_PPV_ARGS(&m_d3d12Device12));
	m_d3d12Device->QueryInterface(IID_PPV_ARGS(&m_d3d12Device13));
	m_d3d12Device->QueryInterface(IID_PPV_ARGS(&m_d3d12Device14));

	D3D12_FEATURE_DATA_FEATURE_LEVELS featureLevels = {};
	featureLevels.NumFeatureLevels = (UINT)crstl::array_size(supportedFeatureLevels);
	featureLevels.pFeatureLevelsRequested = supportedFeatureLevels;
	m_d3d12Device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featureLevels, sizeof(featureLevels));

#if defined(USE_AGILITY_SDK)

	bool usingAgility = false;

	CrFixedPath agilityPath;

	HMODULE hModule = GetModuleHandle(L"D3D12Core.dll");
	if (hModule)
	{
		TCHAR dllPath[260];
		GetModuleFileName(hModule, dllPath, 260);
		agilityPath.append_convert(dllPath);
	}

	usingAgility = agilityPath != "D3D12Core.dll";

#endif

	// Parse render device properties
	m_renderDeviceProperties.vendor = cr3d::GraphicsVendor::FromVendorID(adapterDescriptor.VendorId);
	m_renderDeviceProperties.graphicsApiDisplay.append_sprintf("%s %s", cr3d::GraphicsApi::ToString(m_renderDeviceProperties.graphicsApi), GetD3DFeatureLevelString(featureLevels.MaxSupportedFeatureLevel));

#if defined(USE_AGILITY_SDK)
	if (usingAgility)
	{
		m_renderDeviceProperties.graphicsApiDisplay.append_sprintf(", Agility SDK %i", D3D12SDKVersion);
	}
#endif

	m_renderDeviceProperties.description.append_convert<wchar_t>(adapterDescriptor.Description);
	m_renderDeviceProperties.gpuMemoryBytes = adapterDescriptor.DedicatedVideoMemory;

	// Check architecture
	D3D12_FEATURE_DATA_ARCHITECTURE d3d12Architecture;
	d3d12Architecture.NodeIndex = 0;
	if (m_d3d12Device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE, &d3d12Architecture, sizeof(d3d12Architecture)) == S_OK)
	{
		m_renderDeviceProperties.isUMA = d3d12Architecture.UMA;
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS d3d12Options = {};
	if (m_d3d12Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &d3d12Options, sizeof(d3d12Options)) == S_OK)
	{
		m_renderDeviceProperties.features.conservativeRasterization = d3d12Options.ConservativeRasterizationTier >= D3D12_CONSERVATIVE_RASTERIZATION_TIER_1;
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS5 d3d12Options5 = {};
	if (m_d3d12Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &d3d12Options5, sizeof(d3d12Options5)) == S_OK)
	{
		m_renderDeviceProperties.features.raytracing = d3d12Options5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0;
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS6 d3d12Options6 = {};
	if (m_d3d12Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &d3d12Options6, sizeof(d3d12Options6)) == S_OK)
	{
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS7 d3d12Options7 = {};
	if (m_d3d12Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &d3d12Options7, sizeof(d3d12Options7)) == S_OK)
	{
		m_renderDeviceProperties.features.meshShaders = d3d12Options7.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1;
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS12 d3d12Options12 = {};
	if (m_d3d12Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &d3d12Options12, sizeof(d3d12Options12)) == S_OK)
	{
		m_enhancedBarriersSupported = d3d12Options12.EnhancedBarriersSupported;
	}

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	hResult = m_d3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_d3d12GraphicsCommandQueue));
	CrAssertMsg(SUCCEEDED(hResult), "Error creating command queue");

	if (RenderSystem->GetIsValidationEnabled())
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

	// Create root signatures

 	const CrShaderBytecodeHandle& graphicsRootSignatureBytecode = renderSystem->GetBuiltinShaderBytecode(CrBuiltinShaders::RootSignatureGraphics);
	hResult = m_d3d12Device->CreateRootSignature(0, graphicsRootSignatureBytecode->GetBytecode().data(), graphicsRootSignatureBytecode->GetBytecode().size(),
		__uuidof(ID3D12RootSignature), (void**)&m_d3d12GraphicsRootSignature);
	CrAssertMsg(SUCCEEDED(hResult), "Error creating graphics root signature");

	const CrShaderBytecodeHandle& computeRootSignatureBytecode = renderSystem->GetBuiltinShaderBytecode(CrBuiltinShaders::RootSignatureCompute);
	hResult = m_d3d12Device->CreateRootSignature(0, computeRootSignatureBytecode->GetBytecode().data(), computeRootSignatureBytecode->GetBytecode().size(),
		__uuidof(ID3D12RootSignature), (void**)&m_d3d12ComputeRootSignature);
	CrAssertMsg(SUCCEEDED(hResult), "Error creating compute root signature");

	// Create command signatures

	// DrawIndirect
	{
		D3D12_INDIRECT_ARGUMENT_DESC indirectArgument;
		indirectArgument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;

		D3D12_COMMAND_SIGNATURE_DESC commandSignatureDescriptor = {};
		commandSignatureDescriptor.ByteStride = sizeof(D3D12_DRAW_ARGUMENTS);
		commandSignatureDescriptor.NumArgumentDescs = 1;
		commandSignatureDescriptor.pArgumentDescs = &indirectArgument;

		hResult = m_d3d12Device->CreateCommandSignature(&commandSignatureDescriptor, nullptr, __uuidof(ID3D12CommandSignature), (void**)&m_d3d12DrawIndirectCommandSignature);
		CrAssertMsg(hResult == S_OK, "Error creating command signature");
	}

	// DrawIndexedIndirect
	{
		D3D12_INDIRECT_ARGUMENT_DESC indirectArgument;
		indirectArgument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

		D3D12_COMMAND_SIGNATURE_DESC commandSignatureDescriptor = {};
		commandSignatureDescriptor.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
		commandSignatureDescriptor.NumArgumentDescs = 1;
		commandSignatureDescriptor.pArgumentDescs = &indirectArgument;

		hResult = m_d3d12Device->CreateCommandSignature(&commandSignatureDescriptor, nullptr, __uuidof(ID3D12CommandSignature), (void**)&m_d3d12DrawIndexedIndirectCommandSignature);
		CrAssertMsg(hResult == S_OK, "Error creating command signature");
	}
	
	// DispatchIndirect
	{
		D3D12_INDIRECT_ARGUMENT_DESC indirectArgument;
		indirectArgument.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

		D3D12_COMMAND_SIGNATURE_DESC commandSignatureDescriptor = {};
		commandSignatureDescriptor.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);
		commandSignatureDescriptor.NumArgumentDescs = 1;
		commandSignatureDescriptor.pArgumentDescs = &indirectArgument;

		hResult = m_d3d12Device->CreateCommandSignature(&commandSignatureDescriptor, nullptr, __uuidof(ID3D12CommandSignature), (void**)&m_d3d12DispatchIndirectCommandSignature);
		CrAssertMsg(hResult == S_OK, "Error creating command signature");
	}

	m_waitIdleFence = CreateGPUFence();
}

CrRenderDeviceD3D12::~CrRenderDeviceD3D12()
{

}

void CrRenderDeviceD3D12::SetD3D12ObjectName(ID3D12Object* object, const char* name)
{
	if (name && name[0] != 0)
	{
		crstl::fixed_wstring128 wName;
		wName.append_convert<char>(name);
		object->SetName(wName.c_str());
	}
}

ICrCommandBuffer* CrRenderDeviceD3D12::CreateCommandBufferPS(const CrCommandBufferDescriptor& descriptor)
{
	return new CrCommandBufferD3D12(this, descriptor);
}

ICrGPUFence* CrRenderDeviceD3D12::CreateGPUFencePS(bool signaled)
{
	return new CrGPUFenceD3D12(this, signaled);
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

ICrComputePipeline* CrRenderDeviceD3D12::CreateComputePipelinePS(const CrComputeShaderHandle& computeShader)
{
	return new CrComputePipelineD3D12(this, computeShader);
}

ICrGPUQueryPool* CrRenderDeviceD3D12::CreateGPUQueryPoolPS(const CrGPUQueryPoolDescriptor& queryPoolDescriptor)
{
	return new CrGPUQueryPoolD3D12(this, queryPoolDescriptor);
}

void CrRenderDeviceD3D12::FinalizeDeletionPS()
{
	m_waitIdleFence = nullptr;
}

cr3d::GPUFenceResult CrRenderDeviceD3D12::WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds)
{
	const CrGPUFenceD3D12* d3dFence = static_cast<const CrGPUFenceD3D12*>(fence);
	ID3D12Fence* d3d12Fence = d3dFence->GetD3D12Fence();

	DWORD waitResult = 0;
	UINT64 currentValue = d3d12Fence->GetCompletedValue();

	if (currentValue < 1)
	{
		HRESULT hResult = d3d12Fence->SetEventOnCompletion(1, d3dFence->GetFenceEvent());
		CrAssertMsg(SUCCEEDED(hResult), "Failed SetEventOnCompletion");
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
	// Reset wait idle fence
	ResetFence(m_waitIdleFence.get());

	// Signal a fence and immediately wait for it
	SignalFence(CrCommandQueueType::Graphics, m_waitIdleFence.get());

	WaitForFence(m_waitIdleFence.get(), UINT64_MAX);
}

uint8_t* CrRenderDeviceD3D12::BeginTextureUploadPS(const ICrTexture* texture)
{
	const CrTextureD3D12* d3d12Texture = static_cast<const CrTextureD3D12*>(texture);

	D3D12_RESOURCE_DESC resourceDescriptor = d3d12Texture->GetD3D12Resource()->GetDesc();

	UINT64 stagingBufferSizeBytes;
	m_d3d12Device->GetCopyableFootprints(&resourceDescriptor, 0, d3d12Texture->GetD3D12SubresourceCount(), 0, nullptr, nullptr, nullptr, &stagingBufferSizeBytes);

	CrHardwareGPUBufferDescriptor stagingBufferDescriptor(cr3d::BufferUsage::TransferSrc, cr3d::MemoryAccess::StagingUpload, (uint32_t)stagingBufferSizeBytes);
	stagingBufferDescriptor.name = "Texture Upload Staging Buffer";

	CrTextureUpload textureUpload;
	textureUpload.stagingBuffer = CreateHardwareGPUBuffer(stagingBufferDescriptor);
	textureUpload.mipmapStart = 0;
	textureUpload.mipmapCount = texture->GetMipmapCount();
	textureUpload.sliceStart = 0;
	textureUpload.sliceCount = texture->GetSliceCount();

	CrHash textureHash(&texture, sizeof(texture));

	// Add to the open uploads for when we end the texture upload
	m_openTextureUploads.insert(textureHash, textureUpload);

	return (uint8_t*)static_cast<CrHardwareGPUBufferD3D12*>(textureUpload.stagingBuffer.get())->Lock();
}

void CrRenderDeviceD3D12::EndTextureUploadPS(const ICrTexture* destinationTexture)
{
	CrHash textureHash(&destinationTexture, sizeof(destinationTexture));
	const auto textureUploadIter = m_openTextureUploads.find(textureHash);
	CrAssertMsg(textureUploadIter != m_openTextureUploads.end(), "Tried ending texture upload with no begin");

	const CrTextureUpload& textureUpload = textureUploadIter->second;

	const CrTextureD3D12* d3d12DestinationTexture = static_cast<const CrTextureD3D12*>(destinationTexture);
	CrHardwareGPUBufferD3D12* d3d12StagingBuffer = static_cast<CrHardwareGPUBufferD3D12*>(textureUpload.stagingBuffer.get());

	d3d12StagingBuffer->Unlock();

	CrCommandBufferD3D12* d3d12CommandBuffer = static_cast<CrCommandBufferD3D12*>(GetAuxiliaryCommandBuffer().get());
	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = d3d12DestinationTexture->GetD3D12Resource();
		barrier.Transition.StateBefore = d3d12DestinationTexture->GetD3D12DefaultLegacyResourceState();
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		D3D12_TEXTURE_BARRIER d3d12TextureBarrier;
		d3d12TextureBarrier.SyncBefore                        = D3D12_BARRIER_SYNC_NONE;
		d3d12TextureBarrier.SyncAfter                         = D3D12_BARRIER_SYNC_COPY;
		d3d12TextureBarrier.AccessBefore                      = D3D12_BARRIER_ACCESS_NO_ACCESS;
		d3d12TextureBarrier.AccessAfter                       = D3D12_BARRIER_ACCESS_COPY_DEST;
		d3d12TextureBarrier.LayoutBefore                      = D3D12_BARRIER_LAYOUT_UNDEFINED;
		d3d12TextureBarrier.LayoutAfter                       = D3D12_BARRIER_LAYOUT_COPY_DEST;
		d3d12TextureBarrier.pResource                         = d3d12DestinationTexture->GetD3D12Resource();
		d3d12TextureBarrier.Subresources.IndexOrFirstMipLevel = 0;
		d3d12TextureBarrier.Subresources.NumMipLevels         = d3d12DestinationTexture->GetMipmapCount();
		d3d12TextureBarrier.Subresources.FirstArraySlice      = 0;
		d3d12TextureBarrier.Subresources.NumArraySlices       = d3d12DestinationTexture->GetSliceCount();
		d3d12TextureBarrier.Subresources.FirstPlane           = 0;
		d3d12TextureBarrier.Subresources.NumPlanes            = 1;
		d3d12TextureBarrier.Flags                             = D3D12_TEXTURE_BARRIER_FLAG_NONE; // TODO Handle discard when we have transient resources

		if (m_enhancedBarriersSupported)
		{
			D3D12_BARRIER_GROUP textureBarrierGroup;
			textureBarrierGroup.Type = D3D12_BARRIER_TYPE_TEXTURE;
			textureBarrierGroup.NumBarriers = 1;
			textureBarrierGroup.pTextureBarriers = &d3d12TextureBarrier;
			d3d12CommandBuffer->GetD3D12CommandList7()->Barrier(1, &textureBarrierGroup);
		}
		else
		{
			d3d12CommandBuffer->GetD3D12CommandList()->ResourceBarrier(1, &barrier);
		}

		cr3d::DataFormat::T format = destinationTexture->GetFormat();

		uint32_t blockWidth = cr3d::DataFormats[format].blockWidth;
		uint32_t blockHeight = cr3d::DataFormats[format].blockHeight;

		for (uint32_t slice = textureUpload.sliceStart; slice < textureUpload.sliceCount; ++slice)
		{
			for (uint32_t mip = textureUpload.mipmapStart; mip < textureUpload.mipmapCount; ++mip)
			{
				cr3d::MipmapLayout mipmapLayout = destinationTexture->GetHardwareMipSliceLayout(mip, slice);

				D3D12_TEXTURE_COPY_LOCATION textureCopySource = {};
				textureCopySource.pResource = d3d12StagingBuffer->GetD3D12Resource();
				textureCopySource.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				textureCopySource.PlacedFootprint.Offset = mipmapLayout.offsetBytes;
				textureCopySource.PlacedFootprint.Footprint.Format   = crd3d::GetDXGIFormat(format);
				textureCopySource.PlacedFootprint.Footprint.Width    = CrMax(blockWidth, destinationTexture->GetWidth() >> mip);
				textureCopySource.PlacedFootprint.Footprint.Height   = CrMax(blockHeight, destinationTexture->GetHeight() >> mip);
				textureCopySource.PlacedFootprint.Footprint.Depth    = CrMax(1u, destinationTexture->GetDepth() >> mip);
				textureCopySource.PlacedFootprint.Footprint.RowPitch = mipmapLayout.rowPitchBytes;

				D3D12_TEXTURE_COPY_LOCATION textureCopyDestination = {};
				textureCopyDestination.pResource = d3d12DestinationTexture->GetD3D12Resource();
				textureCopyDestination.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				textureCopyDestination.SubresourceIndex = crd3d::CalculateSubresource(mip, slice, 0, destinationTexture->GetMipmapCount(), destinationTexture->GetSliceCount());

				d3d12CommandBuffer->GetD3D12CommandList()->CopyTextureRegion(&textureCopyDestination, 0, 0, 0, &textureCopySource, nullptr);
			}
		}

		if (m_enhancedBarriersSupported)
		{
			d3d12TextureBarrier.SyncBefore   = D3D12_BARRIER_SYNC_COPY;
			d3d12TextureBarrier.SyncAfter    = D3D12_BARRIER_SYNC_ALL;
			d3d12TextureBarrier.AccessBefore = D3D12_BARRIER_ACCESS_COPY_DEST;
			d3d12TextureBarrier.AccessAfter  = D3D12_BARRIER_ACCESS_COMMON;
			d3d12TextureBarrier.LayoutBefore = D3D12_BARRIER_LAYOUT_COPY_DEST;
			d3d12TextureBarrier.LayoutAfter  = d3d12DestinationTexture->GetD3D12DefaultTextureLayout();

			D3D12_BARRIER_GROUP textureBarrierGroup;
			textureBarrierGroup.Type = D3D12_BARRIER_TYPE_TEXTURE;
			textureBarrierGroup.NumBarriers = 1;
			textureBarrierGroup.pTextureBarriers = &d3d12TextureBarrier;

			d3d12CommandBuffer->GetD3D12CommandList7()->Barrier(1, &textureBarrierGroup);
		}
		else
		{
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier.Transition.StateAfter = d3d12DestinationTexture->GetD3D12DefaultLegacyResourceState();
			d3d12CommandBuffer->GetD3D12CommandList()->ResourceBarrier(1, &barrier);
		}
	}

	m_openTextureUploads.erase(textureUploadIter);
}

uint8_t* CrRenderDeviceD3D12::BeginBufferUploadPS(const ICrHardwareGPUBuffer* destinationBuffer)
{
	uint32_t stagingBufferSizeBytes = destinationBuffer->GetSizeBytes();

	CrHardwareGPUBufferDescriptor stagingBufferDescriptor(cr3d::BufferUsage::TransferSrc, cr3d::MemoryAccess::StagingUpload, stagingBufferSizeBytes);
	stagingBufferDescriptor.name = "Buffer Upload Staging Buffer";

	CrBufferUpload bufferUpload;
	bufferUpload.stagingBuffer = CreateHardwareGPUBuffer(stagingBufferDescriptor);
	bufferUpload.destinationBuffer = destinationBuffer;
	bufferUpload.sizeBytes = destinationBuffer->GetSizeBytes();
	bufferUpload.sourceOffsetBytes = 0;
	bufferUpload.destinationOffsetBytes = 0; // TODO Add as parameter

	CrHash bufferHash(&destinationBuffer, sizeof(destinationBuffer));

	// Add to the open uploads for when we end the texture upload
	m_openBufferUploads.insert(bufferHash, bufferUpload);

	return (uint8_t*)static_cast<CrHardwareGPUBufferD3D12*>(bufferUpload.stagingBuffer.get())->Lock();
}

void CrRenderDeviceD3D12::EndBufferUploadPS(const ICrHardwareGPUBuffer* destinationBuffer)
{
	CrHash bufferHash(&destinationBuffer, sizeof(destinationBuffer));
	const auto bufferUploadIter = m_openBufferUploads.find(bufferHash);
	CrAssertMsg(bufferUploadIter != m_openBufferUploads.end(), "Tried ending buffer upload with no begin");

	const CrBufferUpload& bufferUpload = bufferUploadIter->second;

	const CrHardwareGPUBufferD3D12* d3d12DestinationBuffer = static_cast<const CrHardwareGPUBufferD3D12*>(destinationBuffer);
	CrHardwareGPUBufferD3D12* d3d12StagingBuffer = static_cast<CrHardwareGPUBufferD3D12*>(bufferUpload.stagingBuffer.get());

	d3d12StagingBuffer->Unlock();

	CrCommandBufferD3D12* d3d12CommandBuffer = static_cast<CrCommandBufferD3D12*>(GetAuxiliaryCommandBuffer().get());
	{
		// TODO Consider using CopyResource when copying the entire resource

		d3d12CommandBuffer->GetD3D12CommandList()->CopyBufferRegion
		(
			d3d12DestinationBuffer->GetD3D12Resource(), bufferUpload.destinationOffsetBytes, 
			d3d12StagingBuffer->GetD3D12Resource(), bufferUpload.sourceOffsetBytes,
			bufferUpload.sizeBytes
		);
	}

	m_openBufferUploads.erase(bufferUploadIter);
}

CrHardwareGPUBufferHandle CrRenderDeviceD3D12::DownloadBufferPS(const ICrHardwareGPUBuffer* sourceBuffer)
{
	const CrHardwareGPUBufferD3D12* d3d12SourceBuffer = static_cast<const CrHardwareGPUBufferD3D12*>(sourceBuffer);

	D3D12_RESOURCE_DESC resourceDescriptor = d3d12SourceBuffer->GetD3D12Resource()->GetDesc();

	// If a resource contains a buffer, then it simply contains one subresource with an index of 0

	UINT64 stagingBufferSizeBytes;
	m_d3d12Device->GetCopyableFootprints(&resourceDescriptor, 0, 1, 0, nullptr, nullptr, nullptr, &stagingBufferSizeBytes);

	CrHardwareGPUBufferDescriptor stagingBufferDescriptor(cr3d::BufferUsage::TransferDst, cr3d::MemoryAccess::StagingDownload, (uint32_t)stagingBufferSizeBytes);
	CrHardwareGPUBufferHandle stagingBuffer = CreateHardwareGPUBuffer(stagingBufferDescriptor);
	CrHardwareGPUBufferD3D12* d3d12StagingBuffer = static_cast<CrHardwareGPUBufferD3D12*>(stagingBuffer.get());

	CrCommandBufferD3D12* d3d12CommandBuffer = static_cast<CrCommandBufferD3D12*>(GetAuxiliaryCommandBuffer().get());
	{
		// We assume the staging buffer is in the copy destination state, as we've created it in here
		CrAssertMsg(d3d12StagingBuffer->GetDefaultResourceState() == D3D12_RESOURCE_STATE_COPY_DEST, "Staging buffer in incorrect resource state");

		D3D12_RESOURCE_BARRIER barrier = {};

		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = d3d12SourceBuffer->GetD3D12Resource();
		barrier.Transition.StateBefore = d3d12SourceBuffer->GetDefaultResourceState();
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;

		d3d12CommandBuffer->GetD3D12CommandList()->ResourceBarrier(1, &barrier);

		d3d12CommandBuffer->GetD3D12CommandList()->CopyBufferRegion
		(
			d3d12StagingBuffer->GetD3D12Resource(), 0,
			d3d12SourceBuffer->GetD3D12Resource(), 0,
			stagingBufferSizeBytes
		);

		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.StateAfter = d3d12SourceBuffer->GetDefaultResourceState();

		d3d12CommandBuffer->GetD3D12CommandList()->ResourceBarrier(1, &barrier);
	}

	return stagingBuffer;
}

void CrRenderDeviceD3D12::SubmitCommandBufferPS(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence)
{
	unused_parameter(waitSemaphore);
	unused_parameter(signalSemaphore);

	const CrCommandBufferD3D12* d3d12CommandBuffer = static_cast<const CrCommandBufferD3D12*>(commandBuffer);

	ID3D12CommandList* d3d12CommandList = { d3d12CommandBuffer->GetD3D12CommandList() };
	m_d3d12GraphicsCommandQueue->ExecuteCommandLists(1, &d3d12CommandList);

	// Signal fence so we can wait for it on next use
	SignalFencePS(CrCommandQueueType::Graphics, signalFence);
}
