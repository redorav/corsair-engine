#pragma once

#include "Rendering/ICrTexture.h"

#include "Core/Containers/CrArray.h"
#include "Core/Containers/CrVector.h"

#include <d3d12.h>

class ICrRenderDevice;

class CrTextureD3D12 final : public ICrTexture
{
public:

	CrTextureD3D12(ICrRenderDevice* renderDevice, const CrTextureCreateParams& params);

	~CrTextureD3D12();

private:


};
