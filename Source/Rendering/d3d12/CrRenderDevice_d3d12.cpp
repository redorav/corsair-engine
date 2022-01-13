#include "CrRendering_pch.h"

#include "CrRenderSystem_d3d12.h"
#include "CrRenderDevice_d3d12.h"

#include "CrCommandQueue_d3d12.h"
#include "CrCommandBuffer_d3d12.h"
#include "CrTexture_d3d12.h"
#include "CrSampler_d3d12.h"
#include "CrSwapchain_d3d12.h"
#include "CrGPUBuffer_d3d12.h"
#include "CrGPUSynchronization_d3d12.h"
#include "CrShader_d3d12.h"
#include "CrPipeline_d3d12.h"

#include "CrD3D12.h"

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
	hResult = m_d3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_d3d12CommandQueue));
	CrAssertMsg(SUCCEEDED(hResult), "Error creating command queue");

	ID3D12InfoQueue* d3d12InfoQueue = NULL;
	if (SUCCEEDED(m_d3d12Device->QueryInterface(__uuidof(ID3D12InfoQueue), (void**)&d3d12InfoQueue)))
	{
		d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);

		//D3D12_MESSAGE_ID blockedIds[] = {};
		//D3D12_INFO_QUEUE_FILTER filter = {};
		//filter.DenyList.pIDList = nullptr;
		//filter.DenyList.NumIDs = 0;
		//d3d12InfoQueue->AddRetrievalFilterEntries(&filter);
		//d3d12InfoQueue->AddStorageFilterEntries(&filter);
		d3d12InfoQueue->Release();
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

ICrCommandQueue* CrRenderDeviceD3D12::CreateCommandQueuePS(CrCommandQueueType::T type)
{
	return new CrCommandQueueD3D12(this, type);
}

ICrGPUFence* CrRenderDeviceD3D12::CreateGPUFencePS()
{
	return new CrGPUFenceD3D12(this);
}

ICrGPUSemaphore* CrRenderDeviceD3D12::CreateGPUSemaphorePS()
{
	return nullptr;
}

ICrGraphicsShader* CrRenderDeviceD3D12::CreateGraphicsShaderPS(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor) const
{
	return new CrGraphicsShaderD3D12(this, graphicsShaderDescriptor);
}

ICrComputeShader* CrRenderDeviceD3D12::CreateComputeShaderPS(const CrComputeShaderDescriptor& computeShaderDescriptor) const
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

ICrGraphicsPipeline* CrRenderDeviceD3D12::CreateGraphicsPipelinePS(const CrGraphicsPipelineDescriptor& pipelineDescriptor, const ICrGraphicsShader* graphicsShader, const CrVertexDescriptor& vertexDescriptor)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3d12PipelineStateDescriptor;

	d3d12PipelineStateDescriptor.SampleMask = 0xffffffff;
	d3d12PipelineStateDescriptor.PrimitiveTopologyType = crd3d::GetD3D12PrimitiveTopologyType(pipelineDescriptor.primitiveTopology);
	d3d12PipelineStateDescriptor.NodeMask = 0;
	
	D3D12_RASTERIZER_DESC& rasterizerDesc = d3d12PipelineStateDescriptor.RasterizerState;
	rasterizerDesc.FillMode = crd3d::GetD3D12PolygonFillMode(pipelineDescriptor.rasterizerState.fillMode);
	rasterizerDesc.CullMode = crd3d::GetD3D12PolygonCullMode(pipelineDescriptor.rasterizerState.cullMode);
	rasterizerDesc.FrontCounterClockwise = pipelineDescriptor.rasterizerState.frontFace == cr3d::FrontFace::Clockwise ? false : true;
	rasterizerDesc.DepthBias = *reinterpret_cast<const INT*>(&pipelineDescriptor.rasterizerState.depthBias);
	rasterizerDesc.DepthBiasClamp = pipelineDescriptor.rasterizerState.depthBiasClamp;
	rasterizerDesc.SlopeScaledDepthBias = pipelineDescriptor.rasterizerState.slopeScaledDepthBias;
	rasterizerDesc.DepthClipEnable = pipelineDescriptor.rasterizerState.depthClipEnable;
	rasterizerDesc.MultisampleEnable = pipelineDescriptor.rasterizerState.multisampleEnable;
	rasterizerDesc.AntialiasedLineEnable = pipelineDescriptor.rasterizerState.antialiasedLineEnable;
	rasterizerDesc.ForcedSampleCount = 1;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	uint32_t numRenderTargets = (uint32_t)pipelineDescriptor.numRenderTargets;

	D3D12_BLEND_DESC& blendDesc = d3d12PipelineStateDescriptor.BlendState;
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;	
	d3d12PipelineStateDescriptor.NumRenderTargets = numRenderTargets;

	const CrRenderTargetBlendDescriptor& firstBlendState = pipelineDescriptor.blendState.renderTargetBlends[0];

	for (uint32_t i = 0; i < numRenderTargets; ++i)
	{
		const CrRenderTargetBlendDescriptor& renderTargetBlend = pipelineDescriptor.blendState.renderTargetBlends[i];
		D3D12_RENDER_TARGET_BLEND_DESC& renderTargetDesc = blendDesc.RenderTarget[i];

		renderTargetDesc.BlendEnable = renderTargetBlend.enable;
		renderTargetDesc.LogicOpEnable = false;
		renderTargetDesc.LogicOp = D3D12_LOGIC_OP_NOOP;

		renderTargetDesc.BlendOp = crd3d::GetD3DBlendOp(renderTargetBlend.colorBlendOp);
		renderTargetDesc.SrcBlend = crd3d::GetD3DBlendFactor(renderTargetBlend.srcColorBlendFactor);
		renderTargetDesc.DestBlend = crd3d::GetD3DBlendFactor(renderTargetBlend.dstColorBlendFactor);

		renderTargetDesc.BlendOpAlpha = crd3d::GetD3DBlendOp(renderTargetBlend.alphaBlendOp);
		renderTargetDesc.SrcBlendAlpha = crd3d::GetD3DBlendFactor(renderTargetBlend.srcAlphaBlendFactor);
		renderTargetDesc.DestBlendAlpha = crd3d::GetD3DBlendFactor(renderTargetBlend.dstAlphaBlendFactor);

		renderTargetDesc.RenderTargetWriteMask = renderTargetBlend.colorWriteMask;

		d3d12PipelineStateDescriptor.RTVFormats[i] = crd3d::GetDXGIFormat(pipelineDescriptor.renderTargets.colorFormats[i]);

		// If any blend state is different, turn on independent blend
		blendDesc.IndependentBlendEnable = blendDesc.IndependentBlendEnable || (firstBlendState != renderTargetBlend);
	}

	d3d12PipelineStateDescriptor.DSVFormat = crd3d::GetDXGIFormat(pipelineDescriptor.renderTargets.depthFormat);

	DXGI_SAMPLE_DESC& sampleDesc = d3d12PipelineStateDescriptor.SampleDesc;
	sampleDesc.Count = crd3d::GetD3D12SampleCount(pipelineDescriptor.sampleCount);
	sampleDesc.Quality = 1;

	D3D12_DEPTH_STENCIL_DESC& depthStencilDesc = d3d12PipelineStateDescriptor.DepthStencilState;
	depthStencilDesc.DepthEnable = pipelineDescriptor.depthStencilState.depthTestEnable;
	depthStencilDesc.DepthWriteMask = pipelineDescriptor.depthStencilState.depthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = crd3d::GetD3DCompareOp(pipelineDescriptor.depthStencilState.depthCompareOp);
	
	depthStencilDesc.StencilEnable = pipelineDescriptor.depthStencilState.stencilTestEnable;
	depthStencilDesc.StencilReadMask = pipelineDescriptor.depthStencilState.stencilReadMask;
	depthStencilDesc.StencilWriteMask = pipelineDescriptor.depthStencilState.stencilWriteMask;
	
	depthStencilDesc.FrontFace.StencilFailOp = crd3d::GetD3DStencilOp(pipelineDescriptor.depthStencilState.frontStencilFailOp);
	depthStencilDesc.FrontFace.StencilDepthFailOp = crd3d::GetD3DStencilOp(pipelineDescriptor.depthStencilState.frontDepthFailOp);
	depthStencilDesc.FrontFace.StencilPassOp = crd3d::GetD3DStencilOp(pipelineDescriptor.depthStencilState.frontStencilPassOp);
	depthStencilDesc.FrontFace.StencilFunc = crd3d::GetD3DCompareOp(pipelineDescriptor.depthStencilState.frontStencilCompareOp);

	depthStencilDesc.BackFace.StencilFailOp = crd3d::GetD3DStencilOp(pipelineDescriptor.depthStencilState.backStencilFailOp);
	depthStencilDesc.BackFace.StencilDepthFailOp = crd3d::GetD3DStencilOp(pipelineDescriptor.depthStencilState.backDepthFailOp);
	depthStencilDesc.BackFace.StencilPassOp = crd3d::GetD3DStencilOp(pipelineDescriptor.depthStencilState.backStencilPassOp);
	depthStencilDesc.BackFace.StencilFunc = crd3d::GetD3DCompareOp(pipelineDescriptor.depthStencilState.backStencilCompareOp);

	d3d12PipelineStateDescriptor.StreamOutput = {};

	// Only useful for triangle strips
	d3d12PipelineStateDescriptor.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

	d3d12PipelineStateDescriptor.CachedPSO.pCachedBlob = nullptr;
	d3d12PipelineStateDescriptor.CachedPSO.CachedBlobSizeInBytes = 0;

	d3d12PipelineStateDescriptor.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	const CrGraphicsShaderD3D12* d3d12GraphicsShader = static_cast<const CrGraphicsShaderD3D12*>(graphicsShader);

	const CrVector<CrShaderBytecodeSharedHandle>& bytecodes = d3d12GraphicsShader->GetBytecodes();

	d3d12PipelineStateDescriptor.VS = {};
	d3d12PipelineStateDescriptor.PS = {};
	d3d12PipelineStateDescriptor.HS = {};
	d3d12PipelineStateDescriptor.DS = {};
	d3d12PipelineStateDescriptor.GS = {};

	for (uint32_t i = 0; i < bytecodes.size(); ++i)
	{
		const CrShaderBytecodeSharedHandle& bytecode = bytecodes[i];

		switch (bytecode->GetShaderStage())
		{
			case cr3d::ShaderStage::Vertex: d3d12PipelineStateDescriptor.VS = { bytecode->GetBytecode().data(), bytecode->GetBytecode().size() }; break;
			case cr3d::ShaderStage::Pixel: d3d12PipelineStateDescriptor.PS = { bytecode->GetBytecode().data(), bytecode->GetBytecode().size() }; break;
			case cr3d::ShaderStage::Hull: d3d12PipelineStateDescriptor.HS = { bytecode->GetBytecode().data(), bytecode->GetBytecode().size() }; break;
			case cr3d::ShaderStage::Domain: d3d12PipelineStateDescriptor.DS = { bytecode->GetBytecode().data(), bytecode->GetBytecode().size() }; break;
			case cr3d::ShaderStage::Geometry: d3d12PipelineStateDescriptor.GS = { bytecode->GetBytecode().data(), bytecode->GetBytecode().size() }; break;
		}
	}

	D3D12_INPUT_LAYOUT_DESC& inputLayoutDescriptor = d3d12PipelineStateDescriptor.InputLayout;

	CrArray<D3D12_INPUT_ELEMENT_DESC, cr3d::MaxVertexAttributes> inputElementDescriptors;
	CrArray<CrFixedString16, cr3d::MaxVertexAttributes> renamedAttributes;

	for (uint32_t i = 0; i < vertexDescriptor.GetAttributeCount(); ++i)
	{
		D3D12_INPUT_ELEMENT_DESC& inputElementDescriptor = inputElementDescriptors[i];
		const CrVertexAttribute& vertexAttribute = vertexDescriptor.GetAttribute(i);
		const CrVertexSemantic::Data& semanticData = CrVertexSemantic::GetData((CrVertexSemantic::T)vertexAttribute.semantic);

		// D3D12 doesn't like semantics with names at the end, instead it
		// expects the index as an integer in SemanticIndex
		// Deal with this case by copying the semantic name and removing the index
		if (semanticData.indexOffset == 0xffffffff)
		{
			inputElementDescriptor.SemanticName = semanticData.semanticName.c_str();
			inputElementDescriptor.SemanticIndex = 0;
		}
		else
		{
			CrFixedString16& semanticNameCopy = renamedAttributes[i];
			semanticNameCopy = semanticData.semanticName.c_str();
			semanticNameCopy[semanticData.indexOffset] = 0;
			inputElementDescriptor.SemanticName = semanticNameCopy.c_str();
			inputElementDescriptor.SemanticIndex = semanticData.index;
		}

		inputElementDescriptor.Format = crd3d::GetDXGIFormat((cr3d::DataFormat::T)vertexAttribute.format);
		inputElementDescriptor.InputSlot = vertexAttribute.streamId;
		inputElementDescriptor.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElementDescriptor.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA; // Per-instance vertex data
		inputElementDescriptor.InstanceDataStepRate = 0;
	}

	inputLayoutDescriptor.pInputElementDescs = inputElementDescriptors.data();
	inputLayoutDescriptor.NumElements = vertexDescriptor.GetAttributeCount();

	// TODO Decide on an adequate strategy for the root signatures
	// ID3D12RootSignature* pRootSignature;
	d3d12PipelineStateDescriptor.pRootSignature = nullptr;

	ID3D12PipelineState* d3d12PipelineState;
	HRESULT hResult = m_d3d12Device->CreateGraphicsPipelineState(&d3d12PipelineStateDescriptor, IID_PPV_ARGS(&d3d12PipelineState));

	if (hResult == S_OK)
	{
		CrGraphicsPipelineD3D12* d3d12GraphicsPipeline = new CrGraphicsPipelineD3D12();
		d3d12GraphicsPipeline->m_d3d12PipelineState = d3d12PipelineState;
		return d3d12GraphicsPipeline;
	}
	else
	{
		return nullptr;
	}
}

ICrComputePipeline* CrRenderDeviceD3D12::CreateComputePipelinePS(const CrComputePipelineDescriptor& pipelineDescriptor, const ICrComputeShader* computeShader)
{
	unused_parameter(pipelineDescriptor);
	unused_parameter(computeShader);
	return nullptr;
}

ICrGPUQueryPool* CrRenderDeviceD3D12::CreateGPUQueryPoolPS(const CrGPUQueryPoolDescriptor& queryPoolDescriptor)
{
	unused_parameter(queryPoolDescriptor);
	return nullptr;
}

cr3d::GPUFenceResult CrRenderDeviceD3D12::WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds) const
{
	unused_parameter(fence);
	unused_parameter(timeoutNanoseconds);
	return cr3d::GPUFenceResult::Success;
}

cr3d::GPUFenceResult CrRenderDeviceD3D12::GetFenceStatusPS(const ICrGPUFence* fence) const
{
	unused_parameter(fence);
	return cr3d::GPUFenceResult::Success;
}

void CrRenderDeviceD3D12::ResetFencePS(const ICrGPUFence* fence)
{
	unused_parameter(fence);
}

void CrRenderDeviceD3D12::WaitIdlePS()
{

}

bool CrRenderDeviceD3D12::GetIsFeatureSupported(CrRenderingFeature::T feature) const
{
	unused_parameter(feature);
	return false;
}