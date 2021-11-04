#include "CrRendering_pch.h"
#include "CrShader_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrShaderReflection_d3d12.h"

#include "Rendering/CrShaderResourceMetadata.h"

#include "Core/Logging/ICrDebug.h"

static ICrShaderBindingTable* CreateBindingTable(const ICrRenderDevice* renderDevice, const CrShaderReflectionD3D12& vulkanReflection)
{
	unused_parameter(renderDevice);
	unused_parameter(vulkanReflection);
	return nullptr;
}

CrGraphicsShaderD3D12::CrGraphicsShaderD3D12(const ICrRenderDevice* renderDevice, const CrGraphicsShaderDescriptor& graphicsShaderDescriptor)
	: ICrGraphicsShader(renderDevice, graphicsShaderDescriptor)
{
	m_d3d12Device = static_cast<const CrRenderDeviceD3D12*>(renderDevice)->GetD3D12Device();

	CrShaderReflectionD3D12 d3d12Reflection;
	
	// Create the shader modules and parse reflection information
	for (const CrShaderBytecodeSharedHandle& shaderBytecode : graphicsShaderDescriptor.m_bytecodes)
	{
		d3d12Reflection.AddBytecode(shaderBytecode);
	}
	
	// Create the optimized shader resource table
	m_bindingTable = CrUniquePtr<ICrShaderBindingTable>(CreateBindingTable(renderDevice, d3d12Reflection));
}

CrGraphicsShaderD3D12::~CrGraphicsShaderD3D12()
{

}

CrComputeShaderD3D12::CrComputeShaderD3D12(const ICrRenderDevice* renderDevice, const CrComputeShaderDescriptor& computeShaderDescriptor)
	: ICrComputeShader(renderDevice, computeShaderDescriptor)
{
	m_d3d12Device = static_cast<const CrRenderDeviceD3D12*>(renderDevice)->GetD3D12Device();
}
