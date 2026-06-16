#include "Graphics/CrRendering_pch.h"
#include "CrShaderD3D12.h"
#include "DeviceD3D12.h"

#include "Graphics/CrShaderResourceMetadata.h"
#include "Graphics/IShader.inl"

#include "Core/Logging/ICrDebug.h"
#include "Core/CrMacros.h"

namespace crgfx
{
	GraphicsShaderD3D12::GraphicsShaderD3D12(crgfx::IDevice* renderDevice, const crgfx::GraphicsShaderDescriptor& graphicsShaderDescriptor)
		: IGraphicsShader(renderDevice, graphicsShaderDescriptor)
	{
		ShaderBindingLayoutResources resources;

		// Create the shader modules and parse reflection information
		for (const ShaderBytecodeHandle& shaderBytecode : graphicsShaderDescriptor.m_bytecodes)
		{
			const CrShaderReflectionHeader& reflectionHeader = shaderBytecode->GetReflection();
			ShaderBindingLayout::AddResources(reflectionHeader, resources, [](crgfx::ShaderStage::T, const CrShaderReflectionResource&){});
		}

		m_bindingLayout = crstl::unique_ptr<ShaderBindingLayout>(new ShaderBindingLayout(resources));
	}

	GraphicsShaderD3D12::~GraphicsShaderD3D12()
	{

	}

	ComputeShaderD3D12::ComputeShaderD3D12(crgfx::IDevice* renderDevice, const crgfx::ComputeShaderDescriptor& computeShaderDescriptor)
		: IComputeShader(renderDevice, computeShaderDescriptor)
	{
		ShaderBindingLayoutResources resources;
		const CrShaderReflectionHeader& reflectionHeader = computeShaderDescriptor.m_bytecode->GetReflection();
		ShaderBindingLayout::AddResources(reflectionHeader, resources, [](crgfx::ShaderStage::T, const CrShaderReflectionResource&){});
		m_bindingLayout = crstl::unique_ptr<ShaderBindingLayout>(new ShaderBindingLayout(resources));
	}
};