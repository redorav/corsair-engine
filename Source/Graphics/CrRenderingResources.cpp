#include "Graphics/CrRendering_pch.h"

#include "CrRenderingResources.h"

#include "Graphics/IGraphicsSystem.h"
#include "Graphics/ISampler.h"
#include "Graphics/ITexture.h"
#include "Graphics/IDevice.h"

CrRenderingResources* RenderingResources;

void CrRenderingResources::Initialize()
{
	CrAssert(RenderingResources == nullptr);
	RenderingResources = new CrRenderingResources();
}

void CrRenderingResources::Deinitialize()
{
	CrAssert(RenderingResources != nullptr);
	delete RenderingResources;
}

CrRenderingResources::CrRenderingResources()
{
	//------------------------
	// Create default samplers
	//------------------------

	{
		crgfx::SamplerDescriptor descriptor;
		descriptor.addressModeU = crgfx::AddressMode::ClampToEdge;
		descriptor.addressModeV = crgfx::AddressMode::ClampToEdge;
		descriptor.addressModeW = crgfx::AddressMode::ClampToEdge;
		descriptor.name = "Linear Clamp Sampler";
		AllLinearClampSampler = crgfx::GetDevice()->CreateSampler(descriptor);
	}

	{
		crgfx::SamplerDescriptor descriptor;
		descriptor.addressModeU = crgfx::AddressMode::Wrap;
		descriptor.addressModeV = crgfx::AddressMode::Wrap;
		descriptor.addressModeW = crgfx::AddressMode::Wrap;
		descriptor.name = "Linear Wrap Sampler";
		AllLinearWrapSampler = crgfx::GetDevice()->CreateSampler(descriptor);
	}

	{
		crgfx::SamplerDescriptor descriptor;
		descriptor.addressModeU = crgfx::AddressMode::ClampToEdge;
		descriptor.addressModeV = crgfx::AddressMode::ClampToEdge;
		descriptor.addressModeW = crgfx::AddressMode::ClampToEdge;
		descriptor.name = "Point Clamp Sampler";
		AllPointClampSampler = crgfx::GetDevice()->CreateSampler(descriptor);
	}

	{
		crgfx::SamplerDescriptor descriptor;
		descriptor.addressModeU = crgfx::AddressMode::Wrap;
		descriptor.addressModeV = crgfx::AddressMode::Wrap;
		descriptor.addressModeW = crgfx::AddressMode::Wrap;
		descriptor.name = "Point Wrap Sampler";
		AllPointWrapSampler = crgfx::GetDevice()->CreateSampler(descriptor);
	}

	//------------------------
	// Create default textures
	//------------------------

	{
		uint8_t whiteTextureInitialData[4 * 4 * 4];
		memset(whiteTextureInitialData, 0xff, sizeof(whiteTextureInitialData));

		crgfx::TextureDescriptor whiteTextureDescriptor;
		whiteTextureDescriptor.width = 4;
		whiteTextureDescriptor.height = 4;
		whiteTextureDescriptor.initialData = whiteTextureInitialData;
		whiteTextureDescriptor.initialDataSize = sizeof(whiteTextureInitialData);
		whiteTextureDescriptor.name = "White Small Texture";
		WhiteSmallTexture = crgfx::GetDevice()->CreateTexture(whiteTextureDescriptor);
	}

	{
		uint8_t blackTextureInitialData[4 * 4 * 4];
		memset(blackTextureInitialData, 0, sizeof(blackTextureInitialData));

		crgfx::TextureDescriptor blackTextureDescriptor;
		blackTextureDescriptor.width = 4;
		blackTextureDescriptor.height = 4;
		blackTextureDescriptor.initialData = blackTextureInitialData;
		blackTextureDescriptor.initialDataSize = sizeof(blackTextureInitialData);
		blackTextureDescriptor.name = "Black Small Texture";
		BlackSmallTexture = crgfx::GetDevice()->CreateTexture(blackTextureDescriptor);
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

		crgfx::TextureDescriptor defaultNormalTextureDescriptor;
		defaultNormalTextureDescriptor.width = 4;
		defaultNormalTextureDescriptor.height = 4;
		defaultNormalTextureDescriptor.initialData = (const uint8_t*)normalMapInitialData;
		defaultNormalTextureDescriptor.initialDataSize = sizeof(normalMapInitialData);
		defaultNormalTextureDescriptor.name = "Normal Map Small Texture";
		NormalsSmallTexture = crgfx::GetDevice()->CreateTexture(defaultNormalTextureDescriptor);
	}
}
