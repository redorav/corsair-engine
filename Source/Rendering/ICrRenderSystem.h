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
	
	// On PC we can programmatically load multiple debuggers. They cannot
	// all be simultaneously loaded or loaded for all platforms
	bool enableRenderDoc = false;

	bool enablePIX = false;
};

namespace CrBuiltinShaders { enum T : uint32_t; };

struct CrRenderDeviceDescriptor;

class ICrRenderSystem
{
public:

	ICrRenderSystem(const CrRenderSystemDescriptor& renderSystemDescriptor);

	virtual ~ICrRenderSystem();

	static void Initialize(const CrRenderSystemDescriptor& renderSystemDescriptor);

	static const CrRenderDeviceHandle& GetRenderDevice();

	static void CreateRenderDevice(const CrRenderDeviceDescriptor& descriptor);

	static bool GetIsValidationEnabled();

	static bool GetIsRenderDocEnabled();

	static cr3d::GraphicsApi::T GetGraphicsApi();

	static const CrShaderBytecodeHandle& GetBuiltinShaderBytecode(CrBuiltinShaders::T builtinShader);

protected:

	CrVector<CrShaderBytecodeHandle> m_builtinShaderBytecodes;

	CrRenderDeviceHandle m_mainDevice;

	CrRenderSystemDescriptor m_descriptor;

	CrRenderDoc m_renderDoc;

	CrPIX m_pix;

	virtual ICrRenderDevice* CreateRenderDevicePS(const CrRenderDeviceDescriptor& descriptor) const = 0;
};

extern CrUniquePtr<ICrRenderSystem> RenderSystem;