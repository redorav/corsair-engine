#pragma once

#include "Rendering/CrRendering.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

#include "Rendering/FrameCapture/CrRenderDoc.h"
#include "Rendering/FrameCapture/CrPIX.h"

#include "Core/SmartPointers/CrIntrusivePtr.h"
#include "Core/Containers/CrVector.h"

struct CrRenderSystemDescriptor 
{
	cr3d::GraphicsApi::T graphicsApi = cr3d::GraphicsApi::Count;

	// e.g. Vulkan layers, D3D debug layer
	bool enableValidation = false;
};

namespace CrBuiltinShaders { enum T : uint32_t; };

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

	const CrShaderBytecodeHandle& GetBuiltinShaderBytecode(CrBuiltinShaders::T builtinShader) const;

protected:

	CrVector<CrShaderBytecodeHandle> m_builtinShaderBytecodes;

	CrRenderDeviceHandle m_mainDevice;

	CrRenderSystemDescriptor m_descriptor;

	CrRenderDoc m_renderDoc;

	CrPIX m_pix;

	virtual ICrRenderDevice* CreateRenderDevicePS(const CrRenderDeviceDescriptor& descriptor) = 0;
};

extern crstl::unique_ptr<ICrRenderSystem> RenderSystem;