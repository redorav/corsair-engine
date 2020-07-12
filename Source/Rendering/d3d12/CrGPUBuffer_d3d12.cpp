#include "CrRendering_pch.h"

#include "CrGPUBuffer_d3d12.h"
#include "CrRenderDevice_d3d12.h"
#include "CrD3D12.h"

#include "Core/Logging/ICrDebug.h"

CrHardwareGPUBufferD3D12::CrHardwareGPUBufferD3D12(CrRenderDeviceD3D12* renderDevice, const CrGPUBufferCreateParams& params) : ICrHardwareGPUBuffer(params)
{
	
}

void* CrHardwareGPUBufferD3D12::LockPS()
{
	void* data = nullptr;
	return data;
}

void CrHardwareGPUBufferD3D12::UnlockPS()
{

}