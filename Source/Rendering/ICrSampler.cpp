#include "Rendering/CrRendering_pch.h"
#include "ICrSampler.h"

#include "Core/CrMacros.h"

#include "Rendering/CrRendering.h"

CrSamplerDescriptor::CrSamplerDescriptor() 
	: minFilter(cr3d::Filter::Linear)
	, magFilter(cr3d::Filter::Linear)
	, mipmapFilter(cr3d::Filter::Linear)
	, addressModeU(cr3d::AddressMode::ClampToEdge)
	, addressModeV(cr3d::AddressMode::ClampToEdge)
	, addressModeW(cr3d::AddressMode::ClampToEdge)
	, enableAnisotropy(false)
	, enableCompare(false)
	, compareOp(cr3d::CompareOp::Always)
	, borderColor(cr3d::BorderColor::OpaqueBlack)
	, maxAnisotropy(1)
	, mipLodBias(0.0f)
	, minLod(0.0f)
	, maxLod(1024.0f) // Some arbitrarily big number to mean no clamp
{

}

ICrSampler::ICrSampler(ICrRenderDevice* renderDevice) : CrGPUDeletable(renderDevice)
{
	m_renderDevice = renderDevice;
}