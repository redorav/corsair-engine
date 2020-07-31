#pragma once

#include "CrGPUBuffer.h"
#include <d3d12.h>

#include "Core/Logging/ICrDebug.h"

class CrRenderDeviceD3D12;

class CrHardwareGPUBufferD3D12 final : public ICrHardwareGPUBuffer
{
public:

	CrHardwareGPUBufferD3D12(CrRenderDeviceD3D12* renderDevice, const CrGPUBufferDescriptor& params);

	virtual void* LockPS() override;

	virtual void UnlockPS() override;

private:

};