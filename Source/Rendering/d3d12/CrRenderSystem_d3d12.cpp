#include "CrRendering_pch.h"

#include "CrRenderSystem_d3d12.h"

#include "CrRenderDevice_d3d12.h"

#include "Core/CrMacros.h"

#include "Core/Logging/ICrDebug.h"

CrRenderSystemD3D12::CrRenderSystemD3D12(const CrRenderSystemDescriptor& renderSystemDescriptor)
{
	unused_parameter(renderSystemDescriptor);
}

ICrRenderDevice* CrRenderSystemD3D12::CreateRenderDevicePS() const
{
	return nullptr; // new CrRenderDeviceD3D12(this);
}
