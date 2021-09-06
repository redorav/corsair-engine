#pragma once

#include "CrRenderingForwardDeclarations.h"

#include "Core/SmartPointers/CrSharedPtr.h"

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
	float depthBias;
	float depthBiasClamp;
	float slopeScaledDepthBias;
};

struct CrRenderTargetBlendDescriptor
{
	cr3d::BlendFactor srcColorBlendFactor : 5;
	cr3d::BlendFactor dstColorBlendFactor : 5;

	cr3d::BlendFactor srcAlphaBlendFactor : 5;
	cr3d::BlendFactor dstAlphaBlendFactor : 5;
	cr3d::ColorWriteMask colorWriteMask : 4;
	cr3d::BlendOp colorBlendOp : 3;
	cr3d::BlendOp alphaBlendOp : 3;

	uint8_t enable : 1;
};

struct CrBlendStateDescriptor
{
	CrArray<CrRenderTargetBlendDescriptor, cr3d::MaxRenderTargets> renderTargetBlends;

	// See https://msdn.microsoft.com/en-us/library/windows/desktop/dn770339(v=vs.85).aspx for why logicOps is 
	// in the blend state and not a per render target field.
	uint8_t logicOpEnable : 1;
	cr3d::LogicOp logicOp : 4;
	float blendConstants[4];
};

struct CrStencilOpDescriptor
{
	cr3d::StencilOp stencilFailOp : 3;
	cr3d::StencilOp depthFailOp : 3;
	cr3d::StencilOp stencilPassOp : 3;
	cr3d::CompareOp stencilCompareOp : 3;
	uint8_t stencilReadMask : 8;
	uint8_t stencilWriteMask : 8;
	uint8_t reference : 8;
};

struct CrDepthStencilStateDescriptor
{
	uint8_t               depthTestEnable : 1;
	uint8_t               depthWriteEnable : 1;
	cr3d::CompareOp       depthCompareOp : 3;
	uint8_t               depthBoundsTestEnable : 1;
	uint8_t               stencilTestEnable : 1;
	CrStencilOpDescriptor front;
	CrStencilOpDescriptor back;
	float                 minDepthBounds;
	float                 maxDepthBounds;
};

struct CrRenderTargetFormatDescriptor
{
	CrArray<cr3d::DataFormat::T, cr3d::MaxRenderTargets> colorFormats;
	cr3d::DataFormat::T depthFormat = cr3d::DataFormat::Invalid;
	cr3d::SampleCount sampleCount = cr3d::SampleCount::S1;
};

// TODO Optimize size of pipeline descriptor
struct CrGraphicsPipelineDescriptor : public CrAutoHashable<CrGraphicsPipelineDescriptor>
{
	CrGraphicsPipelineDescriptor()
	{
		primitiveTopology         = cr3d::PrimitiveTopology::TriangleList;
		sampleCount               = cr3d::SampleCount::S1;

		rasterizerState.fillMode  = cr3d::PolygonFillMode::Fill;
		rasterizerState.frontFace = cr3d::FrontFace::Clockwise;
		rasterizerState.cullMode  = cr3d::PolygonCullMode::Back;

		numRenderTargets          = 1;

		// Don't put a loop here to initialize the color write masks. It helps the compiler
		// hoist the code outside of loops
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

	cr3d::PrimitiveTopology        primitiveTopology : 4;

	// Multisample state
	cr3d::SampleCount              sampleCount       : 4;
	uint32_t                       numRenderTargets  : 4;
	CrRasterizerStateDescriptor    rasterizerState = {};
	CrBlendStateDescriptor         blendState = {};
	CrDepthStencilStateDescriptor  depthStencilState = {};
	CrRenderTargetFormatDescriptor renderTargets = {};
};

class ICrGraphicsPipeline
{
public:

	virtual ~ICrGraphicsPipeline() {}

	const CrGraphicsShaderHandle& GetShader() const { return m_shader; }

	// TODO PRIVATE

	CrGraphicsShaderHandle m_shader;
	
	uint32_t m_usedVertexStreamCount = 0;
};

struct CrComputePipelineDescriptor : public CrAutoHashable<CrComputePipelineDescriptor>
{

};

class ICrComputePipeline
{
public:

	virtual ~ICrComputePipeline() {}

	// TODO PRIVATE

	CrComputeShaderHandle m_shader;
};