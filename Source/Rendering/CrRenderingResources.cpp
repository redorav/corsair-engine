#include "Rendering/CrRendering_pch.h"

#include "CrRenderingResources.h"

#include "Rendering/ICrSampler.h"
#include "Rendering/ICrTexture.h"
#include "Rendering/ICrRenderDevice.h"

CrRenderingResources GlobalRenderResources;

CrRenderingResources& CrRenderingResources::Get()
{
	return GlobalRenderResources;
}

void CrRenderingResources::Initialize(ICrRenderDevice* renderDevice)
{
	//------------------------
	// Create default samplers
	//------------------------

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

	//------------------------
	// Create default textures
	//------------------------

	{
		uint8_t whiteTextureInitialData[4 * 4 * 4];
		memset(whiteTextureInitialData, 0xff, sizeof(whiteTextureInitialData));

		CrTextureDescriptor whiteTextureDescriptor;
		whiteTextureDescriptor.width = 4;
		whiteTextureDescriptor.height = 4;
		whiteTextureDescriptor.initialData = whiteTextureInitialData;
		whiteTextureDescriptor.initialDataSize = sizeof(whiteTextureInitialData);
		whiteTextureDescriptor.name = "White Small Texture";
		WhiteSmallTexture = renderDevice->CreateTexture(whiteTextureDescriptor);
	}

	{
		uint8_t blackTextureInitialData[4 * 4 * 4];
		memset(blackTextureInitialData, 0, sizeof(blackTextureInitialData));

		CrTextureDescriptor blackTextureDescriptor;
		blackTextureDescriptor.width = 4;
		blackTextureDescriptor.height = 4;
		blackTextureDescriptor.initialData = blackTextureInitialData;
		blackTextureDescriptor.initialDataSize = sizeof(blackTextureInitialData);
		blackTextureDescriptor.name = "Black Small Texture";
		BlackSmallTexture = renderDevice->CreateTexture(blackTextureDescriptor);
	}

	{
		uint8_t normalMapInitialData[4 * 4 * 4];

		for (uint32_t x = 0; x < 4; ++x)
		{
			for (uint32_t y = 0; y < 4; ++y)
			{
				normalMapInitialData[y * 4 * 4 + x * 4 + 0] = 128;
				normalMapInitialData[y * 4 * 4 + x * 4 + 1] = 128;
				normalMapInitialData[y * 4 * 4 + x * 4 + 2] = 255;
				normalMapInitialData[y * 4 * 4 + x * 4 + 3] = 255;
			}
		}

		CrTextureDescriptor defaultNormalTextureDescriptor;
		defaultNormalTextureDescriptor.width = 4;
		defaultNormalTextureDescriptor.height = 4;
		defaultNormalTextureDescriptor.initialData = (const uint8_t*)normalMapInitialData;
		defaultNormalTextureDescriptor.initialDataSize = sizeof(normalMapInitialData);
		defaultNormalTextureDescriptor.name = "Normal Map Small Texture";
		NormalsSmallTexture = renderDevice->CreateTexture(defaultNormalTextureDescriptor);
	}
}

void CrRenderingResources::Deinitialize()
{
	// Call destructors of all local variables
	*this = {};
}