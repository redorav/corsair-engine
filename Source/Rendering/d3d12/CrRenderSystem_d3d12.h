#pragma once

#include "Rendering/ICrRenderSystem.h"

#include "Core/Containers/CrSet.h"

class CrRenderSystemD3D12 final : public ICrRenderSystem
{
public:

	CrRenderSystemD3D12(const CrRenderSystemDescriptor& renderSystemDescriptor);

	virtual ICrRenderDevice* CreateRenderDevicePS(const CrRenderDeviceDescriptor& descriptor) override;

	IDXGIFactory4* GetDXGIFactory4() const { return m_dxgiFactory4; }

	IDXGIFactory6* GetDXGIFactory6() const { return m_dxgiFactory6; }

	IDXGIFactory6* GetDXGIFactory7() const { return m_dxgiFactory7; }

	bool InitializeNVAPI();

	bool InitializePIX();

private:

	ID3D12Debug* m_d3d12DebugController;

	IDXGIFactory4* m_dxgiFactory4 = nullptr;

	IDXGIFactory6* m_dxgiFactory6 = nullptr;

	IDXGIFactory7* m_dxgiFactory7 = nullptr;

	bool m_nvapiInitialized = false;

	bool m_pixInitialized = false;
};