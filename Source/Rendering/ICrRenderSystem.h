#pragma once

#include "Rendering/CrRendering.h"

#include "Rendering/CrRenderingForwardDeclarations.h"

#include "Core/SmartPointers/CrSharedPtr.h"

#include "Rendering/ICrRenderDevice.h"

struct CrRenderSystemDescriptor 
{
	cr3d::GraphicsApi::T graphicsApi = cr3d::GraphicsApi::Vulkan;
	bool enableValidation = false; // e.g. Vulkan layers
	bool enableDebuggingTool = false; // e.g. renderdoc
};

namespace CrBuiltinShaders { enum T : uint32_t; };

class ICrRenderSystem
{
public:

	ICrRenderSystem(const CrRenderSystemDescriptor& renderSystemDescriptor);

	virtual ~ICrRenderSystem();

	static ICrRenderSystem* Get();

	static void Initialize(const CrRenderSystemDescriptor& renderSystemDescriptor);

	static const CrRenderDeviceSharedHandle& GetRenderDevice();

	static void CreateRenderDevice();

	static cr3d::GraphicsApi::T GetGraphicsApi();

	static const CrShaderBytecodeSharedHandle& GetBuiltinShaderBytecode(CrBuiltinShaders::T builtinShader);

protected:

	CrVector<CrShaderBytecodeSharedHandle> m_builtinShaderBytecodes;

	CrRenderDeviceSharedHandle m_mainDevice;

	cr3d::GraphicsApi::T m_graphicsApi = cr3d::GraphicsApi::Count;

	virtual ICrRenderDevice* CreateRenderDevicePS() const = 0;
};