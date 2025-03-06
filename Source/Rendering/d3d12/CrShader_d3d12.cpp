#include "Rendering/CrRendering_pch.h"
#include "CrShader_d3d12.h"
#include "CrRenderDevice_d3d12.h"

#include "Rendering/CrShaderResourceMetadata.h"
#include "Rendering/ICrShader.inl"

#include "Core/Logging/ICrDebug.h"
#include "Core/CrMacros.h"

CrGraphicsShaderD3D12::CrGraphicsShaderD3D12(ICrRenderDevice* renderDevice, const CrGraphicsShaderDescriptor& graphicsShaderDescriptor)
	: ICrGraphicsShader(renderDevice, graphicsShaderDescriptor)
{
	CrShaderBindingLayoutResources resources;

	// Create the shader modules and parse reflection information
	for (const CrShaderBytecodeHandle& shaderBytecode : graphicsShaderDescriptor.m_bytecodes)
	{
		const CrShaderReflectionHeader& reflectionHeader = shaderBytecode->GetReflection();
		ICrShaderBindingLayout::AddResources(reflectionHeader, resources, [](cr3d::ShaderStage::T, const CrShaderReflectionResource&){});
	}

	m_bindingLayout = crstl::unique_ptr<ICrShaderBindingLayout>(new ICrShaderBindingLayout(resources));
}

CrGraphicsShaderD3D12::~CrGraphicsShaderD3D12()
{

}

CrComputeShaderD3D12::CrComputeShaderD3D12(ICrRenderDevice* renderDevice, const CrComputeShaderDescriptor& computeShaderDescriptor)
	: ICrComputeShader(renderDevice, computeShaderDescriptor)
{
	CrShaderBindingLayoutResources resources;
	const CrShaderReflectionHeader& reflectionHeader = computeShaderDescriptor.m_bytecode->GetReflection();
	ICrShaderBindingLayout::AddResources(reflectionHeader, resources, [](cr3d::ShaderStage::T, const CrShaderReflectionResource&){});
	m_bindingLayout = crstl::unique_ptr<ICrShaderBindingLayout>(new ICrShaderBindingLayout(resources));
}
