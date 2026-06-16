#pragma once

#include "Graphics/IShader.h"
#include <d3d12.h>

namespace crgfx
{
	class GraphicsShaderD3D12 final : public IGraphicsShader
	{
	public:

		GraphicsShaderD3D12(crgfx::IDevice* renderDevice, const GraphicsShaderDescriptor& graphicsShaderDescriptor);

		~GraphicsShaderD3D12();
	};

	class ComputeShaderD3D12 final : public IComputeShader
	{
	public:

		ComputeShaderD3D12(crgfx::IDevice* renderDevice, const ComputeShaderDescriptor& computeShaderDescriptor);
	};
};