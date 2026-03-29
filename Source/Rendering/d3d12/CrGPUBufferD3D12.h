#pragma once

#include "Rendering/CrGPUBuffer.h"
#include "d3d12.h"

#include "Core/Logging/ICrDebug.h"

class CrRenderDeviceD3D12;

class CrHardwareGPUBufferD3D12 final : public ICrHardwareGPUBuffer
{
public:

	CrHardwareGPUBufferD3D12(CrRenderDeviceD3D12* renderDevice, const CrHardwareGPUBufferDescriptor& descriptor);

	virtual ~CrHardwareGPUBufferD3D12() override;

	ID3D12Resource* GetD3D12Resource() const { return m_d3d12Resource; }

	D3D12_RESOURCE_STATES GetDefaultResourceState() const { return m_d3d12InitialState; }

	virtual void* LockPS() override;

	virtual void UnlockPS() override;

private:

	ID3D12Resource* m_d3d12Resource;

	D3D12_RESOURCE_STATES m_d3d12InitialState;
};