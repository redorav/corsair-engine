#include "Graphics/CrRendering_pch.h"

#include "IGraphicsSystem.h"

#include "IDevice.h"

// Include all the necessary platforms here

#if defined(VULKAN_API)
#include "Vulkan/GraphicsSystemVulkan.h"
#endif

#if defined(D3D12_API)
#include "D3D12/GraphicsSystemD3D12.h"
#endif

#include "Core/Streams/CrMemoryStream.h"
#include "Graphics/ICrShader.h"

#include "GeneratedShaders/BuiltinShaders.h"

#include "crstl/unique_ptr.h"

crstl::unique_ptr<crgfx::IGraphicsSystem> GraphicsSystem = nullptr;

namespace crgfx
{
	IGraphicsSystem::IGraphicsSystem(const crgfx::GraphicsSystemDescriptor& graphicsSystemDescriptor)
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

	IGraphicsSystem::~IGraphicsSystem()
	{

	}

	void crgfx::InitializeGraphicsSystem(const crgfx::GraphicsSystemDescriptor& graphicsSystemDescriptor)
	{
		IGraphicsSystem* renderSystem = nullptr;

		// Treat this like a factory (on PC) through the API. That way the rest of the code
		// doesn't need to know about platform-specific code, only the render device.
#if defined(VULKAN_API)
		if (graphicsSystemDescriptor.graphicsApi == crgfx::GraphicsApi::Vulkan)
		{
			renderSystem = new GraphicsSystemVulkan(graphicsSystemDescriptor);
		}
#endif

#if defined(D3D12_API)
		if (graphicsSystemDescriptor.graphicsApi == crgfx::GraphicsApi::D3D12)
		{
			renderSystem = new GraphicsSystemD3D12(graphicsSystemDescriptor);
		}
#endif

		GraphicsSystem = crstl::unique_ptr<IGraphicsSystem>(renderSystem);
	}

	void crgfx::CreateMainDevice(const crgfx::DeviceDescriptor& descriptor)
	{
		GraphicsSystem->CreateMainDevice(descriptor);
	}

	const crgfx::DeviceHandle& crgfx::GetDevice()
	{
		return GraphicsSystem->GetDevice();
	}

	const CrShaderBytecodeHandle& crgfx::GetBuiltinShaderBytecode(CrBuiltinShaders::T builtinShader)
	{
		return GraphicsSystem->GetBuiltinShaderBytecode(builtinShader);
	}

	const CrShaderBytecodeHandle& crgfx::GetBuiltinComputeBytecode(CrBuiltinCompute::T builtinCompute)
	{
		return GraphicsSystem->GetBuiltinComputeBytecode(builtinCompute);
	}

	crgfx::GraphicsApi::T crgfx::GetGraphicsApi()
	{
		return GraphicsSystem->GetGraphicsApi();
	}

	bool crgfx::GetIsValidationEnabled()
	{
		return GraphicsSystem->GetIsValidationEnabled();
	}

	void IGraphicsSystem::InitializeRenderdoc()
	{
		m_renderDoc.Initialize(m_descriptor);
	}

	const crgfx::DeviceHandle& IGraphicsSystem::GetDevice() const
	{
		return m_mainDevice;
	}

	void IGraphicsSystem::CreateMainDevice(const crgfx::DeviceDescriptor& descriptor)
	{
		m_mainDevice = crgfx::DeviceHandle(CreateDevicePS(descriptor));

		m_mainDevice->Initialize();
	}

	bool IGraphicsSystem::GetIsValidationEnabled() const
	{
		return m_descriptor.enableValidation;
	}

	crgfx::GraphicsApi::T IGraphicsSystem::GetGraphicsApi() const
	{
		return m_descriptor.graphicsApi;
	}

	const CrShaderBytecodeHandle& IGraphicsSystem::GetBuiltinShaderBytecode(CrBuiltinShaders::T builtinShader) const
	{
		return m_builtinShaderBytecodes[builtinShader];
	}

	const CrShaderBytecodeHandle& IGraphicsSystem::GetBuiltinComputeBytecode(CrBuiltinCompute::T builtinCompute) const
	{
		return m_builtinComputeBytecodes[builtinCompute];
	}
};