#include "Rendering/CrRendering_pch.h"

#include "CrPipeline_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrShader_d3d12.h"
#include "CrD3D12.h"

CrGraphicsPipelineD3D12::CrGraphicsPipelineD3D12
(
	CrRenderDeviceD3D12* d3d12RenderDevice, const CrGraphicsPipelineDescriptor& pipelineDescriptor,
	const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor
)
	: ICrGraphicsPipeline(d3d12RenderDevice, graphicsShader, vertexDescriptor)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3d12PipelineStateDescriptor;

	d3d12PipelineStateDescriptor.SampleMask = 0xffffffff;
	d3d12PipelineStateDescriptor.PrimitiveTopologyType = crd3d::GetD3D12PrimitiveTopologyType(pipelineDescriptor.primitiveTopology);
	d3d12PipelineStateDescriptor.NodeMask = 0;

	m_d3d12PrimitiveTopology = crd3d::GetD3D12PrimitiveTopology(pipelineDescriptor.primitiveTopology);

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
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	uint32_t numRenderTargets = 0;

	D3D12_BLEND_DESC& blendDesc = d3d12PipelineStateDescriptor.BlendState;
	blendDesc = {};
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;

	const CrRenderTargetBlendDescriptor& firstBlendState = pipelineDescriptor.blendState.renderTargetBlends[0];

	static_assert(cr3d::MaxRenderTargets == sizeof_array(d3d12PipelineStateDescriptor.RTVFormats), "Array not large enough ");

	for (uint32_t i = 0, end = cr3d::MaxRenderTargets; i < end; ++i)
	{
		const CrRenderTargetFormatDescriptor& renderTargets = pipelineDescriptor.renderTargets;
		D3D12_RENDER_TARGET_BLEND_DESC& renderTargetDesc = blendDesc.RenderTarget[i];

		if (renderTargets.colorFormats[i] != cr3d::DataFormat::Invalid)
		{
			const CrRenderTargetBlendDescriptor& renderTargetBlend = pipelineDescriptor.blendState.renderTargetBlends[i];

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
			numRenderTargets++;
		}
		else
		{
			d3d12PipelineStateDescriptor.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
		}
	}

	d3d12PipelineStateDescriptor.NumRenderTargets = numRenderTargets;

	if (pipelineDescriptor.renderTargets.depthFormat != cr3d::DataFormat::Invalid)
	{
		d3d12PipelineStateDescriptor.DSVFormat = crd3d::GetDXGIFormat(pipelineDescriptor.renderTargets.depthFormat);
	}
	else
	{
		d3d12PipelineStateDescriptor.DSVFormat = DXGI_FORMAT_UNKNOWN;
	}

	DXGI_SAMPLE_DESC& sampleDesc = d3d12PipelineStateDescriptor.SampleDesc;
	sampleDesc.Count = crd3d::GetD3D12SampleCount(pipelineDescriptor.sampleCount);
	sampleDesc.Quality = 0;

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

	d3d12PipelineStateDescriptor.CachedPSO = {};
	d3d12PipelineStateDescriptor.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	const CrVector<CrShaderBytecodeHandle>& bytecodes = graphicsShader->GetBytecodes();

	d3d12PipelineStateDescriptor.VS = {};
	d3d12PipelineStateDescriptor.PS = {};
	d3d12PipelineStateDescriptor.HS = {};
	d3d12PipelineStateDescriptor.DS = {};
	d3d12PipelineStateDescriptor.GS = {};

	for (uint32_t i = 0; i < bytecodes.size(); ++i)
	{
		const CrShaderBytecodeHandle& bytecode = bytecodes[i];

		switch (bytecode->GetShaderStage())
		{
			case cr3d::ShaderStage::Vertex: d3d12PipelineStateDescriptor.VS = { bytecode->GetBytecode().data(), bytecode->GetBytecode().size() }; break;
			case cr3d::ShaderStage::Pixel: d3d12PipelineStateDescriptor.PS = { bytecode->GetBytecode().data(), bytecode->GetBytecode().size() }; break;
			case cr3d::ShaderStage::Hull: d3d12PipelineStateDescriptor.HS = { bytecode->GetBytecode().data(), bytecode->GetBytecode().size() }; break;
			case cr3d::ShaderStage::Domain: d3d12PipelineStateDescriptor.DS = { bytecode->GetBytecode().data(), bytecode->GetBytecode().size() }; break;
			case cr3d::ShaderStage::Geometry: d3d12PipelineStateDescriptor.GS = { bytecode->GetBytecode().data(), bytecode->GetBytecode().size() }; break;
			default: break;
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

		cr3d::DataFormat::T semanticFormat = (cr3d::DataFormat::T)vertexAttribute.format;
		inputElementDescriptor.Format = crd3d::GetDXGIFormat(semanticFormat);
		inputElementDescriptor.InputSlot = vertexAttribute.streamId;
		inputElementDescriptor.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElementDescriptor.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA; // Per-instance vertex data
		inputElementDescriptor.InstanceDataStepRate = 0;
	}

	inputLayoutDescriptor.pInputElementDescs = inputElementDescriptors.data();
	inputLayoutDescriptor.NumElements = vertexDescriptor.GetAttributeCount();

	// Get root signature from the global root signature repository
	m_d3d12RootSignature = d3d12PipelineStateDescriptor.pRootSignature = d3d12RenderDevice->GetD3D12GraphicsRootSignature();

	HRESULT hResult = d3d12RenderDevice->GetD3D12Device()->CreateGraphicsPipelineState(&d3d12PipelineStateDescriptor, IID_PPV_ARGS(&m_d3d12PipelineState));
	CrAssertMsg(hResult == S_OK, "Failed to create graphics pipeline");
}

CrGraphicsPipelineD3D12::~CrGraphicsPipelineD3D12()
{
	m_d3d12PipelineState->Release();
}

CrComputePipelineD3D12::CrComputePipelineD3D12(CrRenderDeviceD3D12* d3d12RenderDevice, const CrComputeShaderHandle& computeShader)
	: ICrComputePipeline(d3d12RenderDevice, computeShader)
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC d3d12PipelineStateDescriptor;
	d3d12PipelineStateDescriptor.NodeMask = 0;
	d3d12PipelineStateDescriptor.CachedPSO = {};
	d3d12PipelineStateDescriptor.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	const CrShaderBytecodeHandle& bytecode = computeShader->GetBytecode();
	d3d12PipelineStateDescriptor.CS = { bytecode->GetBytecode().data(), bytecode->GetBytecode().size()};

	m_d3d12RootSignature = d3d12PipelineStateDescriptor.pRootSignature = d3d12RenderDevice->GetD3D12ComputeRootSignature();

	HRESULT hResult = d3d12RenderDevice->GetD3D12Device()->CreateComputePipelineState(&d3d12PipelineStateDescriptor, IID_PPV_ARGS(&m_d3d12PipelineState));
	CrAssertMsg(hResult == S_OK, "Failed to create compute pipeline");
}

CrComputePipelineD3D12::~CrComputePipelineD3D12()
{
	m_d3d12PipelineState->Release();
}
