#include "CrRendering_pch.h"

#include "ICrRenderSystem.h"

// Include all the necessary platforms here

#if defined(VULKAN_API)
#include "vulkan/CrRenderSystem_vk.h"
#endif

#if defined(D3D12_API)
#include "d3d12/CrRenderSystem_d3d12.h"
#endif

#include "Core/SmartPointers/CrSharedPtr.h"

static ICrRenderSystem* RenderSystem = nullptr;

ICrRenderSystem* ICrRenderSystem::Get()
{
	return RenderSystem;
}

void ICrRenderSystem::Initialize(const CrRenderSystemDescriptor& renderSystemDescriptor)
{
	// Treat this like a factory (on PC) through the API. That way the rest of the code
	// doesn't need to know about platform-specific code, only the render device.
#if defined(VULKAN_API)
	if (renderSystemDescriptor.graphicsApi == cr3d::GraphicsApi::Vulkan)
	{
		RenderSystem = new CrRenderSystemVulkan(renderSystemDescriptor);
	}
#endif

#if defined(D3D12_API)
	if (renderSystemDescriptor.graphicsApi == cr3d::GraphicsApi::D3D12)
	{
		RenderSystem = new CrRenderSystemD3D12(renderSystemDescriptor);
	}
#endif
}

CrRenderDeviceSharedHandle ICrRenderSystem::GetRenderDevice()
{
	return RenderSystem->m_mainDevice;
}

void ICrRenderSystem::CreateRenderDevice()
{
	RenderSystem->m_mainDevice = CrRenderDeviceSharedHandle(RenderSystem->CreateRenderDevicePS());
}
