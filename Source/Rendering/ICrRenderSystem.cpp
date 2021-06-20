#include "CrRendering_pch.h"

#include "ICrRenderSystem.h"

// Include all the necessary platforms here

#if defined(VULKAN_API)
#include "vulkan/CrRenderSystem_vk.h"
#endif

#if defined(D3D12_API)
#include "d3d12/CrRenderSystem_d3d12.h"
#endif

#include "Core/SmartPointers/CrUniquePtr.h"
#include "Core/SmartPointers/CrSharedPtr.h"

static CrUniquePtr<ICrRenderSystem> RenderSystem = nullptr;

ICrRenderSystem::ICrRenderSystem(const CrRenderSystemDescriptor& renderSystemDescriptor)
{
	m_graphicsApi = renderSystemDescriptor.graphicsApi;
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

CrRenderDeviceSharedHandle ICrRenderSystem::GetRenderDevice()
{
	return RenderSystem->m_mainDevice;
}

void ICrRenderSystem::CreateRenderDevice()
{
	// We need a custom deleter for the render device because we cannot call functions that use virtual methods during the destruction
	// process. It's an unfortunate consequence of the virtual function abstraction
	RenderSystem->m_mainDevice = CrRenderDeviceSharedHandle(RenderSystem->CreateRenderDevicePS(), [](ICrRenderDevice* renderDevice)
	{
		renderDevice->FinalizeDeletionQueue();
		delete renderDevice;
	});
	RenderSystem->m_mainDevice->InitializeDeletionQueue();
}

cr3d::GraphicsApi::T ICrRenderSystem::GetGraphicsApi() const
{
	return m_graphicsApi;
}
