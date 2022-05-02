#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"

// Rendering resources that are used as default
class CrRenderingResources
{
public:

	static CrRenderingResources& Get();

	void Initialize(ICrRenderDevice* renderDevice);

	void Deinitialize();

	// Global samplers

	CrSamplerSharedHandle AllLinearClampSampler;

	CrSamplerSharedHandle AllLinearWrapSampler;

	CrSamplerSharedHandle AllPointClampSampler;

	CrSamplerSharedHandle AllPointWrapSampler;
};