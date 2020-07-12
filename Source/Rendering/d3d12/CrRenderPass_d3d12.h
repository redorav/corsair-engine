#pragma once

#include "ICrRenderPass.h"
#include <d3d12.h>

class ICrRenderDevice;

class CrRenderPassD3D12 final : public ICrRenderPass
{
public:

	CrRenderPassD3D12(ICrRenderDevice* renderDevice, const CrRenderPassDescriptor& renderPassDescriptor);

	~CrRenderPassD3D12();

private:

	
};