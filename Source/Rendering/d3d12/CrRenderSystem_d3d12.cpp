#include "CrRendering_pch.h"

#include "CrRenderSystem_d3d12.h"

#include "CrRenderDevice_d3d12.h"

#include "Core/CrMacros.h"

#include "Core/Logging/ICrDebug.h"

CrRenderSystemD3D12::CrRenderSystemD3D12(const CrRenderSystemDescriptor& renderSystemDescriptor)
{
	UINT createFactoryFlags = 0;

	if (renderSystemDescriptor.enableValidation)
	{
		createFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}

	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_d3d12DebugController))))
	{
		m_d3d12DebugController->EnableDebugLayer();
	}
	
	HRESULT hResult = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory));
	CrAssertMsg(SUCCEEDED(hResult), "Failed to create DXGIFactory");
}

ICrRenderDevice* CrRenderSystemD3D12::CreateRenderDevicePS() const
{
	return new CrRenderDeviceD3D12(this);
}
