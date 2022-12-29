#pragma once

#include "Rendering/CrRenderingForwardDeclarations.h"

#include "Core/SmartPointers/CrIntrusivePtr.h"

// Rendering resources that are used as default
class CrRenderingResources
{
public:

	static CrRenderingResources& Get();

	void Initialize(ICrRenderDevice* renderDevice);

	void Deinitialize();

	// Global samplers

	CrSamplerHandle AllLinearClampSampler;

	CrSamplerHandle AllLinearWrapSampler;

	CrSamplerHandle AllPointClampSampler;

	CrSamplerHandle AllPointWrapSampler;

	CrTextureHandle WhiteSmallTexture;

	CrTextureHandle BlackSmallTexture;

	// Points up in tangent space
	CrTextureHandle NormalsSmallTexture;
};