#pragma once

#include "Graphics/CrGraphicsForwardDeclarations.h"
#include "Graphics/CrGraphics.h"
#include "Graphics/DataFormats.h"
#include "Graphics/GPUDeletable.h"
#include "Graphics/VertexDescriptor.h"

#include "Graphics/IShader.h"

#include "Core/CrHash.h"

#include "crstl/array.h"
#include "crstl/intrusive_ptr.h"

namespace CrBuiltinShaders { enum T : uint32_t; }

namespace CrBuiltinCompute { enum T : uint32_t; }

namespace crgfx
{
	struct RasterizerStateDescriptor
	{
		RasterizerStateDescriptor()
			: fillMode(crgfx::PolygonFillMode::Fill)
			, frontFace(crgfx::FrontFace::Clockwise)
			, cullMode(crgfx::PolygonCullMode::Back)
			, depthClipEnable(true)
			, multisampleEnable(false)
			, antialiasedLineEnable(false)
			, conservativeRasterization(false)
			, padding(0)
			, depthBias(0.0f)
			, depthBiasClamp(0.0f)
			, slopeScaledDepthBias(0.0f)
		{}

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

	static_assert(sizeof(RasterizerStateDescriptor) == 16, "RasterizerStateDescriptor size mismatch");

	struct RenderTargetBlendDescriptor
	{
		RenderTargetBlendDescriptor()
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

		RenderTargetBlendDescriptor
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

		bool operator == (const RenderTargetBlendDescriptor& other) const { return bits == other.bits; }

		bool operator != (const RenderTargetBlendDescriptor& other) const { return bits != other.bits; }
	};

	static_assert(sizeof(RenderTargetBlendDescriptor) == 4, "RenderTargetBlendDescriptor size mismatch");

	struct BlendStateDescriptor
	{
		crstl::array<crgfx::RenderTargetBlendDescriptor, crgfx::MaxRenderTargets> renderTargetBlends;

		// See https://msdn.microsoft.com/en-us/library/windows/desktop/dn770339(v=vs.85).aspx for why logicOps is 
		// in the blend state and not a per render target field.
		uint32_t logicOpEnable : 1;
		crgfx::LogicOp logicOp : 4;
		uint32_t padding : 27;
		float blendConstants[4];
	};

	static_assert(sizeof(BlendStateDescriptor) == 52, "BlendStateDescriptor size mismatch");

	struct DepthStencilStateDescriptor
	{
		DepthStencilStateDescriptor()
			: depthCompareOp(crgfx::CompareOp::Greater) // Reverse depth by default
			, depthTestEnable(true)
			, depthWriteEnable(true)
			, depthBoundsTestEnable(false)
			, stencilTestEnable(false)
			, padding(0)

			, stencilReadMask(0)
			, stencilWriteMask(0)
			, reference(0)

			, frontStencilFailOp(crgfx::StencilOp::Keep)
			, frontDepthFailOp(crgfx::StencilOp::Keep)
			, frontStencilPassOp(crgfx::StencilOp::Keep)
			, frontStencilCompareOp(crgfx::CompareOp::Never)
			 
			, backStencilFailOp(crgfx::StencilOp::Keep)
			, backDepthFailOp(crgfx::StencilOp::Keep)
			, backStencilPassOp(crgfx::StencilOp::Keep)
			, backStencilCompareOp(crgfx::CompareOp::Never)
			 
			, padding2(0)

			, minDepthBounds(0.0f)
			, maxDepthBounds(0.0f)
		{}

		crgfx::CompareOp      depthCompareOp : 3;
		uint32_t              depthTestEnable : 1;
		uint32_t              depthWriteEnable : 1;
		uint32_t              depthBoundsTestEnable : 1;
		uint32_t              stencilTestEnable : 1;
		uint32_t              padding : 1;

		uint32_t              stencilReadMask : 8;
		uint32_t              stencilWriteMask : 8;
		uint32_t              reference : 8;

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

	static_assert(sizeof(DepthStencilStateDescriptor) == 16, "DepthStencilStateDescriptor size mismatch");

	struct RenderTargetFormatDescriptor
	{
		crstl::array<crgfx::DataFormat::T, crgfx::MaxRenderTargets> colorFormats =
		{
			crgfx::DataFormat::Invalid, crgfx::DataFormat::Invalid, crgfx::DataFormat::Invalid, crgfx::DataFormat::Invalid,
			crgfx::DataFormat::Invalid, crgfx::DataFormat::Invalid, crgfx::DataFormat::Invalid, crgfx::DataFormat::Invalid
		};

		crgfx::DataFormat::T depthFormat = crgfx::DataFormat::Invalid;
		crgfx::SampleCount sampleCount = crgfx::SampleCount::S1;
	};

	static_assert(sizeof(RenderTargetFormatDescriptor) == 40, "CrRenderTargetFormatDescriptor size mismatch");

	// TODO Optimize size of pipeline descriptor
	struct GraphicsPipelineDescriptor
	{
		GraphicsPipelineDescriptor()
		{
			primitiveTopology = crgfx::PrimitiveTopology::TriangleList;
			sampleCount = crgfx::SampleCount::S1;
			padding = 0;
		}

		CrHash ComputeHash() const
		{
			return CrHash(this, sizeof(*this));
		}

		crgfx::PrimitiveTopology        primitiveTopology : 4;
		crgfx::SampleCount              sampleCount : 4;
		uint32_t                        padding : 24;

		crgfx::RasterizerStateDescriptor    rasterizerState;
		crgfx::BlendStateDescriptor         blendState;
		crgfx::DepthStencilStateDescriptor  depthStencilState;
		crgfx::RenderTargetFormatDescriptor renderTargets;
	};

	static_assert(sizeof(GraphicsPipelineDescriptor) == 128, "GraphicsPipelineDescriptor size mismatch");

	class IGraphicsPipeline : public GPUAutoDeletable
	{
	public:

		IGraphicsPipeline(crgfx::IDevice* renderDevice, const GraphicsPipelineDescriptor& pipelineDescriptor, const VertexDescriptor& vertexDescriptor);

		virtual ~IGraphicsPipeline();

		uint32_t GetVertexStreamCount() const { return m_usedVertexStreamCount; }

		const ShaderBindingLayout& GetBindingLayout() const { return m_bindingLayout; }

	protected:

		uint32_t m_usedVertexStreamCount = 0;

		ShaderBindingLayout m_bindingLayout;

#if !defined(CR_CONFIG_FINAL)

	public:

		void Recompile(IDevice* renderDevice, const GraphicsShaderBytecode& graphicsShader);

		virtual void RecompilePS(IDevice* renderDevice, const GraphicsShaderBytecode& graphicsShader) = 0;

		CrBuiltinShaders::T GetVertexShaderIndex() const { return m_vertexShaderIndex; }

		CrBuiltinShaders::T GetPixelShaderIndex() const { return m_pixelShaderIndex; }

		void SetShaderIndices(CrBuiltinShaders::T vertexShaderIndex, CrBuiltinShaders::T pixelShaderIndex)
		{
			m_vertexShaderIndex = vertexShaderIndex;
			m_pixelShaderIndex = pixelShaderIndex;
		}

	protected:

		GraphicsPipelineDescriptor m_pipelineDescriptor;

		VertexDescriptor m_vertexDescriptor;

		CrBuiltinShaders::T m_vertexShaderIndex = (CrBuiltinShaders::T)-1;

		CrBuiltinShaders::T m_pixelShaderIndex = (CrBuiltinShaders::T)-1;

#endif
	};

	class IComputePipeline : public GPUAutoDeletable
	{
	public:

		IComputePipeline(IDevice* renderDevice, const ComputeShaderBytecode& computeShaderBytecode);

		virtual ~IComputePipeline();

		const ShaderBindingLayout& GetBindingLayout() const { return m_bindingLayout; }

		uint32_t GetGroupSizeX() const { return m_threadGroupSizeX; }

		uint32_t GetGroupSizeY() const { return m_threadGroupSizeY; }

		uint32_t GetGroupSizeZ() const { return m_threadGroupSizeZ; }

	protected:

		uint32_t m_threadGroupSizeX;

		uint32_t m_threadGroupSizeY;

		uint32_t m_threadGroupSizeZ;

		ShaderBindingLayout m_bindingLayout;

#if !defined(CR_CONFIG_FINAL)

	public:

		void Recompile(IDevice* renderDevice, const ComputeShaderBytecode& computeShaderBytecode);

		virtual void RecompilePS(IDevice* renderDevice, const ComputeShaderBytecode& computeShaderBytecode) = 0;

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
	extern crgfx::RenderTargetBlendDescriptor OpaqueBlend;
	extern crgfx::RenderTargetBlendDescriptor AlphaBlend;
};