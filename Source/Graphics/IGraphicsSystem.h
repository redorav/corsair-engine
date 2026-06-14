#pragma once

#include "Graphics/CrRendering.h"

#include "Graphics/CrRenderingForwardDeclarations.h"

#include "Graphics/FrameCapture/CrRenderDoc.h"
#include "Graphics/FrameCapture/CrPIX.h"

#include "crstl/intrusive_ptr.h"
#include "crstl/vector.h"

namespace CrBuiltinShaders { enum T : uint32_t; };

namespace CrBuiltinCompute { enum T : uint32_t; };

namespace crgfx
{
	struct DeviceDescriptor;

	struct GraphicsSystemDescriptor
	{
		crgfx::GraphicsApi::T graphicsApi = crgfx::GraphicsApi::Count;

		// e.g. Vulkan layers, D3D debug layer
		bool enableValidation = false;

		bool enableRenderDoc = false;

		bool enablePIX = false;

		bool enableNVAPI = true;
	};

	void InitializeGraphicsSystem(const GraphicsSystemDescriptor& graphicsSystemDescriptor);

	void CreateMainDevice(const crgfx::DeviceDescriptor& descriptor);

	const crgfx::DeviceHandle& GetDevice();

	const CrShaderBytecodeHandle& GetBuiltinShaderBytecode(CrBuiltinShaders::T builtinShader);

	const CrShaderBytecodeHandle& GetBuiltinComputeBytecode(CrBuiltinCompute::T builtinCompute);

	crgfx::GraphicsApi::T GetGraphicsApi();

	bool GetIsValidationEnabled();

	class IGraphicsSystem
	{
	public:

		IGraphicsSystem(const crgfx::GraphicsSystemDescriptor& graphicsSystemDescriptor);

		virtual ~IGraphicsSystem();

		// Only the render device calls these functions, as it knows what device we are using

		void InitializeRenderdoc();

		const crgfx::DeviceHandle& GetDevice() const;

		void CreateMainDevice(const crgfx::DeviceDescriptor& descriptor);

		bool GetIsValidationEnabled() const;

		crgfx::GraphicsApi::T GetGraphicsApi() const;

		bool GetIsNVAPIInitialized() const { return m_nvapiInitialized; }

		const CrShaderBytecodeHandle& GetBuiltinShaderBytecode(CrBuiltinShaders::T builtinShader) const;

		const CrShaderBytecodeHandle& GetBuiltinComputeBytecode(CrBuiltinCompute::T builtinCompute) const;

	public:

		crstl::vector<CrShaderBytecodeHandle> m_builtinShaderBytecodes;

		crstl::vector<CrShaderBytecodeHandle> m_builtinComputeBytecodes;

		crgfx::DeviceHandle m_mainDevice;

		crgfx::GraphicsSystemDescriptor m_descriptor;

		CrRenderDoc m_renderDoc;

		CrPIX m_pix;

		bool m_nvapiInitialized;

		virtual crgfx::IDevice* CreateDevicePS(const crgfx::DeviceDescriptor& descriptor) = 0;
	};
};

extern crstl::unique_ptr<crgfx::IGraphicsSystem> GraphicsSystem;