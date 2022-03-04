#pragma once

#include "Rendering/ICrRenderDevice.h"

#include "Core/Containers/CrSet.h"
#include "Core/String/CrString.h"
#include "Core/CrCoreForwardDeclarations.h"

#include "Core/SmartPointers/CrSharedPtr.h"

class CrRenderDeviceVulkan final : public ICrRenderDevice
{
public:

	CrRenderDeviceVulkan(const ICrRenderSystem* renderSystem);

	~CrRenderDeviceVulkan();

	VkInstance GetVkInstance() const { return m_vkInstance; }

	VkDevice GetVkDevice() const { return m_vkDevice; }

	VkPhysicalDevice GetVkPhysicalDevice() const { return m_vkPhysicalDevice; }

	VkPipelineCache GetVkPipelineCache() const { return m_vkPipelineCache; }

	VkPhysicalDeviceProperties GetVkPhysicalDeviceProperties() const { return m_vkPhysicalDeviceProperties; }

	VmaAllocator GetVmaAllocator() const { return m_vmaAllocator; }

	uint32_t GetVkMemoryType(uint32_t typeBits, VkFlags properties) const;

	// In Vulkan, we create the queues up-front with the device so we reserve previously created queue indices
	uint32_t ReserveVkQueueIndex();

	uint32_t GetVkQueueMaxCount() const;

	uint32_t GetVkQueueFamilyIndex() const;

	VkQueue GetVkQueue(CrCommandQueueType::T queueType) const;

	VkCommandPool GetVkCommandPool(CrCommandQueueType::T queueType) const;

	void SetVkObjectName(uint64_t vkObject, VkDebugReportObjectTypeEXT objectType, const char* name) const;

private:

	virtual ICrCommandBuffer* CreateCommandBufferPS(CrCommandQueueType::T type) override;

	virtual ICrGPUFence* CreateGPUFencePS() override;

	virtual ICrGPUSemaphore* CreateGPUSemaphorePS() override;

	virtual ICrGraphicsShader* CreateGraphicsShaderPS(const CrGraphicsShaderDescriptor& graphicsShaderDescriptor) const override;

	virtual ICrComputeShader* CreateComputeShaderPS(const CrComputeShaderDescriptor& computeShaderDescriptor) const override;

	virtual ICrHardwareGPUBuffer* CreateHardwareGPUBufferPS(const CrHardwareGPUBufferDescriptor& descriptor) override;

	virtual ICrSampler* CreateSamplerPS(const CrSamplerDescriptor& descriptor) override;

	virtual ICrSwapchain* CreateSwapchainPS(const CrSwapchainDescriptor& swapchainDescriptor) override;

	virtual ICrTexture* CreateTexturePS(const CrTextureDescriptor& descriptor) override;

	virtual ICrGraphicsPipeline* CreateGraphicsPipelinePS(const CrGraphicsPipelineDescriptor& pipelineDescriptor, const ICrGraphicsShader* graphicsShader, const CrVertexDescriptor& vertexDescriptor) override;

	virtual ICrComputePipeline* CreateComputePipelinePS(const CrComputePipelineDescriptor& pipelineDescriptor, const ICrComputeShader* computeShader) override;

	virtual ICrGPUQueryPool* CreateGPUQueryPoolPS(const CrGPUQueryPoolDescriptor& queryPoolDescriptor) override;

	virtual cr3d::GPUFenceResult WaitForFencePS(const ICrGPUFence* fence, uint64_t timeoutNanoseconds) const override;

	virtual cr3d::GPUFenceResult GetFenceStatusPS(const ICrGPUFence* fence) const override;

	virtual void SignalFencePS(CrCommandQueueType::T queueType, const ICrGPUFence* signalFence) override;

	virtual void ResetFencePS(const ICrGPUFence* fence) override;

	virtual void WaitIdlePS() override;

	virtual void SubmitCommandBufferPS(const ICrCommandBuffer* commandBuffer, const ICrGPUSemaphore* waitSemaphore, const ICrGPUSemaphore* signalSemaphore, const ICrGPUFence* signalFence) override;

	virtual CrSharedPtr<ICrHardwareGPUBuffer> DownloadTextureImmediatePS(const ICrTexture* texture, uint32_t mip, uint32_t slice) override;

	virtual void UploadTextureImmediatePS(const ICrHardwareGPUBuffer* stagingBuffer, const ICrTexture* texture) override;

	void RetrieveQueueFamilies();

	VkResult SelectPhysicalDevice();

	VkResult CreateLogicalDevice();

	// Vulkan-specific support query functions

	bool IsVkDeviceExtensionSupported(const CrString& extension);

	// TODO Make platform-independent
	bool IsDepthStencilFormatSupported(VkFormat depthFormat);

	// Came from RenderSystem. This is the instance this device was created from
	VkInstance m_vkInstance;

	// https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#devsandqueues
	VkDevice m_vkDevice;					// Logical device
	
	VkPhysicalDevice m_vkPhysicalDevice;	// Physical device
	
	VkPhysicalDeviceFeatures m_vkDeviceSupportedFeatures;

	VkPipelineCache m_vkPipelineCache; // Centralized pipeline cache

	CrSet<CrString> m_supportedDeviceExtensions;

	// TODO Make this platform-independent
	CrSet<VkFormat> m_supportedRenderTargetFormats;
	CrSet<VkFormat> m_supportedTextureFormats;
	CrSet<VkFormat> m_supportedDepthStencilFormats;
	CrSet<VkFormat> m_supportedVertexBufferFormats;

	// Stores all available memory (type) properties for the physical device
	VkPhysicalDeviceMemoryProperties m_vkPhysicalDeviceMemoryProperties;

	// Stores the different hardware limits reported by this device, e.g. maximum buffer or texture sizes, queue priorities, etc
	VkPhysicalDeviceProperties m_vkPhysicalDeviceProperties;

	VmaAllocator m_vmaAllocator;

	// Queues
	uint32_t m_maxCommandQueues = 0;
	uint32_t m_commandQueueFamilyIndex = 0; // Index of the queue out of the available ones for our hardware
	uint32_t m_numCommandQueues;

	VkQueue m_vkGraphicsQueue;
	VkCommandPool m_vkGraphicsCommandPool;
};
