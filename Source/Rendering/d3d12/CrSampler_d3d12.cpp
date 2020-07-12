#include "CrRendering_pch.h"
#include "CrSampler_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrD3D12.h"

#include "Core/Logging/ICrDebug.h"

CrSamplerD3D12::CrSamplerD3D12(ICrRenderDevice* renderDevice, const CrSamplerDescriptor& descriptor) : ICrSampler(descriptor)
{
	renderDevice; descriptor;
	CrAssertMsg(false, "Not implemented");
}

CrSamplerD3D12::~CrSamplerD3D12()
{
	CrAssertMsg(false, "Not implemented");
}
