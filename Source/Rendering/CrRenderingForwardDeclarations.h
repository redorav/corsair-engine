#pragma once

#include "Core/CrCoreForwardDeclarations.h"

#include <cstdint>

namespace cr3d
{
	namespace DataFormat { enum T : uint8_t; }
	enum class TextureType : uint8_t;
	using TextureUsageFlags = uint32_t;
	enum class SampleCount :uint8_t;
	struct DataFormatInfo;
	namespace ShaderStage { enum T : uint8_t; }
}

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

class CrShaderBytecode;
using CrShaderBytecodeSharedHandle = CrSharedPtr<CrShaderBytecode>;
struct CrBytecodeLoadDescriptor;
struct CrShaderBytecodeDescriptor;
struct CrShaderStageInfo;

class CrGPUStackAllocator;
class ICrHardwareGPUBuffer;
struct CrGPUBufferDescriptor;
struct CrHardwareGPUBufferDescriptor;
class CrVertexDescriptor;