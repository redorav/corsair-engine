#pragma once

#include "Rendering/CrRendering.h"

#include "Core/CrPlatform.h"

class CrShaderCompilerCommandLine
{
public:

	static const char* GetGraphicsApi(cr3d::GraphicsApi::T graphicsApi)
	{
		switch (graphicsApi)
		{
			case cr3d::GraphicsApi::Vulkan: return "vulkan";
			case cr3d::GraphicsApi::D3D12: return "d3d12";
			default: return "invalid";
		}
	}

	static const char* GetPlatform(cr::Platform::T platform)
	{
		switch (platform)
		{
			case cr::Platform::Windows: return "windows";
			default: return "invalid";
		}
	}

	static const char* GetShaderStage(cr3d::ShaderStage::T stage)
	{
		switch (stage)
		{
			case cr3d::ShaderStage::Vertex: return "vertex";
			case cr3d::ShaderStage::Pixel: return "pixel";
			default: return "invalid";
		}
	}
};