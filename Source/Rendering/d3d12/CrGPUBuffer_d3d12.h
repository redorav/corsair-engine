#pragma once

#include "Rendering/CrGPUBuffer.h"
#include <d3d12.h>

#include "Core/Logging/ICrDebug.h"

class CrRenderDeviceD3D12;

class CrHardwareGPUBufferD3D12 final : public ICrHardwareGPUBuffer
{
public:

	CrHardwareGPUBufferD3D12(CrRenderDeviceD3D12* renderDevice, const CrHardwareGPUBufferDescriptor& descriptor);

	ID3D12Resource* GetD3D12Resource() const { return m_d3d12Resource; }

	D3D12_RESOURCE_STATES GetDefaultResourceState() const { return m_d3d12InitialState; }

	virtual void* LockPS() override;

	virtual void UnlockPS() override;

private:

	ID3D12Resource* m_d3d12Resource;

	D3D12_RESOURCE_STATES m_d3d12InitialState;

	ID3D12Device* m_d3d12Device;
};

inline const CrHardwareGPUBufferD3D12* D3D12Cast(const ICrHardwareGPUBuffer* buffer)
{
	return static_cast<const CrHardwareGPUBufferD3D12*>(buffer);
}

inline CrHardwareGPUBufferD3D12* D3D12Cast(ICrHardwareGPUBuffer* buffer)
{
	return static_cast<CrHardwareGPUBufferD3D12*>(buffer);
}