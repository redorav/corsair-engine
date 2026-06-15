#pragma once

#include "Core/CrCoreForwardDeclarations.h"

#include "Graphics/CrRendering.h"
#include "Graphics/CrRenderingForwardDeclarations.h"
#include "Graphics/CrGPUDeletable.h"
#include "Graphics/CrDataFormats.h"

#include "crstl/array.h"
#include "crstl/fixed_string.h"
#include "crstl/fixed_vector.h"

// Descriptor of a view. This can be used to both create a texture and during binding. Note that this is not an API object, the ownership of
// that lies with the texture. This is to simplify the management of API resources, once a texture is destroyed, all the views it used to
// manage are destroyed too, preventing issues like binding a view whose resource no longer exists

namespace crgfx
{
	namespace TextureViewType
	{
		enum T : uint32_t
		{
			Read, // Shader Resource View
			Write, // Unordered Access View
		};
	};

	struct TextureView
	{
		explicit TextureView
		(
			uint8_t mipmapStart = 0,
			uint8_t mipmapCount = 0xff, // 0xff means all mips are visible
			uint16_t sliceStart = 0,
			uint16_t sliceCount = 0xffff, // 0xffff means all slices are visible
			crgfx::DataFormat::T format = crgfx::DataFormat::Invalid,
			crgfx::TextureViewType::T type = crgfx::TextureViewType::Read,
			crgfx::TexturePlane::T plane = crgfx::TexturePlane::Plane0
		)
			: mipmapStart(mipmapStart)
			, mipmapCount(mipmapCount)
			, sliceStart(sliceStart)
			, sliceCount(sliceCount)
			, format(format)
			, type(type)
			, plane(plane)
		{
		}

		explicit TextureView(crgfx::TexturePlane::T plane) : TextureView()
		{
			this->plane = plane;
		}

		bool operator == (const TextureView& other) const
		{
			return key == other.key;
		}

		union
		{
			struct
			{
				uint32_t mipmapStart : 8;
				uint32_t mipmapCount : 8;
				uint32_t sliceStart : 16;
				uint32_t sliceCount : 16;
				crgfx::DataFormat::T format : 8;
				crgfx::TextureViewType::T type : 6;
				crgfx::TexturePlane::T plane : 2;
			};

			uint64_t key;
		};
	};

	static_assert(sizeof(TextureView) == sizeof(uint64_t));

	struct TextureDescriptor
	{
		TextureDescriptor()
			: width(1)
			, height(1)
			, depth(1)
			, mipmapCount(1)
			, arraySize(1)
			, format(crgfx::DataFormat::RGBA8_Unorm)
			, sampleCount(crgfx::SampleCount::S1)
			, type(crgfx::TextureType::Tex2D)
			, usage(crgfx::TextureUsage::Default)
			, initialData(nullptr)
			, initialDataSize(0)
			, extraData(0)
			, extraDataPtr(nullptr)
			, name(nullptr)
		{}

		uint32_t width;
		uint32_t height;
		uint32_t depth; // Valid for volume textures
		uint32_t mipmapCount;
		uint32_t arraySize; // Valid for texture or cubemap arrays
		crgfx::DataFormat::T format;
		crgfx::SampleCount sampleCount;
		crgfx::TextureType type;
		crgfx::TextureUsageFlags usage;
		crstl::fixed_vector<crgfx::TextureView, crgfx::MaxCustomTextureViews> customViews;

		// Defaults to zero
		union
		{
			float colorClear[4] = {};
			struct
			{
				float depthClear;
				uint8_t stencilClear;
			};
		};

		const uint8_t* initialData; // TODO do better
		uint64_t initialDataSize; // TODO delete from here

		// This extra data exists to be able to pass platform-specific information
		// such as swapchain information, etc.
		uint32_t extraData;
		void* extraDataPtr;

		const char* name;
	};

	class ITexture : public CrGPUAutoDeletable
	{
	public:

		virtual ~ITexture();

		bool IsRenderTarget() const { return (m_usage & crgfx::TextureUsage::RenderTarget) != 0; }

		bool IsUnorderedAccess() const { return (m_usage & crgfx::TextureUsage::UnorderedAccess) != 0; }

		bool IsSwapchain() const { return (m_usage & crgfx::TextureUsage::SwapChain) != 0; }

		bool IsDepthStencil() const { return (m_usage & crgfx::TextureUsage::DepthStencil) != 0; }

		bool Is1DTexture() const { return m_type == crgfx::TextureType::Tex1D; }

		bool Is2DTexture() const { return m_type == crgfx::TextureType::Tex2D; }

		bool IsVolumeTexture() const { return m_type == crgfx::TextureType::Volume; }

		bool IsCubemap() const { return m_type == crgfx::TextureType::Cubemap; }

		crgfx::DataFormat::T GetFormat() const { return m_format; }

		crgfx::SampleCount GetSampleCount() const { return m_sampleCount; }

		uint32_t GetWidth() const { return m_width; }

		uint32_t GetHeight() const { return m_height; }

		uint32_t GetDepth() const { return m_depth; }

		uint32_t GetMipmapCount() const { return m_mipmapCount; }

		uint32_t GetSliceCount() const { return m_arraySize; }

		crgfx::TextureState GetDefaultState() const { return m_defaultState; }

		static crgfx::MipmapLayout GetDDSMipSliceLayout(crgfx::DataFormat::T format, uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipmaps, bool isVolume, uint32_t mip, uint32_t slice);

		crgfx::MipmapLayout GetDDSMipSliceLayout(uint32_t mip, uint32_t slice) const;

		crgfx::MipmapLayout GetHardwareMipSliceLayout(uint32_t mip, uint32_t slice) const;

		uint32_t GetUsedGPUMemory() const { return m_usedGPUMemoryBytes; }

		void CopyIntoTextureMemory(uint8_t* destinationData, const uint8_t* sourceData, uint32_t mip, uint32_t slice);

		void CopyIntoTextureMemory
		(
			uint8_t* destinationData, const uint8_t* sourceData,
			uint32_t startMip, uint32_t mipCount, uint32_t startSlice, uint32_t sliceCount
		);

#if !defined(CR_CONFIG_FINAL)

		const char* GetDebugName() const
		{
			return m_debugName.c_str();
		}

#endif

	protected:

		// We don't allow the constructor externally
		ITexture(crgfx::IDevice* renderDevice, const crgfx::TextureDescriptor& descriptor);

		crgfx::DataFormat::T m_format;

		crgfx::SampleCount m_sampleCount;

		crgfx::TextureType m_type;

		uint32_t m_width;

		uint32_t m_height;

		uint32_t m_depth;

		uint32_t m_arraySize;

		uint32_t m_mipmapCount;

		// Must be populated by the platform-specific implementation. This is the texture size as reported
		// by the hardware and takes into account padding, etc Used for reporting
		uint32_t m_usedGPUMemoryBytes;

		crgfx::TextureState m_defaultState;

		crgfx::TextureUsageFlags m_usage;

		// Mipmap layout that is platform-dependent
		crstl::array<crgfx::MipmapLayout, crgfx::MaxMipmaps> m_hardwareMipmapLayouts;

		// Distance between two consecutive slices
		uint32_t m_slicePitchBytes;

#if !defined(CR_CONFIG_FINAL)

		crstl::fixed_string128 m_debugName;

#endif
	};
}
