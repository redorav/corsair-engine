#include "Rendering/CrRendering_pch.h"

#include "CrRenderSystem_d3d12.h"

#include "CrRenderDevice_d3d12.h"

#include "Core/CrMacros.h"

#include "Core/Logging/ICrDebug.h"

CrRenderSystemD3D12::CrRenderSystemD3D12(const CrRenderSystemDescriptor& renderSystemDescriptor) : ICrRenderSystem(renderSystemDescriptor)
{
	UINT createFactoryFlags = 0;
	HRESULT hResult = S_OK;

	if (renderSystemDescriptor.enableValidation)
	{
		createFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

		hResult = D3D12GetDebugInterface(IID_PPV_ARGS(&m_d3d12DebugController));
		if (SUCCEEDED(hResult))
		{
			m_d3d12DebugController->EnableDebugLayer();

			//ID3D12Debug1* d3d12DebugController1 = nullptr;
			//hr = m_d3d12DebugController->QueryInterface(IID_GRAPHICS_PPV_ARGS(&d3d12DebugController1));
		}
	}

	// Assume DXGI 1.4
	hResult = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory4));
	CrAssertMsg(SUCCEEDED(hResult), "Failed to create DXGIFactory");

	if (m_dxgiFactory4->QueryInterface(IID_PPV_ARGS(&m_dxgiFactory6)) == S_OK)
	{

	}

	if (m_dxgiFactory4->QueryInterface(IID_PPV_ARGS(&m_dxgiFactory7)) == S_OK)
	{

	}
}

ICrRenderDevice* CrRenderSystemD3D12::CreateRenderDevicePS(const CrRenderDeviceDescriptor& descriptor) const
{
	return new CrRenderDeviceD3D12(this, descriptor);
}
