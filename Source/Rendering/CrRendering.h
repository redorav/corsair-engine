#pragma once

#include "Math/CrMath.h"

namespace cr3d
{
	static constexpr uint32_t MaxRenderTargets = 8;

	namespace DataFormat
	{
		enum T : uint32_t
		{
			//-------------
			// Uncompressed
			//-------------

			// 8-bit formats
			R8_Unorm,
			R8_Snorm,
			R8_Uint,
			R8_Sint,

			RG8_Unorm,
			RG8_Snorm,
			RG8_Uint,
			RG8_Sint,

			RGBA8_Unorm,
			RGBA8_Snorm,
			RGBA8_Uint,
			RGBA8_Sint,
			RGBA8_SRGB,

			BGRA8_Unorm,
			BGRA8_SRGB,

			// 16-bit integer formats
			R16_Unorm,
			R16_Snorm,
			R16_Uint,
			R16_Sint,

			RG16_Unorm,
			RG16_Snorm,
			RG16_Uint,
			RG16_Sint,

			RGBA16_Unorm,
			RGBA16_Snorm,
			RGBA16_Uint,
			RGBA16_Sint,

			// 16-bit float formats
			R16_Float,
			RG16_Float,
			RGBA16_Float,

			// 32-bit integer formats
			R32_Uint,
			R32_Sint,
			RG32_Uint,
			RG32_Sint,
			RGB32_Uint,
			RGB32_Sint,
			RGBA32_Uint,
			RGBA32_Sint,

			// 32-bit float formats
			R32_Float,
			RG32_Float,
			RGB32_Float,
			RGBA32_Float,

			// Packed formats
			RGB10A2_Unorm,
			RGB10A2_Uint,
			B5G6R5_Unorm,
			B5G5R5A1_Unorm,
			BGRA4_Unorm,

			RG11B10_Float,
			RGB9E5_Float,

			//-----------
			// Compressed
			//-----------

			// Block Compression
			BC1_RGB_Unorm,  // DXT1
			BC1_RGB_SRGB,
			BC1_RGBA_Unorm, // DXT1A
			BC1_RGBA_SRGB,

			BC2_Unorm,      // DXT2/DXT3
			BC2_SRGB,

			BC3_Unorm,      // DXT4/DXT5
			BC3_SRGB,

			BC4_Unorm,      // 1 8-bit component
			BC4_Snorm,

			BC5_Unorm,      // 2 8-bit components
			BC5_Snorm,

			BC6H_UFloat,    // HDR using half
			BC6H_SFloat,

			BC7_Unorm,      // 3 8-bit components and optional alpha
			BC7_SRGB,

			// ETC2/EAC
			ETC2_RGB8_Unorm,
			ETC2_RGB8_SRGB,

			ETC2_RGB8A1_Unorm,
			ETC2_RGB8A1_SRGB,

			ETC2_RGBA8_Unorm,
			ETC2_RGBA8_SRGB,

			EAC_R11_Unorm,
			EAC_R11_Snorm,
			EAC_R11G11_Unorm,
			EAC_R11G11_Snorm,

			// ASTC
			ASTC_4x4_Unorm,
			ASTC_4x4_SRGB,
			ASTC_5x4_Unorm,
			ASTC_5x4_SRGB,
			ASTC_5x5_Unorm,
			ASTC_5x5_SRGB,
			ASTC_6x5_Unorm,
			ASTC_6x5_SRGB,
			ASTC_6x6_Unorm,
			ASTC_6x6_SRGB,
			ASTC_8x5_Unorm,
			ASTC_8x5_SRGB,
			ASTC_8x6_Unorm,
			ASTC_8x6_SRGB,
			ASTC_8x8_Unorm,
			ASTC_8x8_SRGB,
			ASTC_10x5_Unorm,
			ASTC_10x5_SRGB,
			ASTC_10x6_Unorm,
			ASTC_10x6_SRGB,
			ASTC_10x8_Unorm,
			ASTC_10x8_SRGB,
			ASTC_10x10_Unorm,
			ASTC_10x10_SRGB,
			ASTC_12x10_Unorm,
			ASTC_12x10_SRGB,
			ASTC_12x12_Unorm,
			ASTC_12x12_SRGB,

			// PVRTC
			PVRTC1_2BPP_Unorm,
			PVRTC1_2BPP_SRGB,
			PVRTC1_4BPP_Unorm,
			PVRTC1_4BPP_SRGB,
			PVRTC2_2BPP_Unorm,
			PVRTC2_2BPP_SRGB,
			PVRTC2_4BPP_Unorm,
			PVRTC2_4BPP_SRGB,

			FirstCompressed = BC1_RGB_Unorm,
			LastCompressed = PVRTC2_4BPP_SRGB,

			// Depth-stencil formats
			D16_Unorm,
			D24_Unorm_S8_Uint,
			D24_Unorm_X8,
			D32_Float,
			D32_Float_S8_Uint,

			// Meta formats

			Count,
			Last = D32_Float_S8_Uint,
			Invalid,
		};
	};

	// TODO Move to core
	template<typename T> constexpr const char* CrTypeName();
	template<typename T> constexpr const char* CrTypeName() { return CrTypeName<T>(); }

	template <> constexpr const char* CrTypeName<uint8_t>()		{ return "uint8_t"; }
	template <> constexpr const char* CrTypeName<uint16_t>()	{ return "uint16_t"; }
	template <> constexpr const char* CrTypeName<uint32_t>()	{ return "uint32_t"; }
	template <> constexpr const char* CrTypeName<half>()		{ return "half"; }
	template <> constexpr const char* CrTypeName<float>()		{ return "float"; }

	struct DataFormatInfo
	{
		cr3d::DataFormat::T format : 8;
		uint32_t dataOrBlockSize   : 5; // Bytes
		uint32_t elementSizeR      : 7; // Bits
		uint32_t elementSizeG      : 7; // Bits
		uint32_t elementSizeB      : 7; // Bits
		uint32_t elementSizeA      : 7; // Bits
		uint32_t numComponents     : 3;
		uint32_t compressed        : 1;
		const char* name;
	};

	template<typename type>
	constexpr DataFormatInfo CreateDataFormatInfo(cr3d::DataFormat::T enumEntry, uint32_t numComponents, bool compressed)
	{
		return { enumEntry, sizeof(type) * numComponents, sizeof(type) * 8, sizeof(type) * 8, sizeof(type) * 8, sizeof(type) * 8, numComponents, compressed, CrTypeName<type>() };
	}

	constexpr DataFormatInfo CreateDataFormatInfo
	(
		cr3d::DataFormat::T enumEntry, uint32_t dataSize, 
		uint32_t elementSizeR, uint32_t elementSizeG, uint32_t elementSizeB, uint32_t elementSizeA, 
		uint32_t numComponents, bool compressed, const char* name
	)
	{
		return { enumEntry, dataSize, elementSizeR, elementSizeG, elementSizeB, elementSizeA, numComponents, compressed, name };
	}

	constexpr DataFormatInfo DataFormats[cr3d::DataFormat::Count] = 
	{
		// 8-bit formats
		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::R8_Unorm, 1, false),
		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::R8_Snorm, 1, false),
		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::R8_Uint,  1, false),
		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::R8_Sint,  1, false),

		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::RG8_Unorm, 2, false),
		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::RG8_Snorm, 2, false),
		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::RG8_Uint,  2, false),
		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::RG8_Sint,  2, false),

		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::RGBA8_Unorm, 4, false),
		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::RGBA8_Snorm, 4, false),
		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::RGBA8_Uint,  4, false),
		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::RGBA8_Snorm, 4, false),
		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::RGBA8_SRGB,  4, false),

		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::BGRA8_Unorm, 4, false),
		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::BGRA8_SRGB, 4, false),

		// 16-bit integer formats

		CreateDataFormatInfo<uint16_t>(cr3d::DataFormat::R16_Unorm, 1, false),
		CreateDataFormatInfo<uint16_t>(cr3d::DataFormat::R16_Snorm, 1, false),
		CreateDataFormatInfo<uint16_t>(cr3d::DataFormat::R16_Uint,  1, false),
		CreateDataFormatInfo<uint16_t>(cr3d::DataFormat::R16_Sint,  1, false),

		CreateDataFormatInfo<uint16_t>(cr3d::DataFormat::RG16_Unorm, 2, false),
		CreateDataFormatInfo<uint16_t>(cr3d::DataFormat::RG16_Snorm, 2, false),
		CreateDataFormatInfo<uint16_t>(cr3d::DataFormat::RG16_Uint,  2, false),
		CreateDataFormatInfo<uint16_t>(cr3d::DataFormat::RG16_Sint,  2, false),

		CreateDataFormatInfo<uint16_t>(cr3d::DataFormat::RGBA16_Unorm, 4, false),
		CreateDataFormatInfo<uint16_t>(cr3d::DataFormat::RGBA16_Snorm, 4, false),
		CreateDataFormatInfo<uint16_t>(cr3d::DataFormat::RGBA16_Uint,  4, false),
		CreateDataFormatInfo<uint16_t>(cr3d::DataFormat::RGBA16_Sint,  4, false),

		// 16-bit float formats

		CreateDataFormatInfo<half>(cr3d::DataFormat::R16_Float,    1, false),
		CreateDataFormatInfo<half>(cr3d::DataFormat::RG16_Float,   2, false),
		CreateDataFormatInfo<half>(cr3d::DataFormat::RGBA16_Float, 4, false),

		// 32-bit integer formats
		CreateDataFormatInfo<uint32_t>(cr3d::DataFormat::R32_Uint,    1, false),
		CreateDataFormatInfo<uint32_t>(cr3d::DataFormat::R32_Sint,    1, false),
		CreateDataFormatInfo<uint32_t>(cr3d::DataFormat::RG32_Uint,   2, false),
		CreateDataFormatInfo<uint32_t>(cr3d::DataFormat::RG32_Sint,   2, false),
		CreateDataFormatInfo<uint32_t>(cr3d::DataFormat::RGB32_Uint,  3, false),
		CreateDataFormatInfo<uint32_t>(cr3d::DataFormat::RGB32_Sint,  3, false),
		CreateDataFormatInfo<uint32_t>(cr3d::DataFormat::RGBA32_Uint, 4, false),
		CreateDataFormatInfo<uint32_t>(cr3d::DataFormat::RGBA32_Sint, 4, false),

		// 32-bit float formats
		CreateDataFormatInfo<float>(cr3d::DataFormat::R32_Float,    1, false),
		CreateDataFormatInfo<float>(cr3d::DataFormat::RG32_Float,   2, false),
		CreateDataFormatInfo<float>(cr3d::DataFormat::RGB32_Float,  3, false),
		CreateDataFormatInfo<float>(cr3d::DataFormat::RGBA32_Float, 4, false),

		// TODO fix compressed and varying size formats - need to change bytes to bits, also explicit name
		CreateDataFormatInfo(cr3d::DataFormat::RGB10A2_Unorm,     4, 10, 10, 10, 2, 4, false, "rgb10a2"),
		CreateDataFormatInfo(cr3d::DataFormat::RGB10A2_Uint,      4, 10, 10, 10, 2, 4, false, "rgb10a2"),
		CreateDataFormatInfo(cr3d::DataFormat::B5G5R5A1_Unorm,    2,  5,  5,  5, 1, 4, false, "rgb5a1"),
		CreateDataFormatInfo(cr3d::DataFormat::BGRA4_Unorm,       2,  4,  4,  4, 4, 4, false, "rgba4"),

		CreateDataFormatInfo(cr3d::DataFormat::RG11B10_Float,     4, 11, 11, 10, 0, 3, false, "rg11b10f"),
		CreateDataFormatInfo(cr3d::DataFormat::RGB9E5_Float,      4,  9,  9,  9, 0, 3, false, "rgb9e5"),

		// TODO the compressed formats need reviewing
		CreateDataFormatInfo(cr3d::DataFormat::BC1_RGB_Unorm,     1,  8,  8,  8, 0, 3, true, "bc1"),
		CreateDataFormatInfo(cr3d::DataFormat::BC1_RGB_SRGB,      1,  8,  8,  8, 0, 3, true, "bc1"),
		CreateDataFormatInfo(cr3d::DataFormat::BC1_RGBA_Unorm,    1,  8,  8,  8, 8, 4, true, "bc1"),
		CreateDataFormatInfo(cr3d::DataFormat::BC1_RGBA_SRGB,     1,  8,  8,  8, 8, 4, true, "bc1"),

		CreateDataFormatInfo(cr3d::DataFormat::BC2_Unorm,         2,  8,  8,  8, 8, 4, true, "bc2"),
		CreateDataFormatInfo(cr3d::DataFormat::BC2_SRGB,          2,  8,  8,  8, 8, 4, true, "bc2"),

		CreateDataFormatInfo(cr3d::DataFormat::BC3_Unorm,         2,  8,  8,  8, 8, 4, true, "bc3"),
		CreateDataFormatInfo(cr3d::DataFormat::BC3_SRGB,          2,  8,  8,  8, 8, 4, true, "bc3"),

		CreateDataFormatInfo(cr3d::DataFormat::BC4_Unorm,         1,  8,  8,  8, 8, 4, true, "bc4"),
		CreateDataFormatInfo(cr3d::DataFormat::BC4_Snorm,         1,  8,  8,  8, 8, 4, true, "bc4"),

		CreateDataFormatInfo(cr3d::DataFormat::BC5_Unorm,         2,  8,  8,  8, 8, 4, true, "bc5"),
		CreateDataFormatInfo(cr3d::DataFormat::BC5_Snorm,         2,  8,  8,  8, 8, 4, true, "bc5"),

		CreateDataFormatInfo(cr3d::DataFormat::BC6H_UFloat,       2,  8,  8,  8, 8, 4, true, "bc6h"),
		CreateDataFormatInfo(cr3d::DataFormat::BC6H_SFloat,       2,  8,  8,  8, 8, 4, true, "bc6h"),

		CreateDataFormatInfo(cr3d::DataFormat::BC7_Unorm,         2,  8,  8,  8, 8, 4, true, "bc7"),
		CreateDataFormatInfo(cr3d::DataFormat::BC7_SRGB,          2,  8,  8,  8, 8, 4, true, "bc7"),

		CreateDataFormatInfo(cr3d::DataFormat::ETC2_RGB8_Unorm,   4,  8,  8,  8, 8, 4, true, "etc2"),
		CreateDataFormatInfo(cr3d::DataFormat::ETC2_RGB8_SRGB,    4,  8,  8,  8, 8, 4, true, "etc2"),

		CreateDataFormatInfo(cr3d::DataFormat::ETC2_RGB8A1_Unorm, 4,  8,  8,  8, 8, 4, true, "etc2"),
		CreateDataFormatInfo(cr3d::DataFormat::ETC2_RGB8A1_SRGB,  4,  8,  8,  8, 8, 4, true, "etc2")
	};

	//static_assert(DataFormats[cr3d::DataFormat::Last].format == cr3d::DataFormat::Last, "");

	namespace GraphicsApi
	{
		enum T : uint32_t
		{
			Vulkan,
			D3D12,
			Metal,
			Count
		};
	}

	namespace TextureContent
	{
		enum T : uint32_t
		{
			Diffuse,
			Normals,
			Specular,
			Displacement,
			Emissive,
			Heightmap,
		};
	};

	namespace TextureUsage
	{
		enum T : uint32_t
		{
			Default         = 1 << 0, // Standard usage for textures loaded from file

			CPUReadable     = 1 << 1, // Can use map/unmap on it

			Depth           = 1 << 2, // Use for depth texture

			RenderTarget    = 1 << 3, // Use as render target

			UnorderedAccess = 1 << 4, // Use as unordered access

			SwapChain       = 1 << 5, // Use as image in the swapchain
		};
	};

	using TextureUsageFlags = uint32_t;

	enum class TextureType : uint8_t
	{
		Tex1D, 
		Tex2D, 
		Cubemap, 
		Volume, 
		TextureArray
	};

	namespace ShaderStage
	{
		// Most shaders won't have Hull/Domain or Geometry, for performance we'll reorder them
		enum T : uint32_t
		{
			Vertex,
			Pixel,
			Hull,
			Domain,
			Geometry,
			Compute,
			GraphicsStageCount = Geometry + 1,
			Count = Compute + 1
		};

		inline T& operator++(T& e) { e = static_cast<T>(static_cast<uint32_t>(e) + 1u); return e; } // Pre-increment
		inline T operator++(T& e, int) { T temp = e; e = static_cast<T>(static_cast<uint32_t>(e) + 1u); return temp; } // Post-increment
		inline T& operator--(T& e) { e = static_cast<T>(static_cast<uint32_t>(e) - 1u); return e; } // Pre-decrement
		inline T operator--(T& e, int) { T temp = e; e = static_cast<T>(static_cast<uint32_t>(e) - 1u); return temp; } // Post-decrement
	};

	namespace ResourceState
	{
		enum T : uint32_t
		{
			Undefined        = 0, // Never use this as the destination state in a resource transition operation
			ShaderInput      = 1, // Use as input to a shader other than the pixel shader
			PixelShaderInput = 2, // Use as input to a pixel shader
			RenderTarget     = 3, // Use as a Render Target
			UnorderedAccess  = 4, // Use as Unordered Access
			Present          = 5, // Use as swapchain image
			DepthRead        = 6, // Use as depth read
			DepthWrite       = 7, // Use to write depth to it
			CopyDestination  = 8, // Use as destination of copy operation
			CopySource       = 9, // Use as source of copy operation
			PreInitialized   = 10,
		};
	};

	namespace ShaderResourceType
	{
		enum T : uint32_t
		{
			ConstantBuffer,
			Texture,
			Sampler,
			RWStructuredBuffer,
			ROStructuredBuffer,
			RWTexture,
			DataBuffer,
			RWDataBuffer,
			Count,
			Start = ConstantBuffer,
		};
		
		inline T& operator++(T& e) { e = static_cast<T>(static_cast<uint32_t>(e) + 1u); return e; } // Pre-increment
		inline T operator++(T& e, int) { T temp = e; e = static_cast<T>(static_cast<uint32_t>(e) + 1u); return temp; } // Post-increment
		inline T& operator--(T& e) { e = static_cast<T>(static_cast<uint32_t>(e) - 1u); return e; } // Pre-decrement
		inline T operator--(T& e, int) { T temp = e; e = static_cast<T>(static_cast<uint32_t>(e) - 1u); return temp; } // Post-decrement
	};

	enum class ShaderCodeFormat : uint8_t
	{
		SourceHLSL,
		Binary,
	};

	enum class PrimitiveTopology : uint8_t
	{
		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip,
		// Some platforms (Vulkan) support triangle fans but DX10+ for instance doesn't so we don't expose them
		// https://msdn.microsoft.com/en-us/library/windows/desktop/cc308047(v=vs.85).aspx
		// Adjacency information is for geometry shaders
		LineListAdjacency,
		LineStripAdjacency,
		TriangleListAdjacency,
		TriangleStripAdjacency,
		// Patches are for tessellation shaders
		PatchList
	};

	enum class PolygonFillMode : uint8_t { Fill, Line };
	enum class PolygonCullMode : uint8_t { None, Front, Back };
	enum class FrontFace : uint8_t { Clockwise, CounterClockwise };
	enum class BlendOp : uint8_t { Add, Subtract, ReverseSubtract, Min, Max, };

	enum class BlendFactor : uint8_t
	{
		Zero,
		One,
		SrcColor,
		OneMinusSrcColor,
		DstColor,
		OneMinusDstColor,
		SrcAlpha,
		OneMinusSrcAlpha,
		DstAlpha,
		OneMinusDstAlpha,
		Constant,
		OneMinusConstant,
		// Vulkan has a blend factor called CONSTANT_ALPHA but it's redundant compared to CONSTANT_COLOR. Funnily, DirectX has entries 12 and 13
		// from the D3D1X_BLEND structure missing, which suggests at some point they had something similar but removed them
		SrcAlphaSaturate,
		Src1Color,
		OneMinusSrc1Color,
		Src1Alpha,
		OneMinusSrc1Alpha,
	};

	enum class LogicOp : uint8_t
	{
		Clear,
		And,
		AndReverse,
		Copy,
		AndInverted,
		Noop,
		Xor,
		Or,
		Nor,
		Equivalent,
		Invert,
		OrReverse,
		CopyInverted,
		OrInverted,
		Nand,
		Set,
	};

	typedef uint32_t ColorWriteMask;
	namespace ColorWriteComponent
	{
		enum T : uint8_t
		{
			Red   = 1 << 0,
			Green = 1 << 1,
			Blue  = 1 << 2,
			Alpha = 1 << 3,
		};

		static const ColorWriteMask All = ColorWriteComponent::Red | ColorWriteComponent::Green | ColorWriteComponent::Blue | ColorWriteComponent::Alpha;
		static const ColorWriteMask ColorOnly = ColorWriteComponent::Red | ColorWriteComponent::Green | ColorWriteComponent::Blue;
		static const ColorWriteMask AlphaOnly = ColorWriteComponent::Alpha;
	};

	enum class CompareOp : uint8_t
	{
		Never,
		Less,
		Equal,
		LessOrEqual,
		Greater,
		NotEqual,
		GreaterOrEqual,
		Always,
	};

	enum class StencilOp : uint8_t
	{
		Keep,
		Zero,
		Replace,
		IncrementSaturate,
		DecrementSaturate,
		Invert,
		IncrementAndWrap,
		DecrementAndWrap,
	};

	enum class SampleCount : uint8_t
	{
		S1, S2, S4, S8, S16, S32, S64
	};

	enum class Filter : uint8_t
	{
		Point = 0,
		Linear = 1,
	};

	enum class AddressMode : uint8_t
	{
		ClampToEdge,
		ClampToBorder,
		Wrap,
		Mirror,
		MirrorOnce,
	};

	// DX12 has configurable border colors, but most platforms only support these predefined modes
	enum class BorderColor : uint8_t
	{
		TransparentBlack,
		OpaqueBlack,
		OpaqueWhite,
	};

	constexpr bool IsFormatCompressed(DataFormat::T format)
	{
		return (format >= DataFormat::FirstCompressed && format <= DataFormat::LastCompressed);
	}

	constexpr bool IsFormatSRGB(DataFormat::T format)
	{
		switch (format)
		{
			case DataFormat::RGBA8_SRGB:
			case DataFormat::BGRA8_SRGB:

			case DataFormat::BC1_RGB_SRGB:
			case DataFormat::BC2_SRGB:
			case DataFormat::BC3_SRGB:
			case DataFormat::BC7_SRGB:

			case DataFormat::ETC2_RGB8_SRGB:
			case DataFormat::ETC2_RGB8A1_SRGB:
			case DataFormat::ETC2_RGBA8_SRGB:

			case DataFormat::ASTC_4x4_SRGB:
			case DataFormat::ASTC_5x4_SRGB:
			case DataFormat::ASTC_5x5_SRGB:
			case DataFormat::ASTC_6x5_SRGB:
			case DataFormat::ASTC_6x6_SRGB:
			case DataFormat::ASTC_8x5_SRGB:
			case DataFormat::ASTC_8x6_SRGB:
			case DataFormat::ASTC_8x8_SRGB:
			case DataFormat::ASTC_10x5_SRGB:
			case DataFormat::ASTC_10x6_SRGB:
			case DataFormat::ASTC_10x8_SRGB:
			case DataFormat::ASTC_10x10_SRGB:
			case DataFormat::ASTC_12x10_SRGB:
			case DataFormat::ASTC_12x12_SRGB:

			case DataFormat::PVRTC1_2BPP_SRGB:
			case DataFormat::PVRTC1_4BPP_SRGB:
			case DataFormat::PVRTC2_2BPP_SRGB:
			case DataFormat::PVRTC2_4BPP_SRGB:
				return true;
			default:
				return false;
		}
	}

	constexpr uint32_t GetFormatBitsPerPixelOrBlock(DataFormat::T format)
	{
		if (format >= DataFormat::ASTC_4x4_Unorm && format <= DataFormat::ASTC_12x12_SRGB)
		{
			return 128; // All ASTC blocks are the same size
		}

		switch (format)
		{
			case DataFormat::R8_Unorm:
			case DataFormat::R8_Uint:
			case DataFormat::R8_Snorm:
			case DataFormat::R8_Sint:
				return 8;
			case DataFormat::RG8_Unorm:
			case DataFormat::RG8_Uint:
			case DataFormat::RG8_Snorm:
			case DataFormat::RG8_Sint:
			case DataFormat::R16_Float:
			case DataFormat::D16_Unorm:
			case DataFormat::R16_Unorm:
			case DataFormat::R16_Uint:
			case DataFormat::R16_Snorm:
			case DataFormat::R16_Sint:
			case DataFormat::B5G6R5_Unorm:
			case DataFormat::B5G5R5A1_Unorm:
			case DataFormat::BGRA4_Unorm:
				return 16;
			case DataFormat::BC1_RGB_Unorm:
			case DataFormat::BC1_RGB_SRGB:
			case DataFormat::BC1_RGBA_Unorm:
			case DataFormat::BC1_RGBA_SRGB:
			case DataFormat::BC4_Unorm:
			case DataFormat::BC4_Snorm:
			case DataFormat::RGBA16_Float:
			case DataFormat::RGBA16_Unorm:
			case DataFormat::RGBA16_Uint:
			case DataFormat::RGBA16_Snorm:
			case DataFormat::RGBA16_Sint:
			case DataFormat::RG32_Float:
			case DataFormat::RG32_Uint:
			case DataFormat::RG32_Sint:
			case DataFormat::D32_Float_S8_Uint:
				return 64;
			case DataFormat::RGB32_Float:
			case DataFormat::RGB32_Uint:
			case DataFormat::RGB32_Sint:
				return 96;
			case DataFormat::BC2_Unorm:
			case DataFormat::BC2_SRGB:
			case DataFormat::BC3_Unorm:
			case DataFormat::BC3_SRGB:
			case DataFormat::BC5_Unorm:
			case DataFormat::BC5_Snorm:
			case DataFormat::BC6H_UFloat:
			case DataFormat::BC6H_SFloat:
			case DataFormat::BC7_Unorm:
			case DataFormat::BC7_SRGB:
			case DataFormat::RGBA32_Float:
			case DataFormat::RGBA32_Uint:
			case DataFormat::RGBA32_Sint:
				return 128;
			default:
				return 32; // Most formats are 32 bits per pixel
				break;
		}
	}

	constexpr void GetFormatBlockWidthHeight(DataFormat::T format, uint32_t& blockWidth, uint32_t& blockHeight)
	{
		switch (format)
		{
			case DataFormat::BC1_RGB_Unorm:
			case DataFormat::BC1_RGB_SRGB:
			case DataFormat::BC1_RGBA_Unorm:
			case DataFormat::BC1_RGBA_SRGB:
			case DataFormat::BC4_Unorm:
			case DataFormat::BC4_Snorm:
			case DataFormat::BC2_Unorm:
			case DataFormat::BC2_SRGB:
			case DataFormat::BC3_Unorm:
			case DataFormat::BC3_SRGB:
			case DataFormat::BC5_Unorm:
			case DataFormat::BC5_Snorm:
			case DataFormat::BC6H_UFloat:
			case DataFormat::BC6H_SFloat:
			case DataFormat::BC7_Unorm:
			case DataFormat::BC7_SRGB:
			case DataFormat::ASTC_4x4_Unorm:
			case DataFormat::ASTC_4x4_SRGB:
				blockWidth = 4; blockHeight = 4;
				break;
			case DataFormat::ASTC_5x4_Unorm:
			case DataFormat::ASTC_5x4_SRGB:
				blockWidth = 5; blockHeight = 4;
				break;
			case DataFormat::ASTC_5x5_Unorm:
			case DataFormat::ASTC_5x5_SRGB:
				blockWidth = 5; blockHeight = 5;
				break;
			case DataFormat::ASTC_6x5_Unorm:
			case DataFormat::ASTC_6x5_SRGB:
				blockWidth = 6; blockHeight = 5;
			case DataFormat::ASTC_6x6_Unorm:
			case DataFormat::ASTC_6x6_SRGB:
				blockWidth = 6; blockHeight = 6;
			case DataFormat::ASTC_8x5_Unorm:
			case DataFormat::ASTC_8x5_SRGB:
				blockWidth = 8; blockHeight = 5;
			case DataFormat::ASTC_8x6_Unorm:
			case DataFormat::ASTC_8x6_SRGB:
				blockWidth = 8; blockHeight = 6;
			case DataFormat::ASTC_8x8_Unorm:
			case DataFormat::ASTC_8x8_SRGB:
				blockWidth = 8; blockHeight = 8;
			case DataFormat::ASTC_10x5_Unorm:
			case DataFormat::ASTC_10x5_SRGB:
				blockWidth = 10; blockHeight = 5;
			case DataFormat::ASTC_10x6_Unorm:
			case DataFormat::ASTC_10x6_SRGB:
				blockWidth = 10; blockHeight = 6;
			case DataFormat::ASTC_10x8_Unorm:
			case DataFormat::ASTC_10x8_SRGB:
				blockWidth = 10; blockHeight = 8;
			case DataFormat::ASTC_10x10_Unorm:
			case DataFormat::ASTC_10x10_SRGB:
				blockWidth = 10; blockHeight = 10;
			case DataFormat::ASTC_12x10_Unorm:
			case DataFormat::ASTC_12x10_SRGB:
				blockWidth = 12; blockHeight = 10;
			case DataFormat::ASTC_12x12_Unorm:
			case DataFormat::ASTC_12x12_SRGB:
				blockWidth = 12; blockHeight = 12;
			default:
				blockWidth = 1; blockHeight = 1;
			break;
		}
	}

	constexpr bool IsDepthFormat(DataFormat::T format)
	{
		switch (format)
		{
			case cr3d::DataFormat::D16_Unorm:
			case cr3d::DataFormat::D24_Unorm_S8_Uint:
			case cr3d::DataFormat::D24_Unorm_X8:
			case cr3d::DataFormat::D32_Float:
			case cr3d::DataFormat::D32_Float_S8_Uint:
				return true;
			default:
				return false;
		}
	}

	enum class CameraProjection : uint32_t
	{
		Orthographic,
		Perspective
	};
}

namespace cr3d
{
	namespace BufferUsage
	{
		enum T : uint8_t
		{
			Constant = 1 << 0,
			Vertex = 1 << 1,
			Index = 1 << 2,
			Structured = 1 << 3,
			Data = 1 << 4,
			Byte = 1 << 5,
			Indirect = 1 << 6,

			// Compound
			Storage = Structured | Byte,
		};
	};

	namespace BufferAccess
	{
		enum T : uint8_t
		{
			Immutable = 1 << 0,
			GPUWrite = 1 << 1,
			CPUWrite = 1 << 2,
			CPURead = 1 << 3,
		};
	};

	namespace BufferOwnership
	{
		enum T : uint8_t
		{
			NonOwning,
			Owning
		};
	}

	enum class GPUWaitResult : uint32_t
	{
		Success, // If this wait completed before the timeout
		Timeout, // If this wait did not complete before the timeout
		Error // If some error occurred
	};
}

struct CrViewport
{
	CrViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
		: x(x), y(y), width(width), height(height), minDepth(minDepth), maxDepth(maxDepth)
	{}

	CrViewport(float x, float y, float width, float height) : CrViewport(x, y, width, height, 0.0f, 1.0f)
	{}

	float x;
	float y;
	float width;
	float height;
	float minDepth;
	float maxDepth;
};