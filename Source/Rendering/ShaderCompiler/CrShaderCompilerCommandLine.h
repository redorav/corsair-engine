#pragma once

#include "Rendering/CrRendering.h"

#include "Core/CrPlatform.h"

class CrShaderCompilerCommandLine
{
public:

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
			case cr3d::ShaderStage::Hull: return "hull";
			case cr3d::ShaderStage::Domain: return "domain";
			case cr3d::ShaderStage::Geometry: return "geometry";
			case cr3d::ShaderStage::Compute: return "compute";
			default: return "invalid";
		}
	}
};