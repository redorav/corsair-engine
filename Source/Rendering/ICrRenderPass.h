#pragma once

#include "ICrPipelineStateManager.h"

#include "Core/Containers/CrArray.h"

typedef struct CrRect2D
{
	int32_t x, y;
	uint32_t width, height;
} CrRect2D;

struct CrRenderPassBeginParams
{
	float4 colorClearValue;
	float depthClearValue;
	uint32_t stencilClearValue;
	CrRect2D drawArea;
	bool clear;
};

enum class CrAttachmentLoadOp
{
	Load, Clear, DontCare
};

enum class CrAttachmentStoreOp
{
	Store, DontCare
};

struct CrAttachmentDescriptor
{
	CrAttachmentDescriptor();

	CrAttachmentDescriptor(cr3d::DataFormat::T format, cr3d::SampleCount samples,
						   CrAttachmentLoadOp loadOp, CrAttachmentStoreOp storeOp,
						   CrAttachmentLoadOp stencilLoadOp, CrAttachmentStoreOp stencilStoreOp,
						   cr3d::ResourceState::T initialState, cr3d::ResourceState::T finalState)
		: format(format)
		, samples(samples)
		, loadOp(loadOp)
		, storeOp(storeOp)
		, stencilLoadOp(stencilLoadOp)
		, stencilStoreOp(stencilStoreOp)
		, initialState(initialState)
		, finalState(finalState)
	{}

	cr3d::DataFormat::T format;
	cr3d::SampleCount samples;

	CrAttachmentLoadOp loadOp; // Applies to color or depth
	CrAttachmentStoreOp storeOp;

	CrAttachmentLoadOp stencilLoadOp; // Ignored if attachment is color
	CrAttachmentStoreOp stencilStoreOp;

	cr3d::ResourceState::T initialState;
	cr3d::ResourceState::T finalState;
};

struct CrRenderPassDescriptor : CrAutoHashable<CrRenderPassDescriptor>
{
	CrArray<CrAttachmentDescriptor, cr3d::MaxRenderTargets> m_colorAttachments;
	CrAttachmentDescriptor m_depthAttachment;
};

class ICrRenderPass
{
protected:

	ICrRenderPass() {}

private:
};
