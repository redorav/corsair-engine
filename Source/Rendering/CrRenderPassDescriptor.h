#pragma once

#include "Core/Containers/CrFixedVector.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

struct CrRenderTargetDescriptor
{
	CrRenderTargetDescriptor()
		: texture(nullptr)
		, mipMap(0)
		, slice(0)
		, clearColor(0.0f)
		, depthClearValue(0.0f)
		, stencilClearValue(0)
		, loadOp(CrRenderTargetLoadOp::Load)
		, storeOp(CrRenderTargetStoreOp::Store)
		, stencilLoadOp(CrRenderTargetLoadOp::DontCare)
		, stencilStoreOp(CrRenderTargetStoreOp::DontCare)
		, initialState(cr3d::TextureState::Undefined)
		, finalState(cr3d::TextureState::RenderTarget)
	{}

	const ICrTexture* texture;
	uint32_t mipMap;
	uint32_t slice;

	float4 clearColor;
	float depthClearValue;
	uint32_t stencilClearValue;

	CrRenderTargetLoadOp loadOp;
	CrRenderTargetStoreOp storeOp;
	CrRenderTargetLoadOp stencilLoadOp;
	CrRenderTargetStoreOp stencilStoreOp;
	cr3d::TextureState::T initialState;
	cr3d::TextureState::T finalState;
};

struct CrRenderPassDescriptor
{
	CrFixedVector<CrRenderTargetDescriptor, cr3d::MaxRenderTargets> color;
	CrRenderTargetDescriptor depth;
};