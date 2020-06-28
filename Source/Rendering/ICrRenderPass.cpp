#include "CrRendering_pch.h"

#include "ICrRenderPass.h"

#include "Core/CrMacros.h"

CrAttachmentDescriptor::CrAttachmentDescriptor() : format(cr3d::DataFormat::Invalid)
, samples(cr3d::SampleCount::S1)
, loadOp(CrAttachmentLoadOp::DontCare)
, storeOp(CrAttachmentStoreOp::DontCare)
, stencilLoadOp(CrAttachmentLoadOp::DontCare)
, stencilStoreOp(CrAttachmentStoreOp::DontCare)
, initialState(cr3d::ResourceState::Undefined)
, finalState(cr3d::ResourceState::Undefined)
{

}
