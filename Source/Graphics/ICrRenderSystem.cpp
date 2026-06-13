#include "Graphics/CrRendering_pch.h"

#include "ICrRenderSystem.h"

#include "ICrRenderDevice.h"

// Include all the necessary platforms here

#if defined(VULKAN_API)
#include "Vulkan/CrRenderSystemVulkan.h"
#endif

#if defined(D3D12_API)
#include "D3D12/CrRenderSystemD3D12.h"
#endif

#include "Core/Streams/CrMemoryStream.h"
#include "Graphics/ICrShader.h"

#include "GeneratedShaders/BuiltinShaders.h"

#include "crstl/unique_ptr.h"

crstl::unique_ptr<ICrRenderSystem> RenderSystem = nullptr;

ICrRenderSystem::ICrRenderSystem(const crgfx::GraphicsSystemDescriptor& graphicsSystemDescriptor)
	: m_descriptor(graphicsSystemDescriptor)
	, m_nvapiInitialized(false)
{
	// Load builtin shaders here. The render system is only instantiated once and knows which platform it needs to load bytecodes for. Once all bytecodes are loaded, the
	// rest of the engine interacts with them and not the raw data that was passed in, as the metadata needs to be serialized, etc.

	m_builtinShaderBytecodes.resize(CrBuiltinShaders::Count);

	m_builtinComputeBytecodes.resize(CrBuiltinCompute::Count);

	for (uint32_t i = 0; i < CrBuiltinShaders::Count; ++i)
	{
		const CrBuiltinShaderMetadata& metadata = CrBuiltinShaders::GetMetadata((CrBuiltinShaders::T)i, m_descriptor.graphicsApi);

		// Builtin shaders without code are not an error. Sometimes we need shader code that is specific to an API and we leave the entry blank. This only happens
		// on multi-API platforms which is quite rare
		if (metadata.shaderCode)
		{
			CrReadMemoryStream shaderBytecodeStream(metadata.shaderCode);
			const CrShaderBytecodeHandle& bytecode = CrShaderBytecodeHandle(new CrShaderBytecode());
			shaderBytecodeStream << *bytecode.get();
			m_builtinShaderBytecodes[i] = bytecode;
		}
	}

	for (uint32_t i = 0; i < CrBuiltinCompute::Count; ++i)
	{
		const CrBuiltinComputeMetadata& metadata = CrBuiltinCompute::GetMetadata((CrBuiltinCompute::T)i, m_descriptor.graphicsApi);

		if (metadata.shaderCode)
		{
			CrReadMemoryStream shaderBytecodeStream(metadata.shaderCode);
			const CrShaderBytecodeHandle& bytecode = CrShaderBytecodeHandle(new CrShaderBytecode());
			shaderBytecodeStream << *bytecode.get();
			m_builtinComputeBytecodes[i] = bytecode;
		}
	}
}

ICrRenderSystem::~ICrRenderSystem()
{

}

void crgfx::InitializeGraphicsSystem(const crgfx::GraphicsSystemDescriptor& graphicsSystemDescriptor)
{
	ICrRenderSystem* renderSystem = nullptr;

	// Treat this like a factory (on PC) through the API. That way the rest of the code
	// doesn't need to know about platform-specific code, only the render device.
#if defined(VULKAN_API)
	if (graphicsSystemDescriptor.graphicsApi == crgfx::GraphicsApi::Vulkan)
	{
		renderSystem = new CrRenderSystemVulkan(graphicsSystemDescriptor);
	}
#endif

#if defined(D3D12_API)
	if (graphicsSystemDescriptor.graphicsApi == crgfx::GraphicsApi::D3D12)
	{
		renderSystem = new CrRenderSystemD3D12(graphicsSystemDescriptor);
	}
#endif

	RenderSystem = crstl::unique_ptr<ICrRenderSystem>(renderSystem);
}

void crgfx::CreateMainDevice(const crgfx::DeviceDescriptor& descriptor)
{
	RenderSystem->CreateMainDevice(descriptor);
}

const crgfx::CrRenderDeviceHandle& crgfx::GetRenderDevice()
{
	return RenderSystem->GetRenderDevice();
}

const CrShaderBytecodeHandle& crgfx::GetBuiltinShaderBytecode(CrBuiltinShaders::T builtinShader)
{
	return RenderSystem->GetBuiltinShaderBytecode(builtinShader);
}

const CrShaderBytecodeHandle& crgfx::GetBuiltinComputeBytecode(CrBuiltinCompute::T builtinCompute)
{
	return RenderSystem->GetBuiltinComputeBytecode(builtinCompute);
}

crgfx::GraphicsApi::T crgfx::GetGraphicsApi()
{
	return RenderSystem->GetGraphicsApi();
}

bool crgfx::GetIsValidationEnabled()
{
	return RenderSystem->GetIsValidationEnabled();
}

void ICrRenderSystem::InitializeRenderdoc()
{
	m_renderDoc.Initialize(m_descriptor);
}

const crgfx::CrRenderDeviceHandle& ICrRenderSystem::GetRenderDevice() const
{
	return m_mainDevice;
}

void ICrRenderSystem::CreateMainDevice(const crgfx::DeviceDescriptor& descriptor)
{
	m_mainDevice = crgfx::CrRenderDeviceHandle(CreateRenderDevicePS(descriptor));

	m_mainDevice->Initialize();
}

bool ICrRenderSystem::GetIsValidationEnabled() const
{
	return m_descriptor.enableValidation;
}

crgfx::GraphicsApi::T ICrRenderSystem::GetGraphicsApi() const
{
	return m_descriptor.graphicsApi;
}

const CrShaderBytecodeHandle& ICrRenderSystem::GetBuiltinShaderBytecode(CrBuiltinShaders::T builtinShader) const
{
	return m_builtinShaderBytecodes[builtinShader];
}

const CrShaderBytecodeHandle& ICrRenderSystem::GetBuiltinComputeBytecode(CrBuiltinCompute::T builtinCompute) const
{
	return m_builtinComputeBytecodes[builtinCompute];
}
