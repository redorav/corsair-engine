#include "CrRendering_pch.h"

#include "CrFramebuffer_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrTexture_d3d12.h"
#include "CrD3D12.h"

#include "Core/Logging/ICrDebug.h"

CrFramebufferD3D12::~CrFramebufferD3D12()
{
	
}

CrFramebufferD3D12::CrFramebufferD3D12(ICrRenderDevice* renderDevice, const CrFramebufferCreateParams& params) : ICrFramebuffer(params)
{
	
}
