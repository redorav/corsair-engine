#include "Graphics/CrRendering_pch.h"
#include "ICrSampler.h"

#include "Core/CrMacros.h"

#include "Graphics/CrRendering.h"

CrSamplerDescriptor::CrSamplerDescriptor() 
	: minFilter(crgfx::Filter::Linear)
	, magFilter(crgfx::Filter::Linear)
	, mipmapFilter(crgfx::Filter::Linear)
	, addressModeU(crgfx::AddressMode::ClampToEdge)
	, addressModeV(crgfx::AddressMode::ClampToEdge)
	, addressModeW(crgfx::AddressMode::ClampToEdge)
	, enableAnisotropy(false)
	, enableCompare(false)
	, compareOp(crgfx::CompareOp::Never)
	, borderColor(crgfx::BorderColor::OpaqueBlack)
	, maxAnisotropy(1)
	, mipLodBias(0.0f)
	, minLod(0.0f)
	, maxLod(1024.0f) // Some arbitrarily big number to mean no clamp
{

}

ICrSampler::ICrSampler(crgfx::ICrRenderDevice* renderDevice) : CrGPUAutoDeletable(renderDevice)
{
	m_renderDevice = renderDevice;
}