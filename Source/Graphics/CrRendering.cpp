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

cr3d::GraphicsVendor::T cr3d::GraphicsVendor::FromString(const char* graphicsVendorString)
{
	if (strcmp(graphicsVendorString, "nvidia") == 0)
	{
		return GraphicsVendor::NVIDIA;
	}
	else if (strcmp(graphicsVendorString, "amd") == 0)
	{
		return GraphicsVendor::AMD;
	}
	else if (strcmp(graphicsVendorString, "intel") == 0)
	{
		return GraphicsVendor::Intel;
	}
	else
	{
		return GraphicsVendor::Unknown;
	}
}

cr3d::ShaderStage::T cr3d::ShaderStage::FromString(const char* shaderStageString)
{
	if (strcmp(shaderStageString, "vertex") == 0)
	{
		return ShaderStage::Vertex;
	}
	else if (strcmp(shaderStageString, "pixel") == 0)
	{
		return ShaderStage::Pixel;
	}
	else if (strcmp(shaderStageString, "hull") == 0)
	{
		return ShaderStage::Hull;
	}
	else if (strcmp(shaderStageString, "domain") == 0)
	{
		return ShaderStage::Domain;
	}
	else if (strcmp(shaderStageString, "geometry") == 0)
	{
		return ShaderStage::Geometry;
	}
	else if (strcmp(shaderStageString, "compute") == 0)
	{
		return ShaderStage::Compute;
	}
	else if (strcmp(shaderStageString, "rootsignature") == 0)
	{
		return ShaderStage::RootSignature;
	}
	else
	{
		return ShaderStage::Count;
	}
}
