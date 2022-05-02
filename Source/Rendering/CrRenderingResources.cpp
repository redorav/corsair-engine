#include "CrRendering_pch.h"

#include "CrRenderingResources.h"

#include "Rendering/ICrSampler.h"
#include "Rendering/ICrRenderDevice.h"

CrRenderingResources GlobalRenderResources;

CrRenderingResources& CrRenderingResources::Get()
{
	return GlobalRenderResources;
}

void CrRenderingResources::Initialize(ICrRenderDevice* renderDevice)
{
	{
		CrSamplerDescriptor descriptor;
		descriptor.addressModeU = cr3d::AddressMode::ClampToEdge;
		descriptor.addressModeV = cr3d::AddressMode::ClampToEdge;
		descriptor.addressModeW = cr3d::AddressMode::ClampToEdge;
		descriptor.name = "Linear Clamp Sampler";
		AllLinearClampSampler = renderDevice->CreateSampler(descriptor);
	}

	{
		CrSamplerDescriptor descriptor;
		descriptor.addressModeU = cr3d::AddressMode::Wrap;
		descriptor.addressModeV = cr3d::AddressMode::Wrap;
		descriptor.addressModeW = cr3d::AddressMode::Wrap;
		descriptor.name = "Linear Wrap Sampler";
		AllLinearWrapSampler = renderDevice->CreateSampler(descriptor);
	}

	{
		CrSamplerDescriptor descriptor;
		descriptor.addressModeU = cr3d::AddressMode::ClampToEdge;
		descriptor.addressModeV = cr3d::AddressMode::ClampToEdge;
		descriptor.addressModeW = cr3d::AddressMode::ClampToEdge;
		descriptor.name = "Point Clamp Sampler";
		AllPointClampSampler = renderDevice->CreateSampler(descriptor);
	}

	{
		CrSamplerDescriptor descriptor;
		descriptor.addressModeU = cr3d::AddressMode::Wrap;
		descriptor.addressModeV = cr3d::AddressMode::Wrap;
		descriptor.addressModeW = cr3d::AddressMode::Wrap;
		descriptor.name = "Point Wrap Sampler";
		AllPointWrapSampler = renderDevice->CreateSampler(descriptor);
	}
}

void CrRenderingResources::Deinitialize()
{
	// Call destructors of all local variables
	*this = {};
}