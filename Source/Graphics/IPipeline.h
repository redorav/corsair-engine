#pragma once

#include "Graphics/CrGraphicsForwardDeclarations.h"
#include "Graphics/CrRendering.h"
#include "Graphics/DataFormats.h"
#include "Graphics/CrGPUDeletable.h"
#include "Graphics/CrVertexDescriptor.h"

#include "Core/CrHash.h"

#include "crstl/array.h"
#include "crstl/intrusive_ptr.h"

namespace CrBuiltinShaders { enum T : uint32_t; }

namespace CrBuiltinCompute { enum T : uint32_t; }

namespace crgfx
{
	struct CrRasterizerStateDescriptor
	{
		crgfx::PolygonFillMode fillMode : 2;
		crgfx::PolygonCullMode cullMode : 2;
		crgfx::FrontFace frontFace : 1;
		uint32_t depthClipEnable : 1;
		uint32_t multisampleEnable : 1;
		uint32_t antialiasedLineEnable : 1;
		uint32_t conservativeRasterization : 1;

		uint32_t padding : 23;

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
			srcColorBlendFactor = crgfx::BlendFactor::SrcAlpha;
			dstColorBlendFactor = crgfx::BlendFactor::OneMinusSrcAlpha;
			srcAlphaBlendFactor = crgfx::BlendFactor::One;
			dstAlphaBlendFactor = crgfx::BlendFactor::Zero;
			colorWriteMask = crgfx::ColorWriteComponent::All;
			colorBlendOp = crgfx::BlendOp::Add;
			alphaBlendOp = crgfx::BlendOp::Add;
		}

		CrRenderTargetBlendDescriptor
		(
			crgfx::BlendFactor srcColorBlendFactor,
			crgfx::BlendFactor dstColorBlendFactor,
			crgfx::BlendFactor srcAlphaBlendFactor,
			crgfx::BlendFactor dstAlphaBlendFactor,
			crgfx::ColorWriteMask colorWriteMask,
			crgfx::BlendOp colorBlendOp,
			crgfx::BlendOp alphaBlendOp,
			uint32_t enable
		)
		{
			this->bits = 0;
			this->srcColorBlendFactor = srcColorBlendFactor;
			this->dstColorBlendFactor = dstColorBlendFactor;
			this->srcAlphaBlendFactor = srcAlphaBlendFactor;
			this->dstAlphaBlendFactor = dstAlphaBlendFactor;
			this->colorWriteMask = colorWriteMask;
			this->colorBlendOp = colorBlendOp;
			this->alphaBlendOp = alphaBlendOp;
			this->enable = enable;
		}

		union
		{
			struct
			{
				crgfx::BlendFactor srcColorBlendFactor : 5;
				crgfx::BlendFactor dstColorBlendFactor : 5;

				crgfx::BlendFactor srcAlphaBlendFactor : 5;
				crgfx::BlendFactor dstAlphaBlendFactor : 5;
				crgfx::ColorWriteMask colorWriteMask : 4;
				crgfx::BlendOp colorBlendOp : 3;
				crgfx::BlendOp alphaBlendOp : 3;

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
		crstl::array<crgfx::CrRenderTargetBlendDescriptor, crgfx::MaxRenderTargets> renderTargetBlends;

		// See https://msdn.microsoft.com/en-us/library/windows/desktop/dn770339(v=vs.85).aspx for why logicOps is 
		// in the blend state and not a per render target field.
		uint32_t logicOpEnable : 1;
		crgfx::LogicOp logicOp : 4;
		uint32_t padding : 27;
		float blendConstants[4];
	};

	static_assert(sizeof(CrBlendStateDescriptor) == 52, "CrBlendStateDescriptor size mismatch");

	struct CrDepthStencilStateDescriptor
	{
		crgfx::CompareOp      depthCompareOp : 3;
		uint32_t              depthTestEnable : 1;
		uint32_t              depthWriteEnable : 1;
		uint32_t              depthBoundsTestEnable : 1;
		uint32_t              stencilTestEnable : 1;

		uint32_t              stencilReadMask : 8;
		uint32_t              stencilWriteMask : 8;
		uint32_t              reference : 8;

		uint32_t              padding : 1;

		crgfx::StencilOp      frontStencilFailOp : 3;
		crgfx::StencilOp      frontDepthFailOp : 3;
		crgfx::StencilOp      frontStencilPassOp : 3;
		crgfx::CompareOp      frontStencilCompareOp : 3;

		crgfx::StencilOp      backStencilFailOp : 3;
		crgfx::StencilOp      backDepthFailOp : 3;
		crgfx::StencilOp      backStencilPassOp : 3;
		crgfx::CompareOp      backStencilCompareOp : 3;

		uint32_t              padding2 : 8;

		float                 minDepthBounds;
		float                 maxDepthBounds;
	};

	static_assert(sizeof(CrDepthStencilStateDescriptor) == 16, "CrDepthStencilStateDescriptor size mismatch");

	struct CrRenderTargetFormatDescriptor
	{
		crstl::array<crgfx::DataFormat::T, crgfx::MaxRenderTargets> colorFormats =
		{
			crgfx::DataFormat::Invalid, crgfx::DataFormat::Invalid, crgfx::DataFormat::Invalid, crgfx::DataFormat::Invalid,
			crgfx::DataFormat::Invalid, crgfx::DataFormat::Invalid, crgfx::DataFormat::Invalid, crgfx::DataFormat::Invalid
		};

		crgfx::DataFormat::T depthFormat = crgfx::DataFormat::Invalid;
		crgfx::SampleCount sampleCount = crgfx::SampleCount::S1;
	};

	static_assert(sizeof(CrRenderTargetFormatDescriptor) == 40, "CrRenderTargetFormatDescriptor size mismatch");

	// TODO Optimize size of pipeline descriptor
	struct CrGraphicsPipelineDescriptor
	{
		CrGraphicsPipelineDescriptor()
		{
			primitiveTopology = crgfx::PrimitiveTopology::TriangleList;
			sampleCount = crgfx::SampleCount::S1;

			rasterizerState.fillMode = crgfx::PolygonFillMode::Fill;
			rasterizerState.frontFace = crgfx::FrontFace::Clockwise;
			rasterizerState.cullMode = crgfx::PolygonCullMode::Back;
			rasterizerState.depthClipEnable = true;

			padding = 0;

			// Don't put a loop here to initialize the color write masks
			blendState.renderTargetBlends[0].colorWriteMask = crgfx::ColorWriteComponent::All;
			blendState.renderTargetBlends[1].colorWriteMask = crgfx::ColorWriteComponent::All;
			blendState.renderTargetBlends[2].colorWriteMask = crgfx::ColorWriteComponent::All;
			blendState.renderTargetBlends[3].colorWriteMask = crgfx::ColorWriteComponent::All;
			blendState.renderTargetBlends[4].colorWriteMask = crgfx::ColorWriteComponent::All;
			blendState.renderTargetBlends[5].colorWriteMask = crgfx::ColorWriteComponent::All;
			blendState.renderTargetBlends[6].colorWriteMask = crgfx::ColorWriteComponent::All;
			blendState.renderTargetBlends[7].colorWriteMask = crgfx::ColorWriteComponent::All;

			depthStencilState.depthTestEnable = true;
			depthStencilState.depthWriteEnable = true;
			depthStencilState.depthCompareOp = crgfx::CompareOp::Greater; // Reverse depth by default
		}

		CrHash ComputeHash() const
		{
			return CrHash(this, sizeof(*this));
		}

		crgfx::PrimitiveTopology        primitiveTopology : 4;
		crgfx::SampleCount              sampleCount : 4;
		uint32_t                       padding : 24;

		crgfx::CrRasterizerStateDescriptor    rasterizerState = {};
		crgfx::CrBlendStateDescriptor         blendState = {};
		crgfx::CrDepthStencilStateDescriptor  depthStencilState = {};
		crgfx::CrRenderTargetFormatDescriptor renderTargets = {};
	};

	static_assert(sizeof(CrGraphicsPipelineDescriptor) == 128, "CrGraphicsPipelineDescriptor size mismatch");

	class ICrGraphicsPipeline : public CrGPUAutoDeletable
	{
	public:

		ICrGraphicsPipeline(crgfx::IDevice* renderDevice, const CrGraphicsPipelineDescriptor& pipelineDescriptor, const crgfx::CrGraphicsShaderHandle& graphicsShader, const CrVertexDescriptor& vertexDescriptor);

		virtual ~ICrGraphicsPipeline();

		const crgfx::CrGraphicsShaderHandle& GetShader() const { return m_shader; }

		uint32_t GetVertexStreamCount() const { return m_usedVertexStreamCount; }

	private:

		crgfx::CrGraphicsShaderHandle m_shader;

		uint32_t m_usedVertexStreamCount = 0;

#if !defined(CR_CONFIG_FINAL)

	public:

		void Recompile(crgfx::IDevice* renderDevice, const crgfx::CrGraphicsShaderHandle& graphicsShader);

		virtual void RecompilePS(crgfx::IDevice* renderDevice, const crgfx::CrGraphicsShaderHandle& graphicsShader) = 0;

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

		ICrComputePipeline(crgfx::IDevice* renderDevice, const crgfx::CrComputeShaderHandle& computeShader);

		virtual ~ICrComputePipeline();

		const crgfx::CrComputeShaderHandle& GetShader() const { return m_shader; }

		uint32_t GetGroupSizeX() const { return m_threadGroupSizeX; }

		uint32_t GetGroupSizeY() const { return m_threadGroupSizeY; }

		uint32_t GetGroupSizeZ() const { return m_threadGroupSizeZ; }

	private:

		uint32_t m_threadGroupSizeX;

		uint32_t m_threadGroupSizeY;

		uint32_t m_threadGroupSizeZ;

		crgfx::CrComputeShaderHandle m_shader;

#if !defined(CR_CONFIG_FINAL)

	public:

		void Recompile(crgfx::IDevice* renderDevice, const crgfx::CrComputeShaderHandle& computeShader);

		virtual void RecompilePS(crgfx::IDevice* renderDevice, const crgfx::CrComputeShaderHandle& computeShader) = 0;

		CrBuiltinCompute::T GetComputeShaderIndex() const { return m_computeShaderIndex; }

		void SetComputeShaderIndex(CrBuiltinCompute::T computeShaderIndex) { m_computeShaderIndex = computeShaderIndex; }

	private:

		CrBuiltinCompute::T m_computeShaderIndex = (CrBuiltinCompute::T)-1;

#endif
	};
};

// TODO Move to common graphics resources
namespace CrStandardPipelineStates
{
	extern crgfx::CrRenderTargetBlendDescriptor OpaqueBlend;
	extern crgfx::CrRenderTargetBlendDescriptor AlphaBlend;
};