#pragma once

#include "Rendering/ICrFramebuffer.h"
#include <d3d12.h>

class ICrRenderDevice;

class CrFramebufferD3D12 final : public ICrFramebuffer
{
public:

	~CrFramebufferD3D12();

	CrFramebufferD3D12(ICrRenderDevice* renderDevice, const CrFramebufferCreateParams& params);

private:

};