#include "Graphics/CrRendering_pch.h"
#include "CrRendering.h"

#include <cstring>

crgfx::GraphicsApi::T crgfx::GraphicsApi::FromString(const char* graphicsApiString)
{
	if (strcmp(graphicsApiString, "vulkan") == 0)
	{
		return crgfx::GraphicsApi::Vulkan;
	}
	else if (strcmp(graphicsApiString, "d3d12") == 0)
	{
		return crgfx::GraphicsApi::D3D12;
	}
	else
	{
		return crgfx::GraphicsApi::Count;
	}
}

crgfx::GraphicsVendor::T crgfx::GraphicsVendor::FromString(const char* graphicsVendorString)
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

crgfx::ShaderStage::T crgfx::ShaderStage::FromString(const char* shaderStageString)
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
