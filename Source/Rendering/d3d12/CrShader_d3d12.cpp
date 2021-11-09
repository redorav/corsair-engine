#include "CrRendering_pch.h"
#include "CrShader_d3d12.h"
#include "CrRenderDevice_d3d12.h"

#include "Rendering/CrShaderResourceMetadata.h"

#include "Core/Logging/ICrDebug.h"
#include "Core/CrMacros.h"

CrGraphicsShaderD3D12::CrGraphicsShaderD3D12(const ICrRenderDevice* renderDevice, const CrGraphicsShaderDescriptor& graphicsShaderDescriptor)
	: ICrGraphicsShader(renderDevice, graphicsShaderDescriptor)
{
	m_d3d12Device = static_cast<const CrRenderDeviceD3D12*>(renderDevice)->GetD3D12Device();
}

CrGraphicsShaderD3D12::~CrGraphicsShaderD3D12()
{

}

CrComputeShaderD3D12::CrComputeShaderD3D12(const ICrRenderDevice* renderDevice, const CrComputeShaderDescriptor& computeShaderDescriptor)
	: ICrComputeShader(renderDevice, computeShaderDescriptor)
{
	m_d3d12Device = static_cast<const CrRenderDeviceD3D12*>(renderDevice)->GetD3D12Device();
}
