#pragma once

#include "Graphics/CrRendering.h"

#include "Graphics/CrRenderingForwardDeclarations.h"

#include "Graphics/FrameCapture/CrRenderDoc.h"
#include "Graphics/FrameCapture/CrPIX.h"

#include "crstl/intrusive_ptr.h"
#include "crstl/vector.h"

struct CrRenderSystemDescriptor 
{
	cr3d::GraphicsApi::T graphicsApi = cr3d::GraphicsApi::Count;

	// e.g. Vulkan layers, D3D debug layer
	bool enableValidation = false;

	bool enableRenderDoc = false;

	bool enablePIX = false;

	bool enableNVAPI = true;
};

namespace CrBuiltinShaders { enum T : uint32_t; };

namespace CrBuiltinCompute { enum T : uint32_t; };

struct CrRenderDeviceDescriptor;

class ICrRenderSystem
{
public:

	ICrRenderSystem(const CrRenderSystemDescriptor& renderSystemDescriptor);

	virtual ~ICrRenderSystem();

	static void Initialize(const CrRenderSystemDescriptor& renderSystemDescriptor);

	// Only the render device calls these functions, as it knows what device we are using

	void InitializeRenderdoc();

	const CrRenderDeviceHandle& GetRenderDevice() const;

	void CreateRenderDevice(const CrRenderDeviceDescriptor& descriptor);

	bool GetIsValidationEnabled() const;

	cr3d::GraphicsApi::T GetGraphicsApi() const;

	bool GetIsNVAPIInitialized() const { return m_nvapiInitialized; }

	const CrShaderBytecodeHandle& GetBuiltinShaderBytecode(CrBuiltinShaders::T builtinShader) const;

	const CrShaderBytecodeHandle& GetBuiltinComputeBytecode(CrBuiltinCompute::T builtinCompute) const;

protected:

	crstl::vector<CrShaderBytecodeHandle> m_builtinShaderBytecodes;

	crstl::vector<CrShaderBytecodeHandle> m_builtinComputeBytecodes;

	CrRenderDeviceHandle m_mainDevice;

	CrRenderSystemDescriptor m_descriptor;

	CrRenderDoc m_renderDoc;

	CrPIX m_pix;

	bool m_nvapiInitialized;

	virtual ICrRenderDevice* CreateRenderDevicePS(const CrRenderDeviceDescriptor& descriptor) = 0;
};

extern crstl::unique_ptr<ICrRenderSystem> RenderSystem;