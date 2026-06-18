#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Core/CrTypedId.h"

#include "stdint.h"

namespace crgfx
{
	namespace DataFormat { enum T : uint32_t; }
	struct DataFormatInfo;

	enum class TextureType : uint32_t;
	using TextureUsageFlags = uint32_t;

	namespace ShaderStage { enum T : uint32_t; }

	namespace ShaderStageFlags { enum T : uint32_t; }

	namespace GraphicsApi { enum T : uint32_t; }

	namespace BufferUsage { enum T : uint32_t; }

	namespace MemoryAccess { enum T : uint32_t; }

	namespace ShaderResourceType { enum T : uint32_t; }

	namespace TextureLayout { enum T : uint32_t; }

	namespace TexturePlane { enum T : uint32_t; }

	namespace BufferState { enum T : uint32_t; }

	namespace ShaderStageFlags { enum T : uint32_t; }

	enum class AddressMode : uint32_t;
	enum class BorderColor : uint32_t;
	enum class BlendFactor : uint32_t;
	enum class BlendOp : uint32_t;
	enum class CompareOp : uint32_t;
	enum class Filter : uint32_t;
	enum class FrontFace : uint32_t;
	enum class PrimitiveTopology : uint32_t;
	enum class PolygonCullMode : uint32_t;
	enum class PolygonFillMode : uint32_t;
	enum class SampleCount : uint32_t;
	enum class StencilOp : uint32_t;
	enum class VertexInputRate : uint32_t;

	enum class QueryType : uint32_t;

	struct TextureState;

	namespace CommandQueueType { enum T : uint32_t; }

	enum class RenderTargetLoadOp : uint32_t;
	enum class RenderTargetStoreOp : uint32_t;

	class IGraphicsSystem;

	class IDevice;
	using DeviceHandle = crstl::intrusive_ptr<IDevice>;

	class ITexture;
	using TextureHandle = crstl::intrusive_ptr<ITexture>;
	struct TextureDescriptor;

	class ISampler;
	using SamplerHandle = crstl::intrusive_ptr<ISampler>;
	struct SamplerDescriptor;
	
	class ISwapchain;
	using SwapchainHandle = crstl::intrusive_ptr<ISwapchain>;
	struct SwapchainDescriptor;

	class IGPUFence;
	using GPUFenceHandle = crstl::intrusive_ptr<IGPUFence>;

	class IGPUSemaphore;
	using GPUSemaphoreHandle = crstl::intrusive_ptr<IGPUSemaphore>;

	class ICommandBuffer;
	using CommandBufferHandle = crstl::intrusive_ptr<ICommandBuffer>;
	struct CommandBufferDescriptor;

	struct RenderPassDescriptor;

	// GPU Queries
	class IGPUQueryPool;
	struct GPUQueryPoolDescriptor;
	using GPUQueryPoolHandle = crstl::intrusive_ptr<IGPUQueryPool>;

	// Shaders & Pipeline Objects
	class IGraphicsShader;
	using GraphicsShaderHandle = crstl::intrusive_ptr<IGraphicsShader>;
	struct GraphicsShaderDescriptor;

	class IComputeShader;
	using ComputeShaderHandle = crstl::intrusive_ptr<IComputeShader>;
	struct ComputeShaderDescriptor;

	// Shader Bytecode
	class ShaderBytecode;
	using ShaderBytecodeHandle = crstl::intrusive_ptr<ShaderBytecode>;

	class IGraphicsPipeline;
	using GraphicsPipelineHandle = crstl::intrusive_ptr<IGraphicsPipeline>;
	struct GraphicsPipelineDescriptor;

	class IComputePipeline;
	using ComputePipelineHandle = crstl::intrusive_ptr<IComputePipeline>;

	// GPU Buffers
	class IHardwareGPUBuffer;
	using HardwareGPUBufferHandle = crstl::intrusive_ptr<IHardwareGPUBuffer>;
	struct HardwareGPUBufferDescriptor;

	class GPUBuffer;
	using GPUBufferHandle = crstl::intrusive_ptr<GPUBuffer>;
	struct GPUBufferDescriptor;

	class IndexBuffer;
	using IndexBufferHandle = crstl::intrusive_ptr<IndexBuffer>;

	class VertexBuffer;
	using VertexBufferHandle = crstl::intrusive_ptr<VertexBuffer>;

	template<typename Metadata>
	class StructuredBuffer;

	template<typename Metadata>
	using StructuredBufferHandle = crstl::intrusive_ptr<StructuredBuffer<Metadata>>;

	class TypedBuffer;
	using TypedBufferHandle = crstl::intrusive_ptr<TypedBuffer>;

	namespace VertexSemantic { enum T : uint32_t; }
	struct VertexDescriptor;

	typedef crstl::fixed_function<128, void(const HardwareGPUBufferHandle&)> GPUTransferCallback;
};

class CrGPUStackAllocator;

using CrGPUQueryId = CrTypedId<struct CrGPUQueryDummy, uint32_t>;
class CrGPUTimingQueryTracker;

struct CrShaderReflectionHeader;

// Visibility
struct CrBoundingBox;

// Render World

class CrCamera;
using CrCameraHandle = crstl::intrusive_ptr<CrCamera>;

class CrRenderWorld;
using CrRenderWorldHandle = crstl::intrusive_ptr<CrRenderWorld>;

// Material

class CrMaterial;
using CrMaterialHandle = crstl::intrusive_ptr<CrMaterial>;
struct CrShaderCompilerDefines;

class CrRenderModel;
using CrRenderModelHandle = crstl::intrusive_ptr<CrRenderModel>;

class CrRenderMesh;
using CrRenderMeshHandle = crstl::intrusive_ptr<CrRenderMesh>;

// RenderGraph
class CrRenderGraph;

struct CrRenderGraphPass;
using CrRenderPassId = CrTypedId<CrRenderGraphPass, uint32_t>;

struct CrRenderGraphTextureResource;
using CrRenderGraphTextureId = CrTypedId<CrRenderGraphTextureResource, uint16_t>;

struct CrRenderGraphBufferResource;
using CrRenderGraphBufferId = CrTypedId<CrRenderGraphBufferResource, uint16_t>;

// Forward declare shader resources
namespace ConstantBuffers { enum T : uint8_t; }
namespace Samplers { enum T : uint8_t; }
namespace Textures { enum T : uint8_t; }
namespace RWTextures { enum T : uint8_t; }
namespace StorageBuffers { enum T : uint8_t; }
namespace RWStorageBuffers { enum T : uint8_t; }
namespace TypedBuffers { enum T : uint8_t; }
namespace RWTypedBuffers { enum T : uint8_t; }

struct ConstantBufferMetadata;
struct SamplerMetadata;
struct TextureMetadata;
struct RWTextureMetadata;
struct StorageBufferMetadata;
struct RWStorageBufferMetadata;
struct RWTypedBufferMetadata;