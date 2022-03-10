#include "CrRendering_pch.h"

#include "ICrRenderSystem.h"

// Include all the necessary platforms here

#if defined(VULKAN_API)
#include "vulkan/CrRenderSystem_vk.h"
#endif

#if defined(D3D12_API)
#include "d3d12/CrRenderSystem_d3d12.h"
#endif

#include "Core/Streams/CrMemoryStream.h"
#include "Rendering/ICrShader.h"

#include "GeneratedShaders/BuiltinShaders.h"

#include "Core/SmartPointers/CrUniquePtr.h"
#include "Core/SmartPointers/CrSharedPtr.h"

static CrUniquePtr<ICrRenderSystem> RenderSystem = nullptr;

ICrRenderSystem::ICrRenderSystem(const CrRenderSystemDescriptor& renderSystemDescriptor)
{
	m_graphicsApi = renderSystemDescriptor.graphicsApi;

	// Load builtin shaders here. The render system is only instantiated once and knows
	// which platform it needs to load bytecodes for. Once all bytecodes are loaded, the
	// rest of the engine interacts with them and not the raw data that was passed in, 
	// as the metadata needs to be serialized, etc.

	m_builtinShaderBytecodes.resize(CrBuiltinShaders::Count);

	for (uint32_t i = 0; i < CrBuiltinShaders::Count; ++i)
	{
		const CrBuiltinShaderMetadata& metadata = CrBuiltinShaders::GetBuiltinShaderMetadata((CrBuiltinShaders::T)i, m_graphicsApi);

		// Builtin shaders without code are not an error. Sometimes we need shader code 
		// that is specific to an API and we leave the entry blank. This only happens
		// on multi-API platforms which is quite rare
		if (metadata.shaderCode)
		{
			CrReadMemoryStream shaderBytecodeStream(metadata.shaderCode);
			const CrShaderBytecodeSharedHandle& bytecode = CrShaderBytecodeSharedHandle(new CrShaderBytecode());
			shaderBytecodeStream << *bytecode.get();
			m_builtinShaderBytecodes[i] = bytecode;
		}
	}
}

ICrRenderSystem::~ICrRenderSystem()
{

}

ICrRenderSystem* ICrRenderSystem::Get()
{
	return RenderSystem.get();
}

void ICrRenderSystem::Initialize(const CrRenderSystemDescriptor& renderSystemDescriptor)
{
	ICrRenderSystem* renderSystem = nullptr;

	// Treat this like a factory (on PC) through the API. That way the rest of the code
	// doesn't need to know about platform-specific code, only the render device.
#if defined(VULKAN_API)
	if (renderSystemDescriptor.graphicsApi == cr3d::GraphicsApi::Vulkan)
	{
		renderSystem = new CrRenderSystemVulkan(renderSystemDescriptor);
	}
#endif

#if defined(D3D12_API)
	if (renderSystemDescriptor.graphicsApi == cr3d::GraphicsApi::D3D12)
	{
		renderSystem = new CrRenderSystemD3D12(renderSystemDescriptor);
	}
#endif

	RenderSystem = CrUniquePtr<ICrRenderSystem>(renderSystem);
}

const CrRenderDeviceSharedHandle& ICrRenderSystem::GetRenderDevice()
{
	return RenderSystem->m_mainDevice;
}

void ICrRenderSystem::CreateRenderDevice()
{
	// We need a custom deleter for the render device because we cannot call functions that use virtual methods during the destruction
	// process. It's an unfortunate consequence of the virtual function abstraction
	RenderSystem->m_mainDevice = CrRenderDeviceSharedHandle(RenderSystem->CreateRenderDevicePS(), [](ICrRenderDevice* renderDevice)
	{
		renderDevice->FinalizeDeletion();
		delete renderDevice;
	});

	RenderSystem->m_mainDevice->InitializeDeletionQueue();
}

cr3d::GraphicsApi::T ICrRenderSystem::GetGraphicsApi()
{
	return RenderSystem->m_graphicsApi;
}

const CrShaderBytecodeSharedHandle& ICrRenderSystem::GetBuiltinShaderBytecode(CrBuiltinShaders::T builtinShader)
{
	return RenderSystem->m_builtinShaderBytecodes[builtinShader];
}
