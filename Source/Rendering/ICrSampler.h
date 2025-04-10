#pragma once

#include "Core/CrCoreForwardDeclarations.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

#include "CrGPUDeletable.h"

#include "crstl/fixed_string.h"

struct CrSamplerDescriptor
{
	CrSamplerDescriptor();

	cr3d::Filter minFilter;
	cr3d::Filter magFilter;
	cr3d::Filter mipmapFilter;
	cr3d::AddressMode addressModeU;
	cr3d::AddressMode addressModeV;
	cr3d::AddressMode addressModeW;
	uint32_t enableAnisotropy;
	uint32_t enableCompare;
	cr3d::CompareOp compareOp;
	cr3d::BorderColor borderColor;
	uint32_t maxAnisotropy;
	float mipLodBias;
	float minLod;
	float maxLod;

	crstl::fixed_string128 name;
};

class ICrSampler : public CrGPUAutoDeletable
{
public:

	virtual ~ICrSampler() {}

protected:

	ICrSampler(ICrRenderDevice* renderDevice);
};