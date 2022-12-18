#include "CrRendering_pch.h"
#include "ICrSampler.h"

#include "Core/CrMacros.h"
#include "Core/SmartPointers/CrSharedPtr.h"

#include "Rendering/CrRendering.h"

CrSamplerDescriptor::CrSamplerDescriptor() 
	: minFilter(cr3d::Filter::Linear)
	, magFilter(cr3d::Filter::Linear)
	, mipmapFilter(cr3d::Filter::Linear)
	, addressModeU(cr3d::AddressMode::ClampToEdge)
	, addressModeV(cr3d::AddressMode::ClampToEdge)
	, addressModeW(cr3d::AddressMode::ClampToEdge)
	, mipLodBias(0.0f)
	, enableAnisotropy(false)
	, maxAnisotropy(1)
	, enableCompare(false)
	, compareOp(cr3d::CompareOp::Always)
	, minLod(0.0f)
	, maxLod(1024.0f) // Some arbitrarily big number to mean no clamp
	, borderColor(cr3d::BorderColor::OpaqueBlack)
{

}

ICrSampler::ICrSampler(ICrRenderDevice* renderDevice)
{
	m_renderDevice = renderDevice;
}