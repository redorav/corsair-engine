#pragma once

#include "Rendering/ICrShader.h"
#include <d3d12.h>

class ICrRenderDevice;

class CrShaderBindingTableD3D12 final : public ICrShaderBindingLayout
{
public:

	CrShaderBindingTableD3D12(const CrShaderBindingLayoutResources& resources) : ICrShaderBindingLayout(resources) {}
};

class CrGraphicsShaderD3D12 final : public ICrGraphicsShader
{
public:

	CrGraphicsShaderD3D12(const ICrRenderDevice* renderDevice, const CrGraphicsShaderDescriptor& graphicsShaderDescriptor);

	~CrGraphicsShaderD3D12();

private:

	ID3D12Device* m_d3d12Device;
};

class CrComputeShaderD3D12 final : public ICrComputeShader
{
public:

	CrComputeShaderD3D12(const ICrRenderDevice* renderDevice, const CrComputeShaderDescriptor& computeShaderDescriptor);

private:

	ID3D12Device* m_d3d12Device;
};