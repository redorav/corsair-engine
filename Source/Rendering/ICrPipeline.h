#pragma once

#include "CrRenderingForwardDeclarations.h"

#include "Core/SmartPointers/CrSharedPtr.h"

#include "Core/CrHash.h"

struct CrRasterizerStateDescriptor
{
	cr3d::PolygonFillMode fillMode : 2;
	cr3d::PolygonCullMode cullMode : 2;
	cr3d::FrontFace frontFace : 1; // Default is clockwise
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
	CrRenderTargetBlendDescriptor renderTargetBlends[cr3d::MaxRenderTargets];
	uint8_t numRenderTargets : 8;

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

struct CrMultisampleStateDescriptor
{
	cr3d::SampleCount sampleCount : 4;
};

struct CrGraphicsPipelineDescriptor : public CrAutoHashable<CrGraphicsPipelineDescriptor>
{
	CrGraphicsPipelineDescriptor()
	{
		primitiveTopology = cr3d::PrimitiveTopology::TriangleList;

		rasterizerState.fillMode = cr3d::PolygonFillMode::Fill;
		rasterizerState.frontFace = cr3d::FrontFace::Clockwise;
		rasterizerState.cullMode = cr3d::PolygonCullMode::None;

		multisampleState.sampleCount = cr3d::SampleCount::S1;

		blendState.numRenderTargets = 1;
		blendState.renderTargetBlends[0].colorWriteMask = cr3d::ColorWriteComponent::All;

		depthStencilState.depthTestEnable = true;
		depthStencilState.depthWriteEnable = true;
		depthStencilState.depthCompareOp = cr3d::CompareOp::Greater;
	}

	cr3d::PrimitiveTopology primitiveTopology = {};
	CrRasterizerStateDescriptor rasterizerState = {};
	CrBlendStateDescriptor blendState = {};
	CrDepthStencilStateDescriptor depthStencilState = {};
	CrMultisampleStateDescriptor multisampleState = {};
};

class ICrGraphicsPipeline
{
public:

	virtual ~ICrGraphicsPipeline() {}

	// TODO PRIVATE
	
	CrGraphicsShaderHandle m_shader;
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