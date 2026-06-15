#pragma once

#include "Core/CrCoreForwardDeclarations.h"

#include "Graphics/CrRenderingForwardDeclarations.h"

#include "CrGPUDeletable.h"

#include "crstl/fixed_string.h"

namespace crgfx
{
	struct SamplerDescriptor
	{
		SamplerDescriptor();

		crgfx::Filter minFilter;
		crgfx::Filter magFilter;
		crgfx::Filter mipmapFilter;
		crgfx::AddressMode addressModeU;
		crgfx::AddressMode addressModeV;
		crgfx::AddressMode addressModeW;
		uint32_t enableAnisotropy;
		uint32_t enableCompare;
		crgfx::CompareOp compareOp;
		crgfx::BorderColor borderColor;
		uint32_t maxAnisotropy;
		float mipLodBias;
		float minLod;
		float maxLod;

		crstl::fixed_string128 name;
	};

	class ISampler : public CrGPUAutoDeletable
	{
	public:

		virtual ~ISampler() {}

	protected:

		ISampler(crgfx::IDevice* renderDevice);
	};
};