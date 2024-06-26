#pragma once

#include "Math/CrHalf.h"

namespace cr3d
{
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

			// Depth-stencil formats
			D16_Unorm,
			D24_Unorm_S8_Uint,
			D24_Unorm_X8,
			D32_Float,
			D32_Float_S8_Uint,

			Count,
			Invalid,

			// Quickly check for ranges of texture formats
			FirstUncompressed = R8_Unorm,
			LastUncompressed  = RGB9E5_Float,

			FirstCompressed = BC1_RGB_Unorm,
			LastCompressed  = PVRTC2_4BPP_SRGB,

			FirstBC = BC1_RGB_Unorm,
			LastBC  = BC7_SRGB,

			FirstETC_EAC = ETC2_RGB8_Unorm,
			LastETC_EAC  = EAC_R11G11_Snorm,

			FirstASTC = ASTC_4x4_Unorm,
			LastASTC  = ASTC_12x12_SRGB,

			FirstPVRTC = PVRTC1_2BPP_Unorm,
			LastPVRTC  = PVRTC2_4BPP_SRGB,
		};

		inline T& operator++(T& e) { e = static_cast<T>(static_cast<uint32_t>(e) + 1u); return e; } // Pre-increment
		inline T operator++(T& e, int) { T temp = e; e = static_cast<T>(static_cast<uint32_t>(e) + 1u); return temp; } // Post-increment
		inline T& operator--(T& e) { e = static_cast<T>(static_cast<uint32_t>(e) - 1u); return e; } // Pre-decrement
		inline T operator--(T& e, int) { T temp = e; e = static_cast<T>(static_cast<uint32_t>(e) - 1u); return temp; } // Post-decrement
	};

	// TODO Move to core
	template<typename T> constexpr const char* CrTypeName();
	template<typename T> constexpr const char* CrTypeName() { return CrTypeName<T>(); }

	template <> constexpr const char* CrTypeName<int8_t>()   { return "int8_t"; }
	template <> constexpr const char* CrTypeName<int16_t>()  { return "int16_t"; }
	template <> constexpr const char* CrTypeName<int32_t>()  { return "int32_t"; }
	template <> constexpr const char* CrTypeName<uint8_t>()  { return "uint8_t"; }
	template <> constexpr const char* CrTypeName<uint16_t>() { return "uint16_t"; }
	template <> constexpr const char* CrTypeName<uint32_t>() { return "uint32_t"; }
	template <> constexpr const char* CrTypeName<half>()     { return "half"; }
	template <> constexpr const char* CrTypeName<float>()    { return "float"; }

	struct DataFormatInfo
	{
		cr3d::DataFormat::T format : 8;
		uint32_t dataOrBlockSize   : 5; // Bytes
		uint32_t blockWidth        : 5; // Pixels
		uint32_t blockHeight       : 5; // Pixels
		uint32_t elementSizeR      : 7; // Bits
		uint32_t elementSizeG      : 7; // Bits
		uint32_t elementSizeB      : 7; // Bits
		uint32_t elementSizeA      : 7; // Bits
		uint32_t numComponents     : 3;
		uint32_t compressed        : 1; // Whether format is compressed
		uint32_t hdrFloat          : 1; // Whether format is floating point HDR (covers float, half and packed)
		const char* name;
	};

	static_assert(sizeof(DataFormatInfo) == 8 + sizeof(DataFormatInfo::name), "Size mismatch");

	template<typename type>
	constexpr DataFormatInfo CreateDataFormatInfo(cr3d::DataFormat::T enumEntry, uint32_t numComponents, bool compressed, bool hdrFloat)
	{
		return
		{
			enumEntry,
			(uint32_t) sizeof(type) * numComponents,
			1,
			1,
			(uint32_t) sizeof(type) * 8,
			(uint32_t) sizeof(type) * 8 * (numComponents > 1),
			(uint32_t) sizeof(type) * 8 * (numComponents > 2),
			(uint32_t) sizeof(type) * 8 * (numComponents > 3),
			numComponents,
			compressed,
			hdrFloat,
			CrTypeName<type>()
		};
	}

	constexpr DataFormatInfo CreateDataFormatInfo
	(
		cr3d::DataFormat::T enumEntry, uint32_t dataOrBlockSize, uint32_t blockWidth, uint32_t blockHeight,
		uint32_t elementSizeR, uint32_t elementSizeG, uint32_t elementSizeB, uint32_t elementSizeA, 
		uint32_t numComponents, bool compressed, bool hdrFloat, const char* name
	)
	{
		return { enumEntry, dataOrBlockSize, blockWidth, blockHeight, elementSizeR, elementSizeG, elementSizeB, elementSizeA, numComponents, compressed, hdrFloat, name };
	}

	constexpr DataFormatInfo DataFormats[cr3d::DataFormat::Count] = 
	{
		// 8-bit formats
		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::R8_Unorm, 1, false, false),
		CreateDataFormatInfo< int8_t>(cr3d::DataFormat::R8_Snorm, 1, false, false),
		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::R8_Uint,  1, false, false),
		CreateDataFormatInfo< int8_t>(cr3d::DataFormat::R8_Sint,  1, false, false),

		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::RG8_Unorm, 2, false, false),
		CreateDataFormatInfo< int8_t>(cr3d::DataFormat::RG8_Snorm, 2, false, false),
		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::RG8_Uint,  2, false, false),
		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::RG8_Sint,  2, false, false),

		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::RGBA8_Unorm, 4, false, false),
		CreateDataFormatInfo< int8_t>(cr3d::DataFormat::RGBA8_Snorm, 4, false, false),
		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::RGBA8_Uint,  4, false, false),
		CreateDataFormatInfo< int8_t>(cr3d::DataFormat::RGBA8_Sint,  4, false, false),
		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::RGBA8_SRGB,  4, false, false),

		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::BGRA8_Unorm, 4, false, false),
		CreateDataFormatInfo<uint8_t>(cr3d::DataFormat::BGRA8_SRGB, 4, false, false),

		// 16-bit integer formats

		CreateDataFormatInfo<uint16_t>(cr3d::DataFormat::R16_Unorm, 1, false, false),
		CreateDataFormatInfo< int16_t>(cr3d::DataFormat::R16_Snorm, 1, false, false),
		CreateDataFormatInfo<uint16_t>(cr3d::DataFormat::R16_Uint,  1, false, false),
		CreateDataFormatInfo< int16_t>(cr3d::DataFormat::R16_Sint,  1, false, false),

		CreateDataFormatInfo<uint16_t>(cr3d::DataFormat::RG16_Unorm, 2, false, false),
		CreateDataFormatInfo< int16_t>(cr3d::DataFormat::RG16_Snorm, 2, false, false),
		CreateDataFormatInfo<uint16_t>(cr3d::DataFormat::RG16_Uint,  2, false, false),
		CreateDataFormatInfo< int16_t>(cr3d::DataFormat::RG16_Sint,  2, false, false),

		CreateDataFormatInfo<uint16_t>(cr3d::DataFormat::RGBA16_Unorm, 4, false, false),
		CreateDataFormatInfo< int16_t>(cr3d::DataFormat::RGBA16_Snorm, 4, false, false),
		CreateDataFormatInfo<uint16_t>(cr3d::DataFormat::RGBA16_Uint,  4, false, false),
		CreateDataFormatInfo< int16_t>(cr3d::DataFormat::RGBA16_Sint,  4, false, false),

		// 16-bit float formats

		CreateDataFormatInfo<half>(cr3d::DataFormat::R16_Float,    1, false, true),
		CreateDataFormatInfo<half>(cr3d::DataFormat::RG16_Float,   2, false, true),
		CreateDataFormatInfo<half>(cr3d::DataFormat::RGBA16_Float, 4, false, true),

		// 32-bit integer formats
		CreateDataFormatInfo<uint32_t>(cr3d::DataFormat::R32_Uint,    1, false, false),
		CreateDataFormatInfo< int32_t>(cr3d::DataFormat::R32_Sint,    1, false, false),
		CreateDataFormatInfo<uint32_t>(cr3d::DataFormat::RG32_Uint,   2, false, false),
		CreateDataFormatInfo< int32_t>(cr3d::DataFormat::RG32_Sint,   2, false, false),
		CreateDataFormatInfo<uint32_t>(cr3d::DataFormat::RGB32_Uint,  3, false, false),
		CreateDataFormatInfo< int32_t>(cr3d::DataFormat::RGB32_Sint,  3, false, false),
		CreateDataFormatInfo<uint32_t>(cr3d::DataFormat::RGBA32_Uint, 4, false, false),
		CreateDataFormatInfo< int32_t>(cr3d::DataFormat::RGBA32_Sint, 4, false, false),

		// 32-bit float formats
		CreateDataFormatInfo<float>(cr3d::DataFormat::R32_Float,    1, false, true),
		CreateDataFormatInfo<float>(cr3d::DataFormat::RG32_Float,   2, false, true),
		CreateDataFormatInfo<float>(cr3d::DataFormat::RGB32_Float,  3, false, true),
		CreateDataFormatInfo<float>(cr3d::DataFormat::RGBA32_Float, 4, false, true),

		// TODO fix compressed and varying size formats - need to change bytes to bits, also explicit name
		CreateDataFormatInfo(cr3d::DataFormat::RGB10A2_Unorm,     4, 1, 1, 10, 10, 10, 2, 4, false, false, "rgb10a2"),
		CreateDataFormatInfo(cr3d::DataFormat::RGB10A2_Uint,      4, 1, 1, 10, 10, 10, 2, 4, false, false, "rgb10a2"),
		CreateDataFormatInfo(cr3d::DataFormat::B5G6R5_Unorm,      2, 1, 1,  5,  6,  5, 0, 4, false, false, "r5g6b5"),
		CreateDataFormatInfo(cr3d::DataFormat::B5G5R5A1_Unorm,    2, 1, 1,  5,  5,  5, 1, 4, false, false, "rgb5a1"),
		CreateDataFormatInfo(cr3d::DataFormat::BGRA4_Unorm,       2, 1, 1,  4,  4,  4, 4, 4, false, false, "rgba4"),

		CreateDataFormatInfo(cr3d::DataFormat::RG11B10_Float,     4, 1, 1, 11, 11, 10, 0, 3, false, true, "rg11b10f"),
		CreateDataFormatInfo(cr3d::DataFormat::RGB9E5_Float,      4, 1, 1,  9,  9,  9, 0, 3, false, true, "rgb9e5"),

		// TODO the compressed formats need reviewing
		CreateDataFormatInfo(cr3d::DataFormat::BC1_RGB_Unorm,     1, 4, 4,  8,  8,  8, 0, 3, true, false, "bc1"),
		CreateDataFormatInfo(cr3d::DataFormat::BC1_RGB_SRGB,      1, 4, 4,  8,  8,  8, 0, 3, true, false, "bc1"),
		CreateDataFormatInfo(cr3d::DataFormat::BC1_RGBA_Unorm,    1, 4, 4,  8,  8,  8, 8, 4, true, false, "bc1"),
		CreateDataFormatInfo(cr3d::DataFormat::BC1_RGBA_SRGB,     1, 4, 4,  8,  8,  8, 8, 4, true, false, "bc1"),

		CreateDataFormatInfo(cr3d::DataFormat::BC2_Unorm,         2, 4, 4,  8,  8,  8, 8, 4, true, false, "bc2"),
		CreateDataFormatInfo(cr3d::DataFormat::BC2_SRGB,          2, 4, 4,  8,  8,  8, 8, 4, true, false, "bc2"),

		CreateDataFormatInfo(cr3d::DataFormat::BC3_Unorm,         2, 4, 4,  8,  8,  8, 8, 4, true, false, "bc3"),
		CreateDataFormatInfo(cr3d::DataFormat::BC3_SRGB,          2, 4, 4,  8,  8,  8, 8, 4, true, false, "bc3"),

		CreateDataFormatInfo(cr3d::DataFormat::BC4_Unorm,         1, 4, 4,  8,  8,  8, 8, 4, true, false, "bc4"),
		CreateDataFormatInfo(cr3d::DataFormat::BC4_Snorm,         1, 4, 4,  8,  8,  8, 8, 4, true, false, "bc4"),

		CreateDataFormatInfo(cr3d::DataFormat::BC5_Unorm,         2, 4, 4,  8,  8,  8, 8, 4, true, false, "bc5"),
		CreateDataFormatInfo(cr3d::DataFormat::BC5_Snorm,         2, 4, 4,  8,  8,  8, 8, 4, true, false, "bc5"),

		CreateDataFormatInfo(cr3d::DataFormat::BC6H_UFloat,       2, 4, 4,  8,  8,  8, 8, 4, true, true, "bc6h"),
		CreateDataFormatInfo(cr3d::DataFormat::BC6H_SFloat,       2, 4, 4,  8,  8,  8, 8, 4, true, true, "bc6h"),

		CreateDataFormatInfo(cr3d::DataFormat::BC7_Unorm,         2, 4, 4,  8,  8,  8, 8, 4, true, false, "bc7"),
		CreateDataFormatInfo(cr3d::DataFormat::BC7_SRGB,          2, 4, 4,  8,  8,  8, 8, 4, true, false, "bc7"),

		CreateDataFormatInfo(cr3d::DataFormat::ETC2_RGB8_Unorm,   4, 4, 4,  8,  8,  8, 8, 4, true, false, "etc2"),
		CreateDataFormatInfo(cr3d::DataFormat::ETC2_RGB8_SRGB,    4, 4, 4,  8,  8,  8, 8, 4, true, false, "etc2"),

		CreateDataFormatInfo(cr3d::DataFormat::ETC2_RGB8A1_Unorm, 4, 4, 4,  8,  8,  8, 8, 4, true, false, "etc2"),
		CreateDataFormatInfo(cr3d::DataFormat::ETC2_RGB8A1_SRGB,  4, 4, 4,  8,  8,  8, 8, 4, true, false, "etc2")
	};

	//static_assert(DataFormats[cr3d::DataFormat::Last].format == cr3d::DataFormat::Last, "");

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
				break;
			case DataFormat::ASTC_6x6_Unorm:
			case DataFormat::ASTC_6x6_SRGB:
				blockWidth = 6; blockHeight = 6;
				break;
			case DataFormat::ASTC_8x5_Unorm:
			case DataFormat::ASTC_8x5_SRGB:
				blockWidth = 8; blockHeight = 5;
				break;
			case DataFormat::ASTC_8x6_Unorm:
			case DataFormat::ASTC_8x6_SRGB:
				blockWidth = 8; blockHeight = 6;
				break;
			case DataFormat::ASTC_8x8_Unorm:
			case DataFormat::ASTC_8x8_SRGB:
				blockWidth = 8; blockHeight = 8;
				break;
			case DataFormat::ASTC_10x5_Unorm:
			case DataFormat::ASTC_10x5_SRGB:
				blockWidth = 10; blockHeight = 5;
				break;
			case DataFormat::ASTC_10x6_Unorm:
			case DataFormat::ASTC_10x6_SRGB:
				blockWidth = 10; blockHeight = 6;
				break;
			case DataFormat::ASTC_10x8_Unorm:
			case DataFormat::ASTC_10x8_SRGB:
				blockWidth = 10; blockHeight = 8;
				break;
			case DataFormat::ASTC_10x10_Unorm:
			case DataFormat::ASTC_10x10_SRGB:
				blockWidth = 10; blockHeight = 10;
				break;
			case DataFormat::ASTC_12x10_Unorm:
			case DataFormat::ASTC_12x10_SRGB:
				blockWidth = 12; blockHeight = 10;
				break;
			case DataFormat::ASTC_12x12_Unorm:
			case DataFormat::ASTC_12x12_SRGB:
				blockWidth = 12; blockHeight = 12;
				break;
			default:
				blockWidth = 1; blockHeight = 1;
			break;
		}
	}

	// Whether format has a depth plane
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

	// Whether format has both depth and stencil planes
	constexpr bool IsDepthStencilFormat(DataFormat::T format)
	{
		switch (format)
		{
			case cr3d::DataFormat::D24_Unorm_S8_Uint:
			case cr3d::DataFormat::D24_Unorm_X8:
			case cr3d::DataFormat::D32_Float_S8_Uint:
				return true;
			default:
				return false;
		}
	}

	// Whether depth format does not have stencil (for e.g. shadow maps)
	constexpr bool IsDepthOnlyFormat(DataFormat::T format)
	{
		switch (format)
		{
			case cr3d::DataFormat::D16_Unorm:
			case cr3d::DataFormat::D32_Float:
				return true;
			default:
				return false;
		}
	}
};