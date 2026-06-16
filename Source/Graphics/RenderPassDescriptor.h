#pragma once

#include "Graphics/CrRenderingForwardDeclarations.h"

#include "Math/CrHlslppVectorFloatType.h"

#include "crstl/fixed_vector.h"

namespace crgfx
{
	using CrRenderPassNameString = crstl::fixed_string32;

	// For render target descriptors, the usage is implicit to some extent. Color render targets always
	// have a single state (RenderTarget) and depth targets may have a combination of stencil, depth, or both.
	// For other types of resources we need to specify what the resource starts off with, how we want to use it
	// during the pass, and what we transition it to after we finish.
	struct RenderTargetDescriptor
	{
		RenderTargetDescriptor()
			: texture(nullptr)
			, mipmap(0)
			, slice(0)
			, clearColor(0.0f)
			, depthClearValue(0.0f)
			, stencilClearValue(0)
			, loadOp(crgfx::RenderTargetLoadOp::Load)
			, storeOp(crgfx::RenderTargetStoreOp::Store)
			, stencilLoadOp(crgfx::RenderTargetLoadOp::DontCare)
			, stencilStoreOp(crgfx::RenderTargetStoreOp::DontCare)
		{
		}

		const crgfx::ITexture* texture;
		uint32_t mipmap;
		uint32_t slice;

		float4 clearColor;
		float depthClearValue;
		uint8_t stencilClearValue;

		crgfx::RenderTargetLoadOp loadOp;
		crgfx::RenderTargetStoreOp storeOp;
		crgfx::RenderTargetLoadOp stencilLoadOp;
		crgfx::RenderTargetStoreOp stencilStoreOp;
		crgfx::TextureState initialState;
		crgfx::TextureState usageState;
		crgfx::TextureState finalState;
	};

	// For buffers and textures that aren't used as render targets, we split them into two parts. Even if some data is duplicated in some cases,
	// we can better control which transitions get added to the beginning and end of the pass (sometimes there is a transition at the beginning
	// but not at the end of a pass, if the states are the same)
	struct RenderPassBufferDescriptor
	{
		RenderPassBufferDescriptor
		(
			const ICrHardwareGPUBuffer* hardwareBuffer, uint32_t numElements, uint32_t stride, uint32_t offset,
			crgfx::BufferState::T sourceState, crgfx::ShaderStageFlags::T sourceShaderStages,
			crgfx::BufferState::T destinationState, crgfx::ShaderStageFlags::T destinationShaderStages)
			: hardwareBuffer(hardwareBuffer), numElements(numElements), stride(stride), offset(offset), sourceState(sourceState), sourceShaderStages(sourceShaderStages),
			destinationState(destinationState), destinationShaderStages(destinationShaderStages) {
		}

		const ICrHardwareGPUBuffer* hardwareBuffer;
		uint32_t numElements;
		uint32_t stride;
		uint32_t offset;

		crgfx::BufferState::T sourceState;
		crgfx::ShaderStageFlags::T sourceShaderStages;
		crgfx::BufferState::T destinationState;
		crgfx::ShaderStageFlags::T destinationShaderStages;
	};

	struct RenderPassTextureDescriptor
	{
		RenderPassTextureDescriptor(const crgfx::ITexture* texture, uint32_t mipmapStart, uint32_t mipmapCount,
			uint32_t sliceStart, uint32_t sliceCount, crgfx::TexturePlane::T texturePlane, const crgfx::TextureState& sourceState, const crgfx::TextureState& destinationState)
			: texture(texture), mipmapStart(mipmapStart), mipmapCount(mipmapCount), sliceStart(sliceStart), sliceCount(sliceCount), texturePlane(texturePlane)
			, sourceState(sourceState), destinationState(destinationState) {
		}

		const crgfx::ITexture* texture;

		uint32_t mipmapStart;
		uint32_t mipmapCount;

		uint32_t sliceStart;
		uint32_t sliceCount;

		crgfx::TexturePlane::T texturePlane;

		crgfx::TextureState sourceState;
		crgfx::TextureState destinationState;
	};

	struct RenderPassDescriptor
	{
		static const uint32_t MaxTransitionCount = 32;

		typedef crstl::fixed_vector<RenderPassBufferDescriptor, MaxTransitionCount> BufferTransitionVector;
		typedef crstl::fixed_vector<RenderPassTextureDescriptor, MaxTransitionCount> TextureTransitionVector;

		crgfx::RenderPassType::T type;

		CrRenderPassNameString debugName;
		float4 debugColor;

		crstl::fixed_vector<RenderTargetDescriptor, crgfx::MaxRenderTargets> color;
		RenderTargetDescriptor depth;

		// Transitions when beginning a pass
		BufferTransitionVector beginBuffers;
		TextureTransitionVector beginTextures;

		// Transitions when ending a pass
		BufferTransitionVector endBuffers;
		TextureTransitionVector endTextures;
	};
};