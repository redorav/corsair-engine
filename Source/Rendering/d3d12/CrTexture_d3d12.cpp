#include "CrRendering_pch.h"

#include "CrCommandQueue_d3d12.h"
#include "CrCommandBuffer_d3d12.h"
#include "CrTexture_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrD3D12.h"

#include "Core/Logging/ICrDebug.h"

CrTextureD3D12::CrTextureD3D12(ICrRenderDevice* renderDevice, const CrTextureCreateParams& params)
	: ICrTexture(params)
{
	unused_parameter(renderDevice);
	unused_parameter(params);
}

CrTextureD3D12::~CrTextureD3D12()
{
	
}