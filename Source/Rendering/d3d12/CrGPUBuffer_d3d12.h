#pragma once

#include "Rendering/CrGPUBuffer.h"
#include <d3d12.h>

#include "Core/Logging/ICrDebug.h"

class CrHardwareGPUBufferD3D12 final : public ICrHardwareGPUBuffer
{
public:

	CrHardwareGPUBufferD3D12(ICrRenderDevice* renderDevice, const CrHardwareGPUBufferDescriptor& params);

	virtual void* LockPS() override;

	virtual void UnlockPS() override;

private:

};