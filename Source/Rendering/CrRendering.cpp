#include "Rendering/CrRendering_pch.h"
#include "CrRendering.h"

#include <cstring>

cr3d::GraphicsApi::T cr3d::GraphicsApi::FromString(const char* graphicsApiString)
{
	if (strcmp(graphicsApiString, "vulkan") == 0)
	{
		return cr3d::GraphicsApi::Vulkan;
	}
	else if (strcmp(graphicsApiString, "d3d12") == 0)
	{
		return cr3d::GraphicsApi::D3D12;
	}
	else
	{
		return cr3d::GraphicsApi::Count;
	}
}

cr3d::GraphicsVendor::T cr3d::GraphicsVendor::FromString(const char* s)
{
	if (strcmp(s, "nvidia") == 0)
	{
		return GraphicsVendor::NVIDIA;
	}
	else if (strcmp(s, "amd") == 0)
	{
		return GraphicsVendor::AMD;
	}
	else if (strcmp(s, "intel") == 0)
	{
		return GraphicsVendor::Intel;
	}
	else
	{
		return GraphicsVendor::Unknown;
	}
}
