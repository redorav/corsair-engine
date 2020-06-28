#pragma once

#include "Core/CrCoreForwardDeclarations.h"

struct CrSamplerDescriptor
{
	cr3d::Filter minFilter					: 1;
	cr3d::Filter magFilter					: 1;
	cr3d::Filter mipmapFilter				: 1;
	cr3d::AddressMode addressModeU			: 3;
	cr3d::AddressMode addressModeV			: 3;
	cr3d::AddressMode addressModeW			: 3;
	uint32_t enableAnisotropy				: 1;
	uint32_t enableCompare					: 1;
	cr3d::CompareOp compareOp				: 3;
	cr3d::BorderColor borderColor			: 2;
	float mipLodBias;
	float maxAnisotropy;
	float minLod;
	float maxLod;

	CrSamplerDescriptor();
};

class ICrSampler
{
public:

protected:

	ICrSampler(const CrSamplerDescriptor&);
};