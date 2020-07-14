#pragma once

#include "CrGPUBuffer.h"
#include <d3d12.h>

#include "Core/Logging/ICrDebug.h"

class CrRenderDeviceD3D12;

class CrHardwareGPUBufferD3D12 final : public ICrHardwareGPUBuffer
{
public:

	CrHardwareGPUBufferD3D12(CrRenderDeviceD3D12* renderDevice, const CrGPUBufferCreateParams& params);

	virtual void* LockPS() final override;

	virtual void UnlockPS() final override;

private:

};