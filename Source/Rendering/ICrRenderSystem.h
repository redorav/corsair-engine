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

class ICrRenderSystem
{
public:

	static ICrRenderSystem* Get();

	static void Initialize(const CrRenderSystemDescriptor& renderSystemDescriptor);

	static CrRenderDeviceSharedHandle GetRenderDevice();

	static void CreateRenderDevice();

protected:

	CrRenderDeviceSharedHandle m_mainDevice;

	virtual ICrRenderDevice* CreateRenderDevicePS() const = 0;
};