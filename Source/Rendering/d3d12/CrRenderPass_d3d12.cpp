#include "CrRendering_pch.h"

#include "CrRenderPass_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrCommandBuffer_d3d12.h"

#include "Core/Logging/ICrDebug.h"

CrRenderPassD3D12::CrRenderPassD3D12(ICrRenderDevice* renderDevice, const CrRenderPassDescriptor& renderPassDescriptor)
{
	unused_parameter(renderDevice);
	unused_parameter(renderPassDescriptor);
}

CrRenderPassD3D12::~CrRenderPassD3D12()
{
	
}