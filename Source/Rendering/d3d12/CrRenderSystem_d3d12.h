#pragma once

#include "Rendering/ICrRenderSystem.h"

#include "Core/Containers/CrSet.h"

class CrRenderSystemD3D12 final : public ICrRenderSystem
{
public:

	CrRenderSystemD3D12(const CrRenderSystemDescriptor& renderSystemDescriptor);

	virtual ICrRenderDevice* CreateRenderDevicePS() const override;

	IDXGIFactory4* GetDXGIFactory() const;

private:

	ID3D12Debug* m_d3d12DebugController;

	IDXGIFactory4* m_dxgiFactory;
};

inline IDXGIFactory4* CrRenderSystemD3D12::GetDXGIFactory() const
{
	return m_dxgiFactory;
}