#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Core/CrTypedId.h"

#include "stdint.h"

namespace cr3d
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
	enum class CameraProjection : uint32_t;

	struct TextureState;
};

enum class CrRenderTargetLoadOp : uint32_t;
enum class CrRenderTargetStoreOp : uint32_t;

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

// Forward declare the necessary types for the rendering core

class ICrRenderSystem;

class ICrRenderDevice;
using CrRenderDeviceHandle = crstl::intrusive_ptr<ICrRenderDevice>;

namespace CrCommandQueueType { enum T : uint32_t; }

class ICrTexture;
using CrTextureHandle = crstl::intrusive_ptr<ICrTexture>;
struct CrTextureDescriptor;

class ICrSampler;
using CrSamplerHandle = crstl::intrusive_ptr<ICrSampler>;
struct CrSamplerDescriptor;

class ICrSwapchain;
using CrSwapchainHandle = crstl::intrusive_ptr<ICrSwapchain>;
struct CrSwapchainDescriptor;

class ICrGPUFence;
using CrGPUFenceHandle = crstl::intrusive_ptr<ICrGPUFence>;

class ICrGPUSemaphore;
using CrGPUSemaphoreHandle = crstl::intrusive_ptr<ICrGPUSemaphore>;

class ICrCommandBuffer;
using CrCommandBufferHandle = crstl::intrusive_ptr<ICrCommandBuffer>;
struct CrCommandBufferDescriptor;

struct CrRenderPassDescriptor;
struct CrComputePassDescriptor;

// GPU Queries
class ICrGPUQueryPool;
struct CrGPUQueryPoolDescriptor;
using CrGPUQueryPoolHandle = crstl::intrusive_ptr<ICrGPUQueryPool>;

class CrGPUQueryDummy;
using CrGPUQueryId = CrTypedId<CrGPUQueryDummy, uint32_t>;

struct CrGPUTimestamp;

class CrGPUTimingQueryTracker;

// Shaders & Pipeline Objects
class ICrGraphicsShader;
using CrGraphicsShaderHandle = crstl::intrusive_ptr<ICrGraphicsShader>;
struct CrGraphicsShaderDescriptor;

class ICrComputeShader;
using CrComputeShaderHandle = crstl::intrusive_ptr<ICrComputeShader>;
struct CrComputeShaderDescriptor;

class ICrGraphicsPipeline;
using CrGraphicsPipelineHandle = crstl::intrusive_ptr<ICrGraphicsPipeline>;
struct CrGraphicsPipelineDescriptor;

class ICrComputePipeline;
using CrComputePipelineHandle = crstl::intrusive_ptr<ICrComputePipeline>;
struct CrComputePipelineDescriptor;

// Shader Bytecode
class CrShaderBytecode;
using CrShaderBytecodeHandle = crstl::intrusive_ptr<CrShaderBytecode>;
struct CrShaderCompilationDescriptor;
struct CrShaderBytecodeCompilationDescriptor;
struct CrShaderCompilerDefines;
struct CrShaderReflectionHeader;

class CrMaterial;
using CrMaterialHandle = crstl::intrusive_ptr<CrMaterial>;

// GPU Buffers
class ICrHardwareGPUBuffer;
using CrHardwareGPUBufferHandle = crstl::intrusive_ptr<ICrHardwareGPUBuffer>;
struct CrHardwareGPUBufferDescriptor;

class CrGPUBuffer;
using CrGPUBufferHandle = crstl::intrusive_ptr<CrGPUBuffer>;

struct CrGPUBufferDescriptor;
class CrGPUStackAllocator;

class CrIndexBuffer;
using CrIndexBufferHandle = crstl::intrusive_ptr<CrIndexBuffer>;

class CrVertexBuffer;
using CrVertexBufferHandle = crstl::intrusive_ptr<CrVertexBuffer>;

template<typename Metadata>
class CrStructuredBuffer;

template<typename Metadata>
using CrStructuredBufferHandle = crstl::intrusive_ptr<CrStructuredBuffer<Metadata>>;

class CrTypedBuffer;
using CrTypedBufferHandle = crstl::intrusive_ptr<CrTypedBuffer>;

typedef crstl::fixed_function<128, void(const CrHardwareGPUBufferHandle&)> CrGPUTransferCallbackType;

namespace CrVertexSemantic { enum T : uint32_t; }
struct CrVertexDescriptor;

// Visibility
struct CrBoundingBox;

// Render World

class CrCamera;
using CrCameraHandle = crstl::intrusive_ptr<CrCamera>;

class CrRenderWorld;
using CrRenderWorldHandle = crstl::intrusive_ptr<CrRenderWorld>;

// RenderGraph
class CrRenderGraph;

struct CrRenderGraphPass;
using CrRenderPassId = CrTypedId<CrRenderGraphPass, uint32_t>;

struct CrRenderGraphTextureResource;
using CrRenderGraphTextureId = CrTypedId<CrRenderGraphTextureResource, uint16_t>;

struct CrRenderGraphBufferResource;
using CrRenderGraphBufferId = CrTypedId<CrRenderGraphBufferResource, uint16_t>;

class CrRenderModel;
using CrRenderModelHandle = crstl::intrusive_ptr<CrRenderModel>;

class CrRenderMesh;
using CrRenderMeshHandle = crstl::intrusive_ptr<CrRenderMesh>;