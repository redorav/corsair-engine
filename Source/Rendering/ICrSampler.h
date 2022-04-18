#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Core/String/CrFixedString.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

#include "CrGPUDeletable.h"

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

	CrFixedString128 name;
};

class ICrSampler : public CrGPUDeletable
{
public:

	virtual ~ICrSampler() {}

protected:

	ICrSampler(ICrRenderDevice* renderDevice);

	ICrRenderDevice* m_renderDevice = nullptr;
};