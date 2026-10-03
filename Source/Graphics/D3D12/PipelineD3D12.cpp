#include "Graphics/CrRendering_pch.h"

#include "Graphics/IGraphicsSystem.h"

#include "PipelineD3D12.h"
#include "DeviceD3D12.h"
#include "CrD3D12.h"

#include "Graphics/IShader.inl"

#include "Graphics/Extensions/CrNVAPIHeader.h"

namespace crgfx
{
	GraphicsPipelineD3D12::GraphicsPipelineD3D12
	(
		crgfx::DeviceD3D12* d3d12RenderDevice, const GraphicsPipelineDescriptor& pipelineDescriptor, const crgfx::GraphicsShaderBytecode& graphicsShaderBytecode, const VertexDescriptor& vertexDescriptor
	)
		: IGraphicsPipeline(d3d12RenderDevice, pipelineDescriptor, vertexDescriptor)
	{
		Initialize(d3d12RenderDevice, pipelineDescriptor, graphicsShaderBytecode, vertexDescriptor);
	}

	void GraphicsPipelineD3D12::Initialize(crgfx::DeviceD3D12* d3d12RenderDevice, const GraphicsPipelineDescriptor& pipelineDescriptor, const crgfx::GraphicsShaderBytecode& graphicsShaderBytecode, const VertexDescriptor& vertexDescriptor)
	{
		ShaderBindingLayoutResources resources;

		// Create the shader modules and parse reflection information
		for (const ShaderBytecodeHandle& shaderBytecode : graphicsShaderBytecode.GetBytecodes())
		{
			const CrShaderReflectionHeader& reflectionHeader = shaderBytecode->GetReflection();
			ShaderBindingLayout::AddResources(reflectionHeader, resources, [](crgfx::ShaderStage::T, const CrShaderReflectionResource&) {});
		}

		m_bindingLayout = ShaderBindingLayout(resources);

		D3D12_GRAPHICS_PIPELINE_STATE_DESC d3d12PipelineStateDescriptor;

		d3d12PipelineStateDescriptor.SampleMask = 0xffffffff;
		d3d12PipelineStateDescriptor.PrimitiveTopologyType = crd3d::GetD3D12PrimitiveTopologyType(pipelineDescriptor.primitiveTopology);
		d3d12PipelineStateDescriptor.NodeMask = 0;

		m_d3d12PrimitiveTopology = crd3d::GetD3D12PrimitiveTopology(pipelineDescriptor.primitiveTopology);

		D3D12_RASTERIZER_DESC& rasterizerDesc = d3d12PipelineStateDescriptor.RasterizerState;
		rasterizerDesc.FillMode = crd3d::GetD3D12PolygonFillMode(pipelineDescriptor.rasterizerState.fillMode);
		rasterizerDesc.CullMode = crd3d::GetD3D12PolygonCullMode(pipelineDescriptor.rasterizerState.cullMode);
		rasterizerDesc.FrontCounterClockwise = pipelineDescriptor.rasterizerState.frontFace == crgfx::FrontFace::Clockwise ? false : true;
		rasterizerDesc.DepthBias = *reinterpret_cast<const INT*>(&pipelineDescriptor.rasterizerState.depthBias);
		rasterizerDesc.DepthBiasClamp = pipelineDescriptor.rasterizerState.depthBiasClamp;
		rasterizerDesc.SlopeScaledDepthBias = pipelineDescriptor.rasterizerState.slopeScaledDepthBias;
		rasterizerDesc.DepthClipEnable = pipelineDescriptor.rasterizerState.depthClipEnable;
		rasterizerDesc.MultisampleEnable = pipelineDescriptor.rasterizerState.multisampleEnable;
		rasterizerDesc.AntialiasedLineEnable = pipelineDescriptor.rasterizerState.antialiasedLineEnable;
		rasterizerDesc.ForcedSampleCount = 0;
		rasterizerDesc.ConservativeRaster = pipelineDescriptor.rasterizerState.conservativeRasterization ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		uint32_t numRenderTargets = 0;

		D3D12_BLEND_DESC& blendDesc = d3d12PipelineStateDescriptor.BlendState;
		blendDesc = {};
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;

		const crgfx::RenderTargetBlendDescriptor& firstBlendState = pipelineDescriptor.blendState.renderTargetBlends[0];

		static_assert(crgfx::MaxRenderTargets == sizeof_array(d3d12PipelineStateDescriptor.RTVFormats), "Array not large enough ");

		for (uint32_t i = 0, end = crgfx::MaxRenderTargets; i < end; ++i)
		{
			const crgfx::RenderTargetFormatDescriptor& renderTargets = pipelineDescriptor.renderTargets;
			D3D12_RENDER_TARGET_BLEND_DESC& renderTargetDesc = blendDesc.RenderTarget[i];

			if (renderTargets.colorFormats[i] != crgfx::DataFormat::Invalid)
			{
				const crgfx::RenderTargetBlendDescriptor& renderTargetBlend = pipelineDescriptor.blendState.renderTargetBlends[i];

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

		if (pipelineDescriptor.renderTargets.depthFormat != crgfx::DataFormat::Invalid)
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

		const crstl::span<const ShaderBytecodeHandle>& bytecodes = graphicsShaderBytecode.GetBytecodes();

		d3d12PipelineStateDescriptor.VS = {};
		d3d12PipelineStateDescriptor.PS = {};
		d3d12PipelineStateDescriptor.HS = {};
		d3d12PipelineStateDescriptor.DS = {};
		d3d12PipelineStateDescriptor.GS = {};

		for (uint32_t i = 0; i < bytecodes.size(); ++i)
		{
			const crgfx::ShaderBytecodeHandle& bytecode = bytecodes[i];

			switch (bytecode->GetShaderStage())
			{
				case crgfx::ShaderStage::Vertex: d3d12PipelineStateDescriptor.VS = { bytecode->GetBytecode().data(), bytecode->GetBytecode().size() }; break;
				case crgfx::ShaderStage::Pixel: d3d12PipelineStateDescriptor.PS = { bytecode->GetBytecode().data(), bytecode->GetBytecode().size() }; break;
				case crgfx::ShaderStage::Hull: d3d12PipelineStateDescriptor.HS = { bytecode->GetBytecode().data(), bytecode->GetBytecode().size() }; break;
				case crgfx::ShaderStage::Domain: d3d12PipelineStateDescriptor.DS = { bytecode->GetBytecode().data(), bytecode->GetBytecode().size() }; break;
				case crgfx::ShaderStage::Geometry: d3d12PipelineStateDescriptor.GS = { bytecode->GetBytecode().data(), bytecode->GetBytecode().size() }; break;
				default: break;
			}
		}

		D3D12_INPUT_LAYOUT_DESC& inputLayoutDescriptor = d3d12PipelineStateDescriptor.InputLayout;

		crstl::array<D3D12_INPUT_ELEMENT_DESC, crgfx::MaxVertexAttributes> inputElementDescriptors;
		crstl::array<crstl::fixed_string16, crgfx::MaxVertexAttributes> renamedAttributes;

		for (uint32_t i = 0; i < vertexDescriptor.GetAttributeCount(); ++i)
		{
			D3D12_INPUT_ELEMENT_DESC& inputElementDescriptor = inputElementDescriptors[i];
			const VertexAttribute& vertexAttribute = vertexDescriptor.GetAttribute(i);
			const VertexSemantic::Data& semanticData = VertexSemantic::GetData((VertexSemantic::T)vertexAttribute.semantic);

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
				crstl::fixed_string16& semanticNameCopy = renamedAttributes[i];
				semanticNameCopy = semanticData.semanticName.c_str();
				semanticNameCopy[semanticData.indexOffset] = 0;
				inputElementDescriptor.SemanticName = semanticNameCopy.c_str();
				inputElementDescriptor.SemanticIndex = semanticData.index;
			}

			crgfx::DataFormat::T semanticFormat = (crgfx::DataFormat::T)vertexAttribute.format;
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

		d3d12RenderDevice->SetD3D12ObjectName(m_d3d12PipelineState, graphicsShaderBytecode.GetDebugName());
	}

	GraphicsPipelineD3D12::~GraphicsPipelineD3D12()
	{
		Deinitialize();
	}

	void GraphicsPipelineD3D12::Deinitialize()
	{
		m_d3d12PipelineState->Release();
	}

	ComputePipelineD3D12::ComputePipelineD3D12(crgfx::DeviceD3D12* d3d12RenderDevice, const crgfx::ComputeShaderBytecode& computeShaderBytecode)
		: IComputePipeline(d3d12RenderDevice, computeShaderBytecode)
	{
		Initialize(d3d12RenderDevice, computeShaderBytecode);
	}

	void ComputePipelineD3D12::Initialize(crgfx::DeviceD3D12* d3d12RenderDevice, const crgfx::ComputeShaderBytecode& computeShaderBytecode)
	{
		ShaderBindingLayoutResources resources;

		// Create the shader modules and parse reflection information
		const ShaderBytecodeHandle& shaderBytecode = computeShaderBytecode.GetBytecode();
		const CrShaderReflectionHeader& reflectionHeader = shaderBytecode->GetReflection();
		ShaderBindingLayout::AddResources(reflectionHeader, resources, [](crgfx::ShaderStage::T, const CrShaderReflectionResource&) {});

		m_bindingLayout = ShaderBindingLayout(resources);

		D3D12_COMPUTE_PIPELINE_STATE_DESC d3d12PipelineStateDescriptor;
		d3d12PipelineStateDescriptor.NodeMask = 0;
		d3d12PipelineStateDescriptor.CachedPSO = {};
		d3d12PipelineStateDescriptor.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		d3d12PipelineStateDescriptor.CS = { shaderBytecode->GetBytecode().data(), shaderBytecode->GetBytecode().size() };

		m_d3d12RootSignature = d3d12PipelineStateDescriptor.pRootSignature = d3d12RenderDevice->GetD3D12ComputeRootSignature();

		HRESULT hResult = d3d12RenderDevice->GetD3D12Device()->CreateComputePipelineState(&d3d12PipelineStateDescriptor, IID_PPV_ARGS(&m_d3d12PipelineState));
		CrAssertMsg(hResult == S_OK, "Failed to create compute pipeline");

		d3d12RenderDevice->SetD3D12ObjectName(m_d3d12PipelineState, computeShaderBytecode.GetDebugName());
	}

	ComputePipelineD3D12::~ComputePipelineD3D12()
	{
		Deinitialize();
	}

	void ComputePipelineD3D12::Deinitialize()
	{
		m_d3d12PipelineState->Release();
	}

	#if !defined(CR_CONFIG_FINAL)

	void GraphicsPipelineD3D12::RecompilePS(IDevice* renderDevice, const GraphicsShaderBytecode& graphicsShaderBytecode)
	{
		Deinitialize();
		Initialize(static_cast<DeviceD3D12*>(renderDevice), m_pipelineDescriptor, graphicsShaderBytecode, m_vertexDescriptor);
	}

	void ComputePipelineD3D12::RecompilePS(IDevice* renderDevice, const ComputeShaderBytecode& computeShaderBytecode)
	{
		Deinitialize();
		Initialize(static_cast<DeviceD3D12*>(renderDevice), computeShaderBytecode);
	}

	#endif
};