#pragma once

#include "Rendering/ICrShader.h"
#include <d3d12.h>

class ICrRenderDevice;

class CrGraphicsShaderD3D12 final : public ICrGraphicsShader
{
public:

	CrGraphicsShaderD3D12(ICrRenderDevice* renderDevice, const CrGraphicsShaderDescriptor& graphicsShaderDescriptor);

	~CrGraphicsShaderD3D12();

private:
};

class CrComputeShaderD3D12 final : public ICrComputeShader
{
public:

	CrComputeShaderD3D12(ICrRenderDevice* renderDevice, const CrComputeShaderDescriptor& computeShaderDescriptor);

private:
};