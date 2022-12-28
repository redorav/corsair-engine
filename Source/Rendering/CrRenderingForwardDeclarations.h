#pragma once

#include "Core/CrCoreForwardDeclarations.h"
#include "Core/CrTypedId.h"

#include <cstdint>

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
namespace RWDataBuffers { enum T : uint8_t; }

struct ConstantBufferMetadata;
struct SamplerMetadata;
struct TextureMetadata;
struct RWTextureMetadata;
struct StorageBufferMetadata;
struct RWStorageBufferMetadata;
struct RWDataBufferMetadata;

// Forward declare the necessary types for the rendering core

class ICrRenderSystem;

class ICrRenderDevice;
using CrRenderDeviceHandle = CrSharedPtr<ICrRenderDevice>;

namespace CrCommandQueueType { enum T : uint32_t; }

class ICrTexture;
using CrTextureHandle = CrSharedPtr<ICrTexture>;
struct CrTextureDescriptor;

class ICrSampler;
using CrSamplerHandle = CrSharedPtr<ICrSampler>;
struct CrSamplerDescriptor;

class ICrSwapchain;
using CrSwapchainHandle = CrSharedPtr<ICrSwapchain>;
struct CrSwapchainDescriptor;

class ICrGPUFence;
using CrGPUFenceHandle = CrSharedPtr<ICrGPUFence>;

class ICrGPUSemaphore;
using CrGPUSemaphoreHandle = CrSharedPtr<ICrGPUSemaphore>;

class ICrCommandBuffer;
using CrCommandBufferHandle = CrSharedPtr<ICrCommandBuffer>;
struct CrCommandBufferDescriptor;

struct CrRenderPassDescriptor;
struct CrComputePassDescriptor;

// GPU Queries
class ICrGPUQueryPool;
struct CrGPUQueryPoolDescriptor;
using CrGPUQueryPoolHandle = CrSharedPtr<ICrGPUQueryPool>;

class CrGPUQueryDummy;
using CrGPUQueryId = CrTypedId<CrGPUQueryDummy, uint32_t>;

struct CrGPUTimestamp;

class CrGPUTimingQueryTracker;

// Shaders & Pipeline Objects
class ICrGraphicsShader;
using CrGraphicsShaderHandle = CrSharedPtr<ICrGraphicsShader>;
struct CrGraphicsShaderDescriptor;

class ICrComputeShader;
using CrComputeShaderHandle = CrSharedPtr<ICrComputeShader>;
struct CrComputeShaderDescriptor;

class ICrGraphicsPipeline;
using CrGraphicsPipelineHandle = CrSharedPtr<ICrGraphicsPipeline>;
struct CrGraphicsPipelineDescriptor;

class ICrComputePipeline;
using CrComputePipelineHandle = CrSharedPtr<ICrComputePipeline>;
struct CrComputePipelineDescriptor;

// Shader Bytecode
class CrShaderBytecode;
using CrShaderBytecodeHandle = CrSharedPtr<CrShaderBytecode>;
struct CrShaderCompilationDescriptor;
struct CrShaderBytecodeCompilationDescriptor;
struct CrShaderCompilerDefines;
struct CrShaderReflectionHeader;

class CrMaterial;
using CrMaterialHandle = CrIntrusivePtr<CrMaterial>;

// GPU Buffers
class ICrHardwareGPUBuffer;
using CrHardwareGPUBufferHandle = CrSharedPtr<ICrHardwareGPUBuffer>;
struct CrHardwareGPUBufferDescriptor;

class CrGPUBuffer;
using CrGPUBufferHandle = CrSharedPtr<CrGPUBuffer>;

struct CrGPUBufferDescriptor;
class CrGPUStackAllocator;

class CrIndexBuffer;
using CrIndexBufferHandle = CrSharedPtr<CrIndexBuffer>;

class CrVertexBuffer;
using CrVertexBufferHandle = CrSharedPtr<CrVertexBuffer>;

template<typename Metadata>
class CrStructuredBuffer;

template<typename Metadata>
using CrStructuredBufferHandle = CrSharedPtr<CrStructuredBuffer<Metadata>>;

class CrDataBuffer;
using CrDataBufferHandle = CrSharedPtr<CrDataBuffer>;

typedef CrFixedFunction<4, void(const CrHardwareGPUBufferHandle&)> CrGPUTransferCallbackType;

namespace CrVertexSemantic { enum T : uint32_t; }
struct CrVertexDescriptor;

// Visibility
struct CrBoundingBox;

// Render World

class CrCamera;
using CrCameraHandle = CrIntrusivePtr<CrCamera>;

class CrRenderWorld;
using CrRenderWorldHandle = CrIntrusivePtr<CrRenderWorld>;

// RenderGraph
class CrRenderGraph;

struct CrRenderGraphPass;
using CrRenderPassId = CrTypedId<CrRenderGraphPass, uint32_t>;

struct CrRenderGraphTextureResource;
using CrRenderGraphTextureId = CrTypedId<CrRenderGraphTextureResource, uint16_t>;

struct CrRenderGraphBufferResource;
using CrRenderGraphBufferId = CrTypedId<CrRenderGraphBufferResource, uint16_t>;

class CrRenderModel;
using CrRenderModelHandle = CrIntrusivePtr<CrRenderModel>;