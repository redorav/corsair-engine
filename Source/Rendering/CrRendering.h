#pragma once

#include "stdint.h"

namespace cr3d
{
	static constexpr uint32_t MaxRenderTargets = 8; // Maximum render targets we can bind
	static constexpr uint32_t MaxVertexStreams = 8; // Maximum vertex streams per vertex shader
	static constexpr uint32_t MaxVertexAttributes = 8; // Maximum vertex attributes per vertex shader
	static constexpr uint32_t MaxMipmaps = 14; // Maximum mipmaps per texture (16384x16384)

	namespace GraphicsApi
	{
		enum T : uint32_t
		{
			Vulkan,
			D3D12,
			Metal,
			Count
		};

		constexpr const char* ToString(cr3d::GraphicsApi::T graphicsApi, bool lowercase = false)
		{
			switch (graphicsApi)
			{
				case cr3d::GraphicsApi::Vulkan: return lowercase ? "vulkan" : "Vulkan";
				case cr3d::GraphicsApi::D3D12:  return lowercase ? "d3d12"  : "D3D12";
				case cr3d::GraphicsApi::Metal:  return lowercase ? "metal"  : "Metal";
				default: return lowercase ? "invalid" : "Invalid";
			}
		}

		cr3d::GraphicsApi::T FromString(const char* graphicsApiString);
	}

	namespace GraphicsVendor
	{
		enum T
		{
			Unknown,
			NVIDIA,
			AMD,
			Intel
		};

		constexpr GraphicsVendor::T FromVendorID(unsigned int vendorID)
		{
			switch (vendorID)
			{
			case 0x10DE:
				return GraphicsVendor::NVIDIA;
			case 0x1002:
				return GraphicsVendor::AMD;
			case 0x8086:
				return GraphicsVendor::Intel;
			default:
				return GraphicsVendor::Unknown;
			}
		}

		GraphicsVendor::T FromString(const char* s);
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

			DepthStencil    = 1 << 2, // Use for depth stencil texture

			RenderTarget    = 1 << 3, // Use as render target

			UnorderedAccess = 1 << 4, // Use as unordered access

			SwapChain       = 1 << 5, // Use as image in the swapchain
		};
	};

	using TextureUsageFlags = uint32_t;

	enum class TextureType : uint32_t
	{
		Tex1D, 
		Tex2D, 
		Cubemap, 
		Volume
	};

	namespace ShaderStage
	{
		// Most shaders only have Vertex and Pixel shader, reorder for performance
		enum T : uint32_t
		{
			Vertex   = 0,
			Pixel    = 1,
			Hull     = 2,
			Domain   = 3,
			Geometry = 4,
			Compute  = 5,
			RootSignature = 6, // D3D12 only
			GraphicsStageCount = Geometry + 1,
			Count = Compute + 1,
		};

		inline T& operator++(T& e) { e = static_cast<T>(static_cast<uint32_t>(e) + 1u); return e; }
		inline T operator++(T& e, int) { T temp = e; e = static_cast<T>(static_cast<uint32_t>(e) + 1u); return temp; }
		inline T& operator--(T& e) { e = static_cast<T>(static_cast<uint32_t>(e) - 1u); return e; }
		inline T operator--(T& e, int) { T temp = e; e = static_cast<T>(static_cast<uint32_t>(e) - 1u); return temp; }

		constexpr const char* ToString(cr3d::ShaderStage::T stage, bool lowercase = false)
		{
			switch (stage)
			{
				case cr3d::ShaderStage::Vertex:   return lowercase ? "vertex"   : "Vertex";
				case cr3d::ShaderStage::Pixel:    return lowercase ? "pixel"    : "Pixel";
				case cr3d::ShaderStage::Hull:     return lowercase ? "hull"     : "Hull";
				case cr3d::ShaderStage::Domain:   return lowercase ? "domain"   : "Domain";
				case cr3d::ShaderStage::Geometry: return lowercase ? "geometry" : "Geometry";
				case cr3d::ShaderStage::Compute:  return lowercase ? "compute"  : "Compute";
				case cr3d::ShaderStage::RootSignature:  return lowercase ? "rootsignature" : "RootSignature";
				default: return lowercase ? "invalid" : "Invalid";
			}
		}

		cr3d::ShaderStage::T FromString(const char* shaderStageString);
	};

	// This enum exists so we can OR stages together
	namespace ShaderStageFlags
	{
		enum T : uint32_t
		{
			None     = 0,
			Vertex   = 1 << ShaderStage::Vertex,
			Pixel    = 1 << ShaderStage::Pixel,
			Hull     = 1 << ShaderStage::Hull,
			Domain   = 1 << ShaderStage::Domain,
			Geometry = 1 << ShaderStage::Geometry,
			Compute  = 1 << ShaderStage::Compute,
			Graphics = Vertex | Pixel | Hull | Domain | Geometry,
			Unused   = 0xffffffff
		};

		inline ShaderStageFlags::T operator | (ShaderStageFlags::T flag1, ShaderStageFlags::T flag2)
		{
			return (ShaderStageFlags::T)((uint32_t)flag1 | (uint32_t)flag2);
		}
	};

	// These resource layouts encapsulate resource states as a common denominator between APIs. Some platforms don't even
	// have resource layouts but instead flush caches, etc. These are ways to clue in each platform of what needs doing.
	// It might mean doing the same thing for some of the states.
	namespace TextureLayout
	{
		enum T : uint32_t
		{
			Undefined                       = 0, // Never use this as the destination state in a resource transition operation
			ShaderInput                     = 1, // Use as input to a shader (depth and stencil included when not used as depth-stencil)
			RenderTarget                    = 2, // Use as a render target
			RWTexture                       = 3, // Use as RW texture
			Present                         = 4, // Use as swapchain
			CopySource                      = 5, // Use as source of copy operation
			CopyDestination                 = 6, // Use as destination of copy operation

			// The depth stencil layouts apply to depth-only texture formats as well
			// Internally we select the most optimal layout depending on the texture format
			DepthStencilReadWrite           = 7, // Read and write to and from depth and stencil (regular usage)
			DepthStencilWrite               = 8, // Write only to depth and stencil (overwrite values)
			StencilWriteDepthReadOnly       = 9, // Read-only depth, write to stencil
			DepthWriteStencilReadOnly       = 10, // Write to depth, read-only stencil
			DepthStencilReadOnly            = 11, // Read only from depth and stencil

			// States where we test and read from shader
			DepthStencilReadOnlyShader      = 12, // Test depth and stencil and read in shader
			DepthWriteStencilReadOnlyShader = 13,
			StencilWriteDepthReadOnlyShader = 14,

			Count
		};

		inline const char* ToString(cr3d::TextureLayout::T textureLayout)
		{
			switch (textureLayout)
			{
				case Undefined: return "Undefined";
				case ShaderInput: return "ShaderInput";
				case RenderTarget: return "RenderTarget";
				case RWTexture: return "RWTexture";
				case Present: return "Present";
				case DepthStencilReadWrite: return "DepthStencilWrite";
				case DepthStencilWrite: return "DepthStencilWrite";
				case StencilWriteDepthReadOnly: return "StencilWriteDepthRead";
				case DepthWriteStencilReadOnly: return "DepthWriteStencilRead";
				case DepthStencilReadOnly: return "DepthStencilRead";
				case DepthStencilReadOnlyShader: return "DepthStencilReadAndShader";
				case DepthWriteStencilReadOnlyShader: return "DepthWriteStencilReadAndShader";
				case StencilWriteDepthReadOnlyShader: return "StencilWriteDepthReadAndShader";
				case CopySource: return "CopySource";
				case CopyDestination: return "CopyDestination";
				default: return "";
			}
		}
	};


	struct TextureState
	{
		TextureState() = default;

		TextureState(TextureLayout::T layout, ShaderStageFlags::T stages) : layout(layout), stages(stages) {}

		TextureLayout::T layout = TextureLayout::Undefined;
		ShaderStageFlags::T stages = ShaderStageFlags::None;

		bool operator == (const TextureState& other) const { return layout == other.layout && stages == other.stages; }
		bool operator != (const TextureState& other) const { return layout != other.layout || stages != other.stages; }
	};

	namespace BufferState
	{
		enum T : uint32_t
		{
			Undefined,
			ShaderInput, // StructuredBuffer, Buffer, ByteAddressBuffer
			ReadWrite,   // RWStructuredBuffer, RWBuffer, RWByteAddressBuffer
			CopySource,
			CopyDestination,
			IndirectArgument, // Use buffer as argument to indirect draw/dispatch
			Count
		};

		inline const char* ToString(cr3d::BufferState::T bufferState)
		{
			switch (bufferState)
			{
				case Undefined: return "Undefined";
				case ShaderInput: return "ShaderInput";
				case ReadWrite: return "ReadWrite";
				case CopySource: return "CopySource";
				case CopyDestination: return "CopyDestination";
				case IndirectArgument: return "IndirectArgument";
				default: return "";
			}
		}
	};

	namespace RenderPassType
	{
		enum T
		{
			Graphics,
			Compute
		};
	};

	// It's hard to map certain resources between different APIs, including the usage that is later done in the API
	// so this list caters for things that are common enough that it isn't complicated to map either
	// For example, the RW prefix caters for D3D's way of having resource views (SRV for read-only and UAV for RW)
	// but this list doesn't cover ByteBuffers or Append as they fall under the umbrella of Storage Buffer
	namespace ShaderResourceType
	{
		enum T : uint32_t
		{
			ConstantBuffer,
			Sampler,
			Texture,
			RWTexture,
			StorageBuffer, // StorageBuffers include HLSL StructuredBuffer and ByteBuffer
			RWStorageBuffer,
			TypedBuffer,
			RWTypedBuffer,
			Count,
		};
	};

	namespace CubemapFace
	{
		enum T
		{
			PositiveX = 0,
			NegativeX = 1,
			PositiveY = 2,
			NegativeY = 3,
			PositiveZ = 4,
			NegativeZ = 5,
			Count
		};

		inline T& operator++(T& e) { e = static_cast<T>(static_cast<uint32_t>(e) + 1u); return e; }
		inline T operator++(T& e, int) { T temp = e; e = static_cast<T>(static_cast<uint32_t>(e) + 1u); return temp; }
		inline T& operator--(T& e) { e = static_cast<T>(static_cast<uint32_t>(e) - 1u); return e; }
		inline T operator--(T& e, int) { T temp = e; e = static_cast<T>(static_cast<uint32_t>(e) - 1u); return temp; }
	};

	namespace ShaderInterfaceBuiltinType
	{
		enum T : uint32_t
		{
			Position, // SV_Position
			BaseInstance,
			InstanceId, // SV_InstanceID
			VertexId, // SV_VertexID
			Depth, // SV_Depth
			IsFrontFace, // SV_IsFrontFace

			GroupId, // SV_GroupID
			GroupIndex, // SV_GroupIndex
			GroupThreadId, // SV_GroupThreadID
			DispatchThreadId, // SV_DispatchThreadID

			None = 0xff, // Not a builtin
		};
	};

	enum class PrimitiveTopology : uint32_t
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

	enum class VertexInputRate : uint32_t
	{
		Vertex = 0,
		Instance = 1
	};

	enum class PolygonCullMode : uint32_t { None, Front, Back };
	enum class PolygonFillMode : uint32_t
	{
		Fill = 0, // Fill the triangle
		Line = 1, // Fill the line
	};
	enum class FrontFace : uint32_t { Clockwise, CounterClockwise };
	enum class BlendOp : uint32_t { Add, Subtract, ReverseSubtract, Min, Max, };

	enum class BlendFactor : uint32_t
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

	enum class LogicOp : uint32_t
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
		enum T : uint32_t
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

	// A texture can have multiple aspects, which D3D calls planes and Vulkan calls aspects
	// For example, a depth buffer can have both depth and stencil and certain movie formats have several 'textures' embedded
	namespace TexturePlane
	{
		enum T : uint32_t
		{
			Plane0  = 0,
			Plane1  = 1,
			Plane2  = 2,
			Color   = Plane0,
			Depth   = Plane0,
			Stencil = Plane1,
		};
	};

	enum class CompareOp : uint32_t
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

	enum class StencilOp : uint32_t
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

	enum class SampleCount : uint32_t
	{
		S1, S2, S4, S8, S16, S32, S64
	};

	enum class Filter : uint32_t
	{
		Point = 0,
		Linear = 1,
	};

	enum class AddressMode : uint32_t
	{
		ClampToEdge,
		ClampToBorder,
		Wrap,
		Mirror,
		MirrorOnce,
	};

	// DX12 has configurable border colors, but most platforms only support these predefined modes
	enum class BorderColor : uint32_t
	{
		TransparentBlack,
		OpaqueBlack,
		OpaqueWhite,
	};

	enum class QueryType : uint32_t
	{
		Timestamp,    // Time parts of the frame
		Occlusion,    // Measure occlusion
		Predication,  // Use for predication queries
		PipelineStats // Use to measure stats in the pipeline
	};

	enum class CameraProjection : uint32_t
	{
		Orthographic,
		Perspective
	};

	namespace BufferUsage
	{
		enum T : uint32_t
		{
			None        = 0,
			Constant    = 1 << 0, // Will be used as constant buffer
			Vertex      = 1 << 1, // Will be used as vertex buffer
			Index       = 1 << 2, // Will be used as index buffer
			Structured  = 1 << 3, // Will be bound as a structured buffer
			Typed       = 1 << 4, // Will be bound as a typed (compressed) buffer
			Byte        = 1 << 5, // Will be bound as a byte (raw data) buffer
			Indirect    = 1 << 6, // Will be used as an indirect buffer
			TransferDst = 1 << 7, // Will be used to transfer data from the GPU to the CPU via a copy operation
			TransferSrc = 1 << 8, // Will be used to transfer data from the CPU to the GPU via a copy operation

			// Compound
			Storage = Structured | Byte,
		};

		inline BufferUsage::T operator | (BufferUsage::T flag1, BufferUsage::T flag2)
		{
			return (BufferUsage::T)((uint32_t)flag1 | (uint32_t)flag2);
		}
	};

	namespace MemoryAccess
	{
		enum T : uint32_t
		{
			GPUOnlyRead,     // Device-only, non-mappable. No unordered access, only copy operations. Use for loaded resources
			GPUOnlyWrite,    // Device-only, non-mappable. Unordered access
			GPUWriteCPURead, // CPU random access, cached. GPU queries, feedback textures
			StagingUpload,   // CPU random access. Staging buffer for upload
			StagingDownload, // CPU random access. Staging buffer for download
			CPUStreamToGPU,  // Uncached, write-combined, coherent. Don't read on CPU. Streaming per-frame data like vertex/constant buffers
		};
	};

	namespace BufferOwnership
	{
		enum T : uint32_t
		{
			NonOwning,
			Owning
		};
	}

	enum class GPUFenceResult : uint32_t
	{
		Success,           // Fence is signaled (ready)
		TimeoutOrNotReady, // Fence is unsignaled right now or timed out (not ready)
		Error              // Some error occurred
	};

	struct MipmapLayout
	{
		uint32_t GetMipSize() const { return rowPitchBytes * heightInPixelsBlocks; }

		uint32_t rowPitchBytes;        // Distance in bytes between rows
		uint32_t offsetBytes;          // Where this mipmap begins
		uint32_t heightInPixelsBlocks; // Height in pixels or blocks if compressed
	};
}

struct CrViewport
{
	CrViewport() : x(0.0f), y(0.0f), width(1.0f), height(1.0f), minDepth(0.0f), maxDepth(1.0f) {}

	CrViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
		: x(x), y(y), width(width), height(height), minDepth(minDepth), maxDepth(maxDepth)
	{}

	explicit CrViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height, float minDepth, float maxDepth)
		: x((float)x), y((float)y), width((float)width), height((float)height), minDepth(minDepth), maxDepth(maxDepth)
	{}

	CrViewport(float x, float y, float width, float height) : CrViewport(x, y, width, height, 0.0f, 1.0f)
	{}

	explicit CrViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) : CrViewport(x, y, width, height, 0.0f, 1.0f)
	{}

	bool operator == (const CrViewport& viewport)
	{
		return x == viewport.x && y == viewport.y && width == viewport.width && height == viewport.height;
	}

	bool operator != (const CrViewport& viewport)
	{
		return !(*this == viewport);
	}

	float x;
	float y;
	float width;
	float height;
	float minDepth;
	float maxDepth;
};

struct CrRectangle
{
	CrRectangle() {}

	CrRectangle(int32_t x, int32_t y, uint32_t width, uint32_t height) : x(x), y(y), width(width), height(height) {}

	bool operator == (const CrRectangle& rect)
	{
		return x == rect.x && y == rect.y && width == rect.width && height == rect.height;
	}

	bool operator != (const CrRectangle& rect)
	{
		return !(*this == rect);
	}

	int32_t x = 0;
	int32_t y = 0;
	uint32_t width = 0;
	uint32_t height = 0;
};

// todo put in cr3d
enum class CrRenderTargetLoadOp : uint32_t
{
	Load, Clear, DontCare
};

enum class CrRenderTargetStoreOp : uint32_t
{
	Store, DontCare
};

namespace CrCommandQueueType
{
	// Queues are each a subset of the other
	//  ______________
	// |   Graphics   |
	// |  __________  |
	// | |  Compute | |
	// | |  ______  | |
	// | | | Copy | | |
	// | | |______| | |
	// | |__________| |
	// |______________|

	enum T : uint32_t
	{
		Graphics,
		Compute,
		Copy
	};
};