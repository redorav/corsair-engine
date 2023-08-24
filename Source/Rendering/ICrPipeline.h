#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"
#include "Rendering/CrRendering.h"
#include "Rendering/CrDataFormats.h"
#include "Rendering/CrGPUDeletable.h"
#include "Rendering/CrVertexDescriptor.h"

#include "Core/SmartPointers/CrIntrusivePtr.h"
#include "Core/CrHash.h"
#include "Core/Containers/CrArray.h"

struct CrRasterizerStateDescriptor
{
	cr3d::PolygonFillMode fillMode : 2;
	cr3d::PolygonCullMode cullMode : 2;
	cr3d::FrontFace frontFace : 1;
	uint32_t depthClipEnable : 1;
	uint32_t multisampleEnable : 1;
	uint32_t antialiasedLineEnable : 1;

	uint32_t padding : 24;
	
	float depthBias;
	float depthBiasClamp;
	float slopeScaledDepthBias;
};

static_assert(sizeof(CrRasterizerStateDescriptor) == 16, "CrRasterizerStateDescriptor size mismatch");

struct CrRenderTargetBlendDescriptor
{
	CrRenderTargetBlendDescriptor()
	{
		// We default to standard alpha blending, where colors are blended and alpha is replaced
		bits = 0;
		srcColorBlendFactor = cr3d::BlendFactor::SrcAlpha;
		dstColorBlendFactor = cr3d::BlendFactor::OneMinusSrcAlpha;
		srcAlphaBlendFactor = cr3d::BlendFactor::One;
		dstAlphaBlendFactor = cr3d::BlendFactor::Zero;
		colorWriteMask      = cr3d::ColorWriteComponent::All;
		colorBlendOp        = cr3d::BlendOp::Add;
		alphaBlendOp        = cr3d::BlendOp::Add;
	}

	union
	{
		struct
		{
			cr3d::BlendFactor srcColorBlendFactor : 5;
			cr3d::BlendFactor dstColorBlendFactor : 5;

			cr3d::BlendFactor srcAlphaBlendFactor : 5;
			cr3d::BlendFactor dstAlphaBlendFactor : 5;
			cr3d::ColorWriteMask colorWriteMask : 4;
			cr3d::BlendOp colorBlendOp : 3;
			cr3d::BlendOp alphaBlendOp : 3;

			uint32_t enable : 1;
			uint32_t padding : 1;
		};

		uint32_t bits;
	};

	bool operator == (const CrRenderTargetBlendDescriptor& other) const { return bits == other.bits; }

	bool operator != (const CrRenderTargetBlendDescriptor& other) const { return bits != other.bits; }
};

static_assert(sizeof(CrRenderTargetBlendDescriptor) == 4, "CrRenderTargetBlendDescriptor size mismatch");

struct CrBlendStateDescriptor
{
	CrArray<CrRenderTargetBlendDescriptor, cr3d::MaxRenderTargets> renderTargetBlends;

	// See https://msdn.microsoft.com/en-us/library/windows/desktop/dn770339(v=vs.85).aspx for why logicOps is 
	// in the blend state and not a per render target field.
	uint32_t logicOpEnable : 1;
	cr3d::LogicOp logicOp : 4;
	uint32_t padding : 27;
	float blendConstants[4];
};

static_assert(sizeof(CrBlendStateDescriptor) == 52, "CrBlendStateDescriptor size mismatch");

struct CrDepthStencilStateDescriptor
{
	cr3d::CompareOp       depthCompareOp : 3;
	uint32_t              depthTestEnable : 1;
	uint32_t              depthWriteEnable : 1;
	uint32_t              depthBoundsTestEnable : 1;
	uint32_t              stencilTestEnable : 1;

	uint32_t              stencilReadMask : 8;
	uint32_t              stencilWriteMask : 8;
	uint32_t              reference : 8;

	uint32_t              padding : 1;

	cr3d::StencilOp       frontStencilFailOp : 3;
	cr3d::StencilOp       frontDepthFailOp : 3;
	cr3d::StencilOp       frontStencilPassOp : 3;
	cr3d::CompareOp       frontStencilCompareOp : 3;

	cr3d::StencilOp       backStencilFailOp : 3;
	cr3d::StencilOp       backDepthFailOp : 3;
	cr3d::StencilOp       backStencilPassOp : 3;
	cr3d::CompareOp       backStencilCompareOp : 3;

	uint32_t              padding2 : 8;

	float                 minDepthBounds;
	float                 maxDepthBounds;
};

static_assert(sizeof(CrDepthStencilStateDescriptor) == 16, "CrDepthStencilStateDescriptor size mismatch");

struct CrRenderTargetFormatDescriptor
{
	CrArray<cr3d::DataFormat::T, cr3d::MaxRenderTargets> colorFormats =
	{
		cr3d::DataFormat::Invalid, cr3d::DataFormat::Invalid, cr3d::DataFormat::Invalid, cr3d::DataFormat::Invalid,
		cr3d::DataFormat::Invalid, cr3d::DataFormat::Invalid, cr3d::DataFormat::Invalid, cr3d::DataFormat::Invalid
	};

	cr3d::DataFormat::T depthFormat = cr3d::DataFormat::Invalid;
	cr3d::SampleCount sampleCount = cr3d::SampleCount::S1;
};

static_assert(sizeof(CrRenderTargetFormatDescriptor) == 40, "CrRenderTargetFormatDescriptor size mismatch");

// TODO Optimize size of pipeline descriptor
struct CrGraphicsPipelineDescriptor
{
	CrGraphicsPipelineDescriptor()
	{
		primitiveTopology         = cr3d::PrimitiveTopology::TriangleList;
		sampleCount               = cr3d::SampleCount::S1;
		
		rasterizerState.fillMode  = cr3d::PolygonFillMode::Fill;
		rasterizerState.frontFace = cr3d::FrontFace::Clockwise;
		rasterizerState.cullMode  = cr3d::PolygonCullMode::Back;
		rasterizerState.depthClipEnable = true;
		
		padding                   = 0;
		
		// Don't put a loop here to initialize the color write masks
		blendState.renderTargetBlends[0].colorWriteMask = cr3d::ColorWriteComponent::All;
		blendState.renderTargetBlends[1].colorWriteMask = cr3d::ColorWriteComponent::All;
		blendState.renderTargetBlends[2].colorWriteMask = cr3d::ColorWriteComponent::All;
		blendState.renderTargetBlends[3].colorWriteMask = cr3d::ColorWriteComponent::All;
		blendState.renderTargetBlends[4].colorWriteMask = cr3d::ColorWriteComponent::All;
		blendState.renderTargetBlends[5].colorWriteMask = cr3d::ColorWriteComponent::All;
		blendState.renderTargetBlends[6].colorWriteMask = cr3d::ColorWriteComponent::All;
		blendState.renderTargetBlends[7].colorWriteMask = cr3d::ColorWriteComponent::All;
		
		depthStencilState.depthTestEnable  = true;
		depthStencilState.depthWriteEnable = true;
		depthStencilState.depthCompareOp   = cr3d::CompareOp::Greater; // Reverse depth by default
	}

	CrHash ComputeHash() const
	{
		return CrHash(this, sizeof(*this));
	}

	cr3d::PrimitiveTopology        primitiveTopology : 4;
	cr3d::SampleCount              sampleCount       : 4;
	uint32_t                       padding           : 24;

	CrRasterizerStateDescriptor    rasterizerState = {};
	CrBlendStateDescriptor         blendState = {};
	CrDepthStencilStateDescriptor  depthStencilState = {};
	CrRenderTargetFormatDescriptor renderTargets = {};
};

static_assert(sizeof(CrGraphicsPipelineDescriptor) == 128, "CrGraphicsPipelineDescriptor size mismatch");

namespace CrBuiltinShaders
{
	enum T : uint32_t;
}

class ICrGraphicsPipeline : public CrGPUAutoDeletable
{
public:

	ICrGraphicsPipeline(ICrRenderDevice* renderDevice, const CrGraphicsPipelineDescriptor& pipelineDescriptor, const CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor);

	virtual ~ICrGraphicsPipeline();

	const CrGraphicsShaderHandle& GetShader() const { return m_shader; }

	uint32_t GetVertexStreamCount() const { return m_usedVertexStreamCount; }

private:

	CrGraphicsShaderHandle m_shader;
	
	uint32_t m_usedVertexStreamCount = 0;

#if !defined(CR_CONFIG_FINAL)

public:

	virtual void Recompile(ICrRenderDevice* renderDevice, const CrGraphicsShaderHandle& graphicsShader) = 0;

	CrBuiltinShaders::T GetVertexShaderIndex() const { return m_vertexShaderIndex; }

	CrBuiltinShaders::T GetPixelShaderIndex() const { return m_pixelShaderIndex; }

	void SetShaderIndices(CrBuiltinShaders::T vertexShaderIndex, CrBuiltinShaders::T pixelShaderIndex)
	{
		m_vertexShaderIndex = vertexShaderIndex;
		m_pixelShaderIndex = pixelShaderIndex;
	}

protected:

	CrGraphicsPipelineDescriptor m_pipelineDescriptor;

	CrVertexDescriptor m_vertexDescriptor;

	CrBuiltinShaders::T m_vertexShaderIndex = (CrBuiltinShaders::T)-1;

	CrBuiltinShaders::T m_pixelShaderIndex = (CrBuiltinShaders::T)-1;

#endif
};

class ICrComputePipeline : public CrGPUAutoDeletable
{
public:

	ICrComputePipeline(ICrRenderDevice* renderDevice, const CrComputeShaderHandle& computeShader);

	virtual ~ICrComputePipeline();

	const CrComputeShaderHandle& GetShader() const { return m_shader; }

private:

	CrComputeShaderHandle m_shader;

#if !defined(CR_CONFIG_FINAL)

public:

	virtual void Recompile(ICrRenderDevice* renderDevice, const CrComputeShaderHandle& computeShader) = 0;

	CrBuiltinShaders::T GetComputeShaderIndex() const { return m_computeShaderIndex; }
	
	void SetComputeShaderIndex(CrBuiltinShaders::T computeShaderIndex) { m_computeShaderIndex = computeShaderIndex; }

private:

	CrBuiltinShaders::T m_computeShaderIndex = (CrBuiltinShaders::T)-1;

#endif
};