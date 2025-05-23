#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"

#include "Math/CrHlslppVectorFloatType.h"

#include "crstl/fixed_vector.h"

using CrRenderPassNameString = crstl::fixed_string32;

// For render target descriptors, the usage is implicit to some extent. Color render targets always
// have a single state (RenderTarget) and depth targets may have a combination of stencil, depth, or both.
// For other types of resources we need to specify what the resource starts off with, how we want to use it
// during the pass, and what we transition it to after we finish.
struct CrRenderTargetDescriptor
{
	CrRenderTargetDescriptor()
		: texture(nullptr)
		, mipmap(0)
		, slice(0)
		, clearColor(0.0f)
		, depthClearValue(0.0f)
		, stencilClearValue(0)
		, loadOp(CrRenderTargetLoadOp::Load)
		, storeOp(CrRenderTargetStoreOp::Store)
		, stencilLoadOp(CrRenderTargetLoadOp::DontCare)
		, stencilStoreOp(CrRenderTargetStoreOp::DontCare)
	{}

	const ICrTexture* texture;
	uint32_t mipmap;
	uint32_t slice;

	float4 clearColor;
	float depthClearValue;
	uint8_t stencilClearValue;

	CrRenderTargetLoadOp loadOp;
	CrRenderTargetStoreOp storeOp;
	CrRenderTargetLoadOp stencilLoadOp;
	CrRenderTargetStoreOp stencilStoreOp;
	cr3d::TextureState initialState;
	cr3d::TextureState usageState;
	cr3d::TextureState finalState;
};

// For buffers and textures that aren't used as render targets, we split them into two parts. Even if some data is duplicated in some cases,
// we can better control which transitions get added to the beginning and end of the pass (sometimes there is a transition at the beginning
// but not at the end of a pass, if the states are the same)
struct CrRenderPassBufferDescriptor
{
	CrRenderPassBufferDescriptor
	(
		const ICrHardwareGPUBuffer* hardwareBuffer, uint32_t numElements, uint32_t stride, uint32_t offset,
		cr3d::BufferState::T sourceState, cr3d::ShaderStageFlags::T sourceShaderStages, 
		cr3d::BufferState::T destinationState, cr3d::ShaderStageFlags::T destinationShaderStages)
		: hardwareBuffer(hardwareBuffer), numElements(numElements), stride(stride), offset(offset), sourceState(sourceState), sourceShaderStages(sourceShaderStages),
		destinationState(destinationState), destinationShaderStages(destinationShaderStages) {}
	
	const ICrHardwareGPUBuffer* hardwareBuffer;
	uint32_t numElements;
	uint32_t stride;
	uint32_t offset;

	cr3d::BufferState::T sourceState;
	cr3d::ShaderStageFlags::T sourceShaderStages;
	cr3d::BufferState::T destinationState;
	cr3d::ShaderStageFlags::T destinationShaderStages;
};

struct CrRenderPassTextureDescriptor
{
	CrRenderPassTextureDescriptor(const ICrTexture* texture, uint32_t mipmapStart, uint32_t mipmapCount, 
		uint32_t sliceStart, uint32_t sliceCount, cr3d::TexturePlane::T texturePlane, const cr3d::TextureState& sourceState, const cr3d::TextureState& destinationState)
		: texture(texture), mipmapStart(mipmapStart), mipmapCount(mipmapCount), sliceStart(sliceStart), sliceCount(sliceCount), texturePlane(texturePlane)
		, sourceState(sourceState), destinationState(destinationState) {}

	const ICrTexture* texture;

	uint32_t mipmapStart;
	uint32_t mipmapCount;

	uint32_t sliceStart;
	uint32_t sliceCount;

	cr3d::TexturePlane::T texturePlane;

	cr3d::TextureState sourceState;
	cr3d::TextureState destinationState;
};

struct CrRenderPassDescriptor
{
	static const uint32_t MaxTransitionCount = 32;

	typedef crstl::fixed_vector<CrRenderPassBufferDescriptor, MaxTransitionCount> BufferTransitionVector;
	typedef crstl::fixed_vector<CrRenderPassTextureDescriptor, MaxTransitionCount> TextureTransitionVector;

	cr3d::RenderPassType::T type;
	
	CrRenderPassNameString debugName;
	float4 debugColor;

	crstl::fixed_vector<CrRenderTargetDescriptor, cr3d::MaxRenderTargets> color;
	CrRenderTargetDescriptor depth;

	// Transitions when beginning a pass
	BufferTransitionVector beginBuffers;
	TextureTransitionVector beginTextures;

	// Transitions when ending a pass
	BufferTransitionVector endBuffers;
	TextureTransitionVector endTextures;
};