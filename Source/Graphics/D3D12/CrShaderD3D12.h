#pragma once

#include "Graphics/ICrShader.h"
#include <d3d12.h>

class IDevice;

class CrGraphicsShaderD3D12 final : public ICrGraphicsShader
{
public:

	CrGraphicsShaderD3D12(crgfx::IDevice* renderDevice, const CrGraphicsShaderDescriptor& graphicsShaderDescriptor);

	~CrGraphicsShaderD3D12();

private:
};

class CrComputeShaderD3D12 final : public ICrComputeShader
{
public:

	CrComputeShaderD3D12(crgfx::IDevice* renderDevice, const CrComputeShaderDescriptor& computeShaderDescriptor);

private:
};