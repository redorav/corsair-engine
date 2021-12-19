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

	enum class SampleCount : uint32_t;
	
	namespace ShaderStage { enum T : uint32_t; }

	namespace GraphicsApi { enum T : uint32_t; }
}

// Forward declare shader resources
namespace ConstantBuffers { enum T : uint8_t; }
namespace Samplers { enum T : uint8_t; }
namespace Textures { enum T : uint8_t; }
namespace RWTextures { enum T : uint8_t; }
namespace StorageBuffers { enum T : uint8_t; }
namespace RWStorageBuffers { enum T : uint8_t; }
namespace RWDataBuffers { enum T : uint8_t; }

// Forward declare the necessary types for the rendering core

class ICrRenderSystem;

class ICrRenderDevice;
using CrRenderDeviceSharedHandle = CrSharedPtr<ICrRenderDevice>;

class ICrTexture;
using CrTextureSharedHandle = CrSharedPtr<ICrTexture>;
struct CrTextureDescriptor;

class ICrSampler;
using CrSamplerSharedHandle = CrSharedPtr<ICrSampler>;
struct CrSamplerDescriptor;

class ICrCommandQueue;
using CrCommandQueueSharedHandle = CrSharedPtr<ICrCommandQueue>;

class ICrSwapchain;
using CrSwapchainSharedHandle = CrSharedPtr<ICrSwapchain>;
struct CrSwapchainDescriptor;

class ICrGPUFence;
using CrGPUFenceSharedHandle = CrSharedPtr<ICrGPUFence>;

class ICrGPUSemaphore;
using CrGPUSemaphoreSharedHandle = CrSharedPtr<ICrGPUSemaphore>;

class ICrCommandBuffer;
using CrCommandBufferSharedHandle = CrSharedPtr<ICrCommandBuffer>;

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
using CrShaderBytecodeSharedHandle = CrSharedPtr<CrShaderBytecode>;
struct CrShaderCompilationDescriptor;
struct CrShaderBytecodeCompilationDescriptor;
struct CrShaderDefines;
struct CrShaderReflectionHeader;

// GPU Buffers
class ICrHardwareGPUBuffer;
struct CrHardwareGPUBufferDescriptor;
class CrGPUBuffer;
struct CrGPUBufferDescriptor;
class CrGPUStackAllocator;

class CrIndexBufferCommon;
using CrIndexBufferSharedHandle = CrSharedPtr<CrIndexBufferCommon>;

class CrVertexBufferCommon;
using CrVertexBufferSharedHandle = CrSharedPtr<CrVertexBufferCommon>;

template<typename Metadata>
class CrStructuredBuffer;

template<typename Metadata>
using CrStructuredBufferSharedHandle = CrSharedPtr<CrStructuredBuffer<Metadata>>;

class CrDataBuffer;
using CrDataBufferSharedHandle = CrSharedPtr<CrDataBuffer>;

namespace CrVertexSemantic { enum T : uint32_t; }
struct CrVertexDescriptor;

// Visibility
struct CrBoundingBox;

// Render World objects
class CrCamera;
using CrCameraHandle = CrSharedPtr<CrCamera>;

// RenderGraph
class CrRenderGraph;

struct CrRenderGraphPass;
using CrRenderPassId = CrTypedId<CrRenderGraphPass, uint32_t>;

struct CrRenderGraphTextureResource;
using CrRenderGraphTextureId = CrTypedId<CrRenderGraphTextureResource, uint32_t>;

struct CrRenderGraphBufferResource;
using CrRenderGraphBufferId = CrTypedId<CrRenderGraphBufferResource, uint32_t>;