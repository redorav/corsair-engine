#pragma once

#include "Rendering/ICrRenderSystem.h"

#include "Core/String/CrString.h"

#include "Core/Containers/CrVector.h"

#include "Core/Containers/CrSet.h"

class CrRenderSystemD3D12 final : public ICrRenderSystem
{
public:

	CrRenderSystemD3D12(const CrRenderSystemDescriptor& renderSystemDescriptor);

	virtual ICrRenderDevice* CreateRenderDevicePS() const override;

private:

};